// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include "pch.h"
#include <iostream>
#include "entity.h"
#pragma comment(lib, "user32.lib");


// CS:GO Hack Field of View


// This method defines a threat that will run concurrently with the game
DWORD WINAPI MyThread(HMODULE hModule)
{
	// The following 3 lines enable a writable console
	// We don't actually need a console here, but it is very useful to print debugging information to. 
	AllocConsole();
	FILE* f = new FILE;
	freopen_s(&f, "CONOUT$", "w", stdout);

	std::cout << "Injection worked\n";
	std::cout << "Process ID is: " << GetCurrentProcessId() << std::endl;
	// We can see by looking at the process ID in process explorer that this code is being run by the process it was injected into. 
	
	uintptr_t ClientAddr = (uintptr_t)GetModuleHandle(L"client.dll");
	uintptr_t Player = *(uintptr_t*)(ClientAddr + 0x4DCEEAC);
	int feild_view = 90;

	while (true) {
		int FOV = *(int*)(Player + 0x333c);
		std::cout << "Field of View" << FOV << std::endl;
		
		//Increase viewpoint:: key F7
		if (GetAsyncKeyState(0x76) & 1) {
			feild_view = feild_view + 5;
			*(int*)(Player + 0x333c) = feild_view;
		}
		//Decrease viewpoint:: key F8
		 if (GetAsyncKeyState(0x77) & 1) {
			 feild_view = feild_view - 5;
			*(int*)(Player + 0x333c) = feild_view;
		}
		//set viewpoint to default
		if (GetAsyncKeyState(0x78) & 1) {
			feild_view = 90;
			*(int*)(Player + 0x333c) = feild_view;
			
		}
	}


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

