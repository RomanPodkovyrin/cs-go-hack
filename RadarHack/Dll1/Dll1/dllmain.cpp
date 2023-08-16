// dllmain.cpp : Defines the entry point for the DLL application.
//#include <SFML/Graphics.hpp>
#include <Windows.h>
#include "pch.h"
#include <iostream>
#include "entity.h"
#include <GL/glut.h> 
#include <cmath> 
#include <string>

#define PI 3.14159265
float maxDistance = 1500.0f;
float radarRadius = 0.7f;
float arrowLength = 700.0f;
float playerSize = 0.04f;

// OpenGL Functions

void drawCircleSegments(float cx, float cy, float r) {
	int segments = 100;
	for (int i = 0; i < segments; i++) {
		float theta = 2.0f * 3.1415926f * float(i) / float(segments); 
		float x = r * cosf(theta);
		float y = r * sinf(theta);
		glVertex2f(x + cx, y + cy);
	}
}

/*
* Draws Circle outline
*/
void drawCircle(float cx, float cy, float r) {
	glBegin(GL_LINE_LOOP);
	// White
	glColor3f(1.0, 1.0, 1.0);
	drawCircleSegments(cx, cy, r);
	glEnd();
}

/*
* Draws filled in circle
*/
void drawPlayer(float cx, float cy, float r) {
	glBegin(GL_POLYGON);
	drawCircleSegments(cx, cy, r);
	glEnd();
}

/*
* Draws health at a given location
*/
void drawHealth(float x, float y, int health) {
	std::string str = "";
	str = std::to_string(health);
	glColor3f(0.0, 1.0, 0.0);
	glRasterPos2f(x, y);
	int len = str.length();
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i]);
	}
	
}

/*
* Draw line from boint A to point B
*/
void drawLine(float ax, float ay, float bx, float by) {
	glBegin(GL_LINES);
	glVertex2f(ax, ay);
	glVertex2f(bx, by);
	glEnd();
}


// Functions for radar calculations

float angleBetween2Points(float ax, float ay, float bx, float by) {
	return ((atan2(ay - by, ax - bx)) * 180 / PI);
}

/*
* Returns x based on rotation and distance
*/
float translateX(float distance, float angle) {
	return distance * cos(angle) - distance * sin(angle);
}

/*
* Returns y based on rotation and distance
*/
float translateY(float distance, float angle) {
	return distance * cos(angle) + distance * sin(angle);
}

/*
* Rescales CS:GO coordinates to OpenGL coordinates
*/
float rescale(float myx, float myy, float px, float py) {
	float xRescaled = (myx - px);
	float yRescaled = (myy - py);
	float newDistance = (sqrt(pow(xRescaled, 2) + pow(yRescaled, 2)) * radarRadius * 0.7) / maxDistance ;
	return newDistance;
}


/*
* Convert (0 to 180 and 0 to -180) angle to 0 to 360 angle
*/
float convertAngle(float angle) {
	if (angle < 0) {
		angle = 360 + angle;
	}
	return angle;
}

// Main Loop
void display() {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
		glClear(GL_COLOR_BUFFER_BIT);     // Clear the color buffer

		uintptr_t ClientAddr = (uintptr_t)GetModuleHandle(L"client.dll");
		ListOfEntities* listOfEntitiesPointer = (ListOfEntities*)(ClientAddr + 0x4DCEE9C);

		// Draw Radar borders
		drawCircle(0, 0, radarRadius);
		drawCircle(0, 0, radarRadius/2);
		// Drawing my player middle circle
		drawCircle(0, 0, 0.04);
		// Draw my direction facing north
		drawLine(0, 0, 0, radarRadius);
	

		float myPlayerX = 0;
		float myPlayerY = 0;
		float myPlayerPovAngle = 0;
		int myPlayerTeam = 0;
		
		// Getting My player values
		if (listOfEntitiesPointer->listOfEntities) {
			try {
				if (sizeof(ListOfEntities) > 0 && listOfEntitiesPointer && (*listOfEntitiesPointer).listOfEntities[0].EntityPointer) {
					Entity myPlayerPointer = *(*listOfEntitiesPointer).listOfEntities[0].EntityPointer;

					myPlayerX = myPlayerPointer.xAxis;
					myPlayerY = myPlayerPointer.yAxis;
					myPlayerPovAngle = myPlayerPointer.cameraH;
					myPlayerTeam = convertAngle( myPlayerPointer.team);
					std::cout << "My Player: " << myPlayerPointer.health << " x: " << myPlayerX << " y: " << myPlayerY << " angle: " << myPlayerPovAngle << " team: " << myPlayerTeam << std::endl;
				}
			}
			catch (std::exception e) {
				std::cout << "error\n";
			}
		}

		std::cout << "#####Players nearby#######" << std::endl;
		int index = 0;
		for (auto player : listOfEntitiesPointer->listOfEntities) {
			
			if (index == 0) {
				index++;
				continue;
			}
			if (player.EntityPointer) {
				float playerX = player.EntityPointer->xAxis;
				float playerY = player.EntityPointer->yAxis;
				int playerTeam = player.EntityPointer->team;
				int playerHealth = player.EntityPointer->health;
				float playerPovAngle = convertAngle(player.EntityPointer->cameraH);
				

				float distance = sqrt(pow(myPlayerX - playerX, 2) + pow(myPlayerY - playerY, 2) * 1.0);

				// Only display players withing range
				if (playerHealth > 0 && distance <= maxDistance) {
		
					// Draw player
					std::cout <<"Player Health: "<< player.EntityPointer->health << " x: " << player.EntityPointer->xAxis << " y: " << player.EntityPointer->yAxis<< " PoV Angle: "<<playerPovAngle << " distance: " << distance << std::endl;
					float newDistance = rescale(myPlayerX, myPlayerY, playerX, playerY);
					//Angle between My player and other player
					double theta = angleBetween2Points(playerX,playerY,myPlayerX,myPlayerY);
					// transformed angle
					double newAngle = (theta - myPlayerPovAngle + 45) * PI / 180;
					float adjustedx = translateX(newDistance, newAngle);
					float adjustedy = translateY(newDistance, newAngle);
					
					// Different color depending on the team
					if (myPlayerTeam != playerTeam) {
						glColor3f(1.0, 0.0, 0.0); // Red
					}
					else {
						glColor3f(0.0, 0.0, 1.0); // Blue
					}
					drawPlayer(adjustedx, adjustedy, playerSize);



					// Draw the arrow for POV of the player
					//get the position from the displacements in translated coords
					float linePointX = (arrowLength * cos((playerPovAngle)*PI / 180)) + playerX;
					float linePointY = (arrowLength * sin((playerPovAngle)*PI / 180)) + playerY;

					float arrowDistance = rescale(myPlayerX, myPlayerY, linePointX, linePointY);
					//Angle between My player and the arrow end
					float thetaBetweenArrowEndAndMe = angleBetween2Points(linePointX, linePointY, myPlayerX, myPlayerY);
					// transformed angel
					float arrowAngle = (thetaBetweenArrowEndAndMe - myPlayerPovAngle + 45) * PI / 180;
					float arrowEndX = translateX(arrowDistance, arrowAngle);
					float arrowEndY = translateY(arrowDistance, arrowAngle);
					drawLine(adjustedx, adjustedy, arrowEndX, arrowEndY);




					// Health of the player
					drawHealth(adjustedx, adjustedy, playerHealth);
					
					
					

				}
			}
		

			index++;
		}

		std::cout << "################" << "\n";
		glutSwapBuffers();
		Sleep(100);
}




// CS:GO Hack

// This method defines a threat that will run concurrently with the game
DWORD WINAPI MyThread(HMODULE hModule)
{
	AllocConsole();
	FILE* f = new FILE;
	freopen_s(&f, "CONOUT$", "w", stdout);

	std::cout << "RadarHack Worked\n";
	std::cout << "Process ID is: " << GetCurrentProcessId() << std::endl;


	// Start OpenGL Window
	int argc = 1;
	char* argv[1] = { (char*)"Something" };
	glutInit(&argc, argv);                 // Initialize GLUT
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("Radar Hack"); // Create a window with the given title
	glutInitWindowPosition(50, 50); // Position the window's initial top-left corner
	glutInitWindowSize(920, 920);   // Set the window's initial width & height
	glutDisplayFunc(display); // Register display callback handler for window re-paint
	glutIdleFunc(display); // Loop when window is idle
	glutMainLoop();
	
	fclose(f);
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: {
		// We run the cheat code in a seperate thread to stop it interupting the game execution. 
		// Again we dont catch a possible NULL, if we are going down then we can go down in flames. 
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MyThread, hModule, 0, nullptr));
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

