#pragma once

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <ctime>
#include <string>
#include <sstream>
#include <fstream>
#include <amp.h>
#include <thread>

struct ouroffsets
{
	uintptr_t dwLocalPlayer = 0xD892CC;

	// bhop
	uintptr_t m_fFlags = 0x104;
	uintptr_t dwForceJump = 0x524BECC;

	DWORD procId;
} addr;

struct Vector3
{
	float x;
	float y;
	float z;
};