/*
	Aimbot source code for Counter-Strike: Global Offensive on Windows
	The code was tested on Windows 10 and 11
	
	--- HOW TO USE ---
	1. To use the aimbot, run the game FIRST, and then run the .exe build from the solution, or by running the source code in an IDE like Visual Studio
	2. The Aimbot will NOT work on its own, it has been coded so that it uses the LEFT ALT key as a trigger, pressing this key will make the Aimbot do its job
	3. Use the END key to unload the hack, or you can just stop the .exe from running, both work reliably well :)

	!!! BUILD AND RUN AS A .EXE !!!
	!!! Please note that for compatibility purposes, run CS:GO in FULLSCREEN or WINDOWED FULLSCREEN options !!!
*/

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <math.h>


// Key offsets and pointers to key properties for the game so the hack can work
// !!! SOME OF THESE OFFSETS CAN CHANGE, EITHER FIND THEM OR USE A DUMPER TO UPDATE THEM FOR YOUR INSTANCE !!!
#define offset_LocalPlayer 0xDB35DC // The local player entity, used to get the properties of the current player
#define offset_EntityList 0x4DCEEAC // The list of players in the game
#define offset_BoneMatrix 0x26A8 // The stucture of the player's skeleton
#define offset_TeamMembership 0xF4 // The team that each of the players is on
#define offset_PlayerHealth 0x100 // The health of each of the players in the game, property of the entities
#define offset_PlayerOrigin 0x138 // The current positions of the players, property for the entities
#define offset_DormantFlag 0xED // The current alive/dead state of the player, property of the entities
#define offset_ViewMatrix 0x4DC07C4 // View structure used to control the view of the player

// Gets the width and height of the current aimbot canvas, defaults to fullscreen
const int screen_width = GetSystemMetrics(SM_CXSCREEN);
const int screen_height = GetSystemMetrics(SM_CYSCREEN);

// Sets the position of the crosshair, in CS:GO, this is "perfectly" centered
const int crosshair_x = screen_width / 2;
const int crosshair_y = screen_height / 2;

HWND window_handler; // Handler to the CS:GO window
DWORD process_id; // Store the process ID of csgo.exe
HANDLE handle_process; // Used for the communication between game and software, read/write
HDC handle_device; // Handler to use the devices on the machine, for this hack, the mouse
uintptr_t base_module; // Stores the address to the module that we will be using "client.dll" for this hack
int closest; // Set a int for the thread that wil be running

/// <summary>
/// A class to define a vector in 3 dimensions
/// </summary>
class Vector3D {
public:
	float x, y, z;
	Vector3D() { this->x = 0.0f, this->y = 0.0f, this->z = 0.0f; }
	Vector3D(float x, float y, float z) { this->x = x, this->y = y, this->z = z; }
};

/// <summary>
/// Function to return the address of the specified module that we will be using
/// </summary>
/// <param name="module_name"> The name of the module that we are looking for </param>
/// <returns> Retuns the pointer to the module referenced </returns>
uintptr_t get_base_address(const char* module_name) {
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
	if (handle != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 module_entry;
		module_entry.dwSize = sizeof(module_entry);
		if (Module32First(handle, &module_entry)) {
			do {
				if (!strcmp(module_entry.szModule, module_name)) {
					CloseHandle(handle);
					return (uintptr_t)module_entry.modBaseAddr;
				}
			} while (Module32Next(handle, &module_entry));
		}
	}
}

/// <summary>
/// Function to read the memory of the game to identify the values that we want to read and write
/// These can change the data type, so the template has been defined to allow for that
/// </summary>
/// <typeparam name="T"> To allow for the different data types that can be returned and passed to the template </typeparam>
/// <param name="address"> Address of the memory to read and to write to </param>
/// <returns> Returns any value that may be put into the buffer as a result of the function </returns>
template<typename T> T read_process_memory(SIZE_T address) {
	T buffer;
	ReadProcessMemory(handle_process, (LPCVOID)address, &buffer, sizeof(T), NULL);
	return buffer;
}

// Get the team membership of the player referenced
int get_team(uintptr_t player) {
	return read_process_memory<int>(player + offset_TeamMembership);
}

// Get the health of the player referenced
int get_health(uintptr_t player) {
	return read_process_memory<int>(player + offset_PlayerHealth);
}

// Get the pointer to the local player using thje offset
uintptr_t get_local_player() {
	return read_process_memory<int>(base_module + offset_LocalPlayer);
}

// Get the pointer to a player
uintptr_t get_player(int index) {
	return read_process_memory<uintptr_t>(base_module + offset_EntityList + index * 0x10);
}

// Get the location of a player
Vector3D get_player_location(uintptr_t player) {
	return read_process_memory<Vector3D>(player + offset_PlayerOrigin);
}

// Check of the player is "dormant", dead or alive 
bool dormant_check(uintptr_t player) {
	return read_process_memory<int>(player + offset_DormantFlag);
}


// Get the head position of the player
Vector3D get_player_head(uintptr_t player) {
	struct bone_matrix_t {
		byte pad3[12];
		float x;
		byte pad1[12];
		float y;
		byte pad2[12];
		float z;
	};
	uintptr_t base_bone = read_process_memory<int>(player + offset_BoneMatrix);
	bone_matrix_t bone_matrix = read_process_memory<bone_matrix_t>(base_bone + (sizeof(bone_matrix) * 8));
	return Vector3D(bone_matrix.x, bone_matrix.y, bone_matrix.z);
}

// Structure of the view matrix, put in a struct so it can be assigned to virtual memory
struct view_matrix_t {
	float matrix[16];
} vm;


// Structure to transform the 3D world of CS:GO into 2D cooridnates for your screen
struct Vector3D make_world_to_screen(const struct Vector3D position, struct view_matrix_t view_matrix) {
	struct Vector3D result;
	float _x = (view_matrix.matrix[0] * position.x) + (view_matrix.matrix[1] * position.y) + (view_matrix.matrix[2] * position.z) + (view_matrix.matrix[3]);
	float _y = (view_matrix.matrix[4] * position.x) + (view_matrix.matrix[5] * position.y) + (view_matrix.matrix[6] * position.z) + (view_matrix.matrix[7]);
	result.z = (view_matrix.matrix[12] * position.x) + (view_matrix.matrix[13] * position.y) + (view_matrix.matrix[14] * position.z) + (view_matrix.matrix[15]);

	_x *= 1.0f / result.z;
	_y *= 1.0f / result.z;

	result.x = screen_width * 0.5f;
	result.y = screen_height * 0.5f;

	result.x += 0.5f * _x * screen_width + 0.5f;
	result.y -= 0.5f * _y * screen_height + 0.5f;

	return result;
}

// Function to calculate the distance between the crosshair and the enemy's head
float calculate_distance_to_enemy(int x1, int y1, int x2, int y2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

// Function to find the nearest enemy to the crosshair, this functions runs as a seperate thread
int find_nearest_enemy() {
	DWORD entity;
	float finish;
	int closest_entity = 1;
	Vector3D calculate = { 0,0,0 };
	float closest = FLT_MAX;
	int current_team = get_team(get_local_player());
	for (int i = 1; i < 64; i++) {
		entity = get_player(i);
		int enemy_team = get_team(entity); if (enemy_team == current_team) continue;
		int enemy_health = get_health(entity); if (enemy_health < 1 || enemy_health > 100) continue;
		int enemy_dormant = dormant_check(entity); if (enemy_dormant) continue;
		Vector3D head = make_world_to_screen(get_player_head(entity), vm);
		finish = calculate_distance_to_enemy(head.x, head.y, crosshair_x, crosshair_y);
		if (finish < closest) {
			closest = finish;
			closest_entity = i;
		}
	}
	return closest_entity;
}

// This line is completely optional
// !!! IMPLEMENTED FOR DEBUGGING !!!
void draw_line(float start_x, float start_y, float end_x, float end_y) {
	int a, b = 0;
	HPEN pen_Ohandle;
	HPEN pen_Nhandle = CreatePen(PS_SOLID, 2, 0x0000FF);
	pen_Ohandle = (HPEN)SelectObject(handle_device, pen_Nhandle);
	MoveToEx(handle_device, start_x, start_y, NULL);
	a = LineTo(handle_device, end_x, end_y);
	DeleteObject(SelectObject(handle_device, pen_Ohandle));
}

// Creates a thread to always find the nearest enemy
void thread_find_nearest_enemy() {
	while (true) {
		closest = find_nearest_enemy();
	}
}


// The main!
int main() {
	window_handler = FindWindowA(NULL, "Counter-Strike: Global Offensive - Direct3D 9"); // Find the game window
	GetWindowThreadProcessId(window_handler, &process_id); // Pull the process ID
	base_module = get_base_address("client.dll"); // Get a pointer to the client.dll module
	handle_process = OpenProcess(PROCESS_ALL_ACCESS, NULL, process_id); // Open a communication channel between the game and the executable
	handle_device = GetDC(window_handler); // Make a handler for the devices on the PC 
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)thread_find_nearest_enemy, NULL, NULL, NULL); // Create tthe thread and run to find the nearest enemy
	while (!GetAsyncKeyState(VK_END)) { // While DEL has not been pressed
		vm = read_process_memory<view_matrix_t>(base_module + offset_ViewMatrix); // Update the view matrix with every loop
		Vector3D closest_to_head = make_world_to_screen(get_player_head(get_player(closest)), vm); // Get the distance to the nearest enemy's head
		draw_line(crosshair_x, crosshair_y, closest_to_head.x, closest_to_head.y); // Draw a line to the object, for debugging, can be commented out
		if (GetAsyncKeyState(VK_MENU) && closest_to_head.z >= 0.001f) { // If ALT is pressed
			SetCursorPos(closest_to_head.x, closest_to_head.y); // Snap the crosshair to the head of the enemy
			if (calculate_distance_to_enemy(closest_to_head.x, closest_to_head.y, crosshair_x, crosshair_y) < 1.f) { // If the crosshair is on the head
				mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, NULL, NULL); // LEFT CLICK!! Shoot, basically
				mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, NULL, NULL); // Let of of left click, to stop shooting :)
			}
		}
	}
}