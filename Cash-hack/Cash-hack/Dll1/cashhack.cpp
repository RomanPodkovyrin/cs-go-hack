// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include "pch.h"
#include <iostream>
#include "entity.h"



// CS:GO Cash Hack - Oliver

DWORD WINAPI MyThread(HMODULE hModule)
{
	//Define console that we will output cash to
	AllocConsole();
	FILE* f = new FILE;
	freopen_s(&f, "CONOUT$", "w", stdout);

	//Get address of cash
	uintptr_t ClientAddr = (uintptr_t)GetModuleHandle(L"client.dll");
	uintptr_t base = *(uintptr_t*)(ClientAddr + 0x4DEEEDC); //offsets found in cheat engine
	int* cash_address = (int*)(base + 0x117B4);

	printf("Printing cash until END key pressed \n");

	// Run loop until END key is pressed
	while (true && !GetAsyncKeyState(0x23)) {
		// just print the cash.
		int cash = *cash_address;
		printf("Current cash: %d \n", cash);
		std::cout << "################" << "\n";
		Sleep(1000); //wait 1 second between prints to prevent spamming console
	}

	//Gracefully close the thread
	if (f != NULL) { fclose(f); };
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

