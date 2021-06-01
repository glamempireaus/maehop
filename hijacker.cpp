#include <Windows.h> 
#include <iostream>
#include <TlHelp32.h>
#include <string>

#include "hijacker.h"

OBJECT_ATTRIBUTES InitObjectAttributes(PUNICODE_STRING name, ULONG attributes, HANDLE hRoot, PSECURITY_DESCRIPTOR security)
{
	OBJECT_ATTRIBUTES object;

	object.Length = sizeof(OBJECT_ATTRIBUTES);
	object.ObjectName = name;
	object.Attributes = attributes;
	object.RootDirectory = hRoot;
	object.SecurityDescriptor = security;

	return object;
}

SYSTEM_HANDLE_INFORMATION* hInfo;

HANDLE procHandle = NULL;
HANDLE hProcess = NULL;
HANDLE HijackedHandle = NULL;

DWORD GetPID(LPCSTR procName)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, false);
	if (hSnap && hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;

		ZeroMemory(procEntry.szExeFile, sizeof(procEntry.szExeFile)); 

		do
		{
			if (lstrcmpi(procEntry.szExeFile, procName) == NULL) {
				return procEntry.th32ProcessID;
				CloseHandle(hSnap);
			}
		} while (Process32Next(hSnap, &procEntry));
	}
}

bool IsHandleValid(HANDLE handle)
{
	if (handle && handle != INVALID_HANDLE_VALUE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CleanUpAndExit(LPCSTR ErrorMessage)
{
	delete[] hInfo;

	procHandle ? CloseHandle(procHandle) : 0;
	
	std::cout << ErrorMessage << std::endl;
		
	system("pause");
}
HANDLE HijackExistingHandle(DWORD dwTargetProcessId)
{
	HMODULE Ntdll = GetModuleHandleA("ntdll");

	_RtlAdjustPrivilege RtlAdjustPrivilege = (_RtlAdjustPrivilege)GetProcAddress(Ntdll, "RtlAdjustPrivilege");

	boolean OldPriv;

	RtlAdjustPrivilege(SeDebugPriv, TRUE, FALSE, &OldPriv);

	_NtQuerySystemInformation NtQuerySystemInformation = (_NtQuerySystemInformation)GetProcAddress(Ntdll, "NtQuerySystemInformation");

	_NtDuplicateObject NtDuplicateObject = (_NtDuplicateObject)GetProcAddress(Ntdll, "NtDuplicateObject");

	_NtOpenProcess NtOpenProcess = (_NtOpenProcess)GetProcAddress(Ntdll, "NtOpenProcess");

	OBJECT_ATTRIBUTES Obj_Attribute = InitObjectAttributes(NULL, NULL, NULL, NULL);
	
	CLIENT_ID clientID = { 0 };

	DWORD size = sizeof(SYSTEM_HANDLE_INFORMATION);

	hInfo = (SYSTEM_HANDLE_INFORMATION*) new byte[size];

	ZeroMemory(hInfo, size);

	NTSTATUS NtRet = NULL;

	do
	{
		delete[] hInfo;

		size *= 1.5;
		try
		{
			hInfo = (PSYSTEM_HANDLE_INFORMATION) new byte[size];
		}
		catch (std::bad_alloc)
		{
			CleanUpAndExit("Bad Heap Allocation");
		}
		Sleep(1);

	} while ((NtRet = NtQuerySystemInformation(SystemHandleInformation, hInfo, size, NULL)) == STATUS_INFO_LENGTH_MISMATCH);

	if (!NT_SUCCESS(NtRet))
	{
		CleanUpAndExit("NtQuerySystemInformation Failed");
	}

	for (unsigned int i = 0; i < hInfo->HandleCount; ++i)
	{
		static DWORD NumOfOpenHandles; 

		GetProcessHandleCount(GetCurrentProcess(), &NumOfOpenHandles);

		if (NumOfOpenHandles > 65)
		{
			CleanUpAndExit("Error Handle Leakage Detected"); 
		}

		if (!IsHandleValid((HANDLE)hInfo->Handles[i].Handle)) 
		{
			continue;
		}

		if (hInfo->Handles[i].ObjectTypeNumber != ProcessHandleType)
		{
			continue;
		}

		clientID.UniqueProcess = (DWORD*)hInfo->Handles[i].ProcessId;

		procHandle ? CloseHandle(procHandle) : 0;

		NtRet = NtOpenProcess(&procHandle, PROCESS_DUP_HANDLE, &Obj_Attribute, &clientID);
		if (!IsHandleValid(procHandle) || !NT_SUCCESS(NtRet))
		{
			continue;
		}

		NtRet = NtDuplicateObject(procHandle, (HANDLE)hInfo->Handles[i].Handle, NtCurrentProcess, &HijackedHandle, PROCESS_ALL_ACCESS, 0, 0);
		if (!IsHandleValid(HijackedHandle) || !NT_SUCCESS(NtRet))
		{
			continue;
		}

		if (GetProcessId(HijackedHandle) != dwTargetProcessId) 
		{
			CloseHandle(HijackedHandle);
			continue;
		}

		hProcess = HijackedHandle;
	
		break;
	}

	CleanUpAndExit("Success");

	return hProcess;
}


