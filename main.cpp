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

// Global Variables
DWORD procId;
uintptr_t clientaddr, engineaddr;
HANDLE processHandle;
bool toggleBhop = false;

// Function Prototypes
uintptr_t ModuleAddr(const char* DesiredModule);
template<typename T> bool Write(SIZE_T address, T buffer);
template<typename T> bool Read(SIZE_T address, T& buffer);
std::string TitleGen(int num);
void rgb();

// ASCII Art
std::string ascii = R"(
                            __                        _  
                           [  |                      | | 
 _ .--..--.   ,--.   .---.  | |--.   .--.   _ .--.   | | 
[ `.-. .-. | `'_\ : / /__\\ | .-. |/ .'`\ \[ '/'`\ \ | | 
 | | | | | | // | |,| \__., | | | || \__. | | \__/ | |_| 
[___||__||__]\'-;__/ '.__.'[___]|__]'.__.'  | ;.__/  (_) 
                                           [__|          
)";

int main(int argc, char* argv[])
{
    srand(time(NULL));
    SetConsoleTitleA(TitleGen(rand() % 100 + 30).c_str());
    std::thread menuthread(rgb);
    HWND consoleWindow = GetConsoleWindow();
    SetWindowPos(consoleWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    HWND gameWindow = FindWindow(NULL, "Counter-Strike: Global Offensive");
    GetWindowThreadProcessId(gameWindow, &procId);
    clientaddr = ModuleAddr("client.dll");
    engineaddr = ModuleAddr("engine.dll");
    processHandle = HijackExistingHandle(procId);
    Sleep(1000);

    int LocalPlayer, fFlags;
    bool shouldJump = false;
    while (1) {
        if (GetAsyncKeyState(VK_F4) & 1) toggleBhop = !toggleBhop;
        if (toggleBhop && GetAsyncKeyState(VK_SPACE)) {
            if (!shouldJump) {
                LocalPlayer = Read<uintptr_t>(clientaddr + addr.dwLocalPlayer);
                if (Read(LocalPlayer + addr.m_fFlags, fFlags) && (fFlags == 257 || fFlags == 263))
                    Write(clientaddr + addr.dwForceJump, 6);
                shouldJump = true;
            }
        } else {
            shouldJump = false;
        }
        Sleep(1);
    }
    return 0;
}

uintptr_t ModuleAddr(const char* DesiredModule) {
    HANDLE SysSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    MODULEENTRY32 mEntry32 = {};
    mEntry32.dwSize = sizeof(mEntry32);
    while (Module32Next(SysSnap, &mEntry32)) {
        if (!strcmp(mEntry32.szModule, DesiredModule)) {
            CloseHandle(SysSnap);
            return (uintptr_t)mEntry32.modBaseAddr;
        }
    }
    return NULL;
}

template<typename T> 
bool Write(SIZE_T address, T buffer) {
    SIZE_T bytesWritten;
    WriteProcessMemory(processHandle, (LPVOID)address, &buffer, sizeof(buffer), &bytesWritten);
    return bytesWritten == sizeof(buffer);
}

template<typename T> 
bool Read(SIZE_T address, T& buffer) {
    SIZE_T bytesRead;
    ReadProcessMemory(processHandle, (LPCVOID)address, &buffer, sizeof(T), &bytesRead);
    return bytesRead == sizeof(T);
}

std::string TitleGen(int num) {
    std::string nameoftitle;
    for (int i = 0; i < num; i++)
        nameoftitle += rand() % 255 + 1;
    return nameoftitle;
}

void rgb() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    while (true) {
        for (int i = 0; i < 16; ++i) {
            SetConsoleTextAttribute(hConsole, i);
            std::cout << ascii << std::endl;
            std::cout << "Now using maehop." << std::endl;
            std::cout << "F4 for BHOP" << std::endl;
            std::cout << "BHOP = " + std::to_string(toggleBhop) << std::endl;
            Sleep(20);
            system("CLS");
            if (i == 7) ++i;
            if (i == 15) i = 0;
        }
        Sleep(200);
    }
}
