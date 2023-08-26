#pragma once

// System Headers
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <ctime>
#include <string>
#include <sstream>
#include <fstream>
#include <amp.h>
#include <thread>

// Struct to hold game offsets
struct ouroffsets
{
    uintptr_t dwLocalPlayer = 0xD892CC;
    uintptr_t m_fFlags = 0x104; // TODO: Update this offset
    uintptr_t dwForceJump = 0x524BECC; // TODO: Update this offset
    DWORD procId;
} addr;

// Struct to represent a 3D vector
struct Vector3
{
    float x;
    float y;
    float z;
};
