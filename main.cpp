#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <ctime>
#include <string>
#include <sstream>
#include <fstream>
#include <amp.h>
#include <thread>

#include "hijacker.h"
#include "main.h"

DWORD procId;
uintptr_t clientaddr;
uintptr_t engineaddr;
HANDLE processHandle;
bool toggleBhop = false;
bool toggleStrafe = false;

uintptr_t ModuleAddr(const char* DesiredModule)
{
	HANDLE SysSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	MODULEENTRY32 mEntry32 = {};
	mEntry32.dwSize = sizeof(mEntry32);

	ZeroMemory((void*)mEntry32.szModule, sizeof(mEntry32.szModule));

	for (size_t i = 0; i < mEntry32.dwSize; ++i)
	{
		Module32Next(SysSnap, &mEntry32);

		if (!strcmp(mEntry32.szModule, DesiredModule))
		{
			CloseHandle(SysSnap);
			return (uintptr_t)mEntry32.modBaseAddr;
		}
	}
	MessageBoxA(NULL, "Error finding module. CS:GO is not running, or try running as admin.", "Error", NULL);
	system("pause");
	return true;
}

// write memory
template<typename T> void Write(SIZE_T address, T buffer) 
{
	WriteProcessMemory(processHandle, (LPVOID)address, &buffer, sizeof(buffer), NULL);
}

// read memory
template<typename T> T Read(SIZE_T address) {
	T buffer;
	ReadProcessMemory(processHandle, (LPCVOID)address, &buffer, sizeof(T), NULL);
	return buffer;
}

auto TitleGen = [](int num)
{
	std::string nameoftitle;
	for (int i = 0; i < num; i++)
		nameoftitle += rand() % 255 + 1;
	return nameoftitle;
};

std::string ascii = R"(
                            __                        _  
                           [  |                      | | 
 _ .--..--.   ,--.   .---.  | |--.   .--.   _ .--.   | | 
[ `.-. .-. | `'_\ : / /__\\ | .-. |/ .'`\ \[ '/'`\ \ | | 
 | | | | | | // | |,| \__., | | | || \__. | | \__/ | |_| 
[___||__||__]\'-;__/ '.__.'[___]|__]'.__.'  | ;.__/  (_) 
                                           [__|          
)";

void rgb()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	while (true)
	{
		for (int i = 0; i < 16; ++i)
		{
			SetConsoleTextAttribute(hConsole, i);
			std::cout << ascii << std::endl;
			std::cout << "Now using maehop." << std::endl;
			std::cout << "F4 for BHOP" << std::endl;
			std::cout << "BHOP = "  + std::to_string(toggleBhop) << std::endl;
			Sleep(20);
			system("CLS");
			if (i == 7)
			{
				++i;
			}
			if (i == 15)
			{
				i = 0;
			}
		}
		Sleep(200);
	}
}

int main(int argc, char* argv[])
{
	// title
	SetConsoleTitleA(TitleGen(rand() % 100 + 30).c_str());
	std::thread menuthread(rgb);

	// window handle
	HWND consoleWindow = GetConsoleWindow();
	SetWindowPos(consoleWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// csgo process
	HWND gameWindow = FindWindow(NULL, "Counter-Strike: Global Offensive");
	GetWindowThreadProcessId(gameWindow, &procId);
	clientaddr = ModuleAddr("client.dll");
	engineaddr = ModuleAddr("engine.dll");
	processHandle = HijackExistingHandle(procId);

	Sleep(1000);

	int LocalPlayer;
	int fFlags;

	while (1)
	{
		LocalPlayer = Read<uintptr_t>(clientaddr + addr.dwLocalPlayer);
		fFlags = Read<uintptr_t>(LocalPlayer + addr.m_fFlags);

		if (GetAsyncKeyState(VK_F4) & 1)
		{
			toggleBhop = !toggleBhop;
		}

		if (GetAsyncKeyState(VK_SPACE))
		{
			if (toggleBhop)
			{
				// check if in the air, standing or crouched
				if (fFlags == 257 || fFlags == 263)
				{
					Write(clientaddr + addr.dwForceJump, 6);
				}
			}
		}

		Sleep(1);
	}

	return 0;
}


