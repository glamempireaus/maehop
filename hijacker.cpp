#include <Windows.h> 
#include <iostream>
#include <TlHelp32.h>
#include <string>
#include "hijacker.h"

// Initializes the OBJECT_ATTRIBUTES structure.
OBJECT_ATTRIBUTES InitializeObjectAttributes(PUNICODE_STRING name, ULONG attributes, HANDLE hRoot, PSECURITY_DESCRIPTOR security)
{
    OBJECT_ATTRIBUTES objectAttr;
    objectAttr.Length = sizeof(OBJECT_ATTRIBUTES);
    objectAttr.ObjectName = name;
    objectAttr.Attributes = attributes;
    objectAttr.RootDirectory = hRoot;
    objectAttr.SecurityDescriptor = security;
    return objectAttr;
}

bool IsHandleValid(HANDLE handle)
{
    return handle && handle != INVALID_HANDLE_VALUE;
}

void CleanupResources(SYSTEM_HANDLE_INFORMATION*& hInfo, HANDLE& procHandle)
{
    delete[] hInfo;
    hInfo = nullptr;
    if (IsHandleValid(procHandle))
    {
        CloseHandle(procHandle);
        procHandle = nullptr;
    }
}

HANDLE HijackExistingHandle(DWORD dwTargetProcessId)
{
    SYSTEM_HANDLE_INFORMATION* hInfo = nullptr;
    HANDLE procHandle = nullptr;
    HANDLE HijackedHandle = nullptr;

    HMODULE Ntdll = GetModuleHandleA("ntdll");
    if (!Ntdll) return nullptr;

    // Get required functions from ntdll
    _RtlAdjustPrivilege RtlAdjustPrivilege = (_RtlAdjustPrivilege)GetProcAddress(Ntdll, "RtlAdjustPrivilege");
    _NtQuerySystemInformation NtQuerySystemInformation = (_NtQuerySystemInformation)GetProcAddress(Ntdll, "NtQuerySystemInformation");
    _NtDuplicateObject NtDuplicateObject = (_NtDuplicateObject)GetProcAddress(Ntdll, "NtDuplicateObject");
    _NtOpenProcess NtOpenProcess = (_NtOpenProcess)GetProcAddress(Ntdll, "NtOpenProcess");

    if (!RtlAdjustPrivilege || !NtQuerySystemInformation || !NtDuplicateObject || !NtOpenProcess)
    {
        CleanupResources(hInfo, procHandle);
        return nullptr; // Failed to load required functions.
    }

    BOOLEAN OldPriv;
    RtlAdjustPrivilege(SeDebugPriv, TRUE, FALSE, &OldPriv);

    OBJECT_ATTRIBUTES objAttributes = InitializeObjectAttributes(nullptr, 0, nullptr, nullptr);
    CLIENT_ID clientID = {};

    DWORD size = sizeof(SYSTEM_HANDLE_INFORMATION);
    hInfo = new (std::nothrow) SYSTEM_HANDLE_INFORMATION[size];

    if (!hInfo)
    {
        CleanupResources(hInfo, procHandle);
        return nullptr; // Failed memory allocation.
    }

    NTSTATUS status;
    do
    {
        delete[] hInfo;
        size *= 1.5;
        hInfo = new (std::nothrow) SYSTEM_HANDLE_INFORMATION[size];

        if (!hInfo)
        {
            CleanupResources(hInfo, procHandle);
            return nullptr; // Failed memory allocation.
        }

    } while ((status = NtQuerySystemInformation(SystemHandleInformation, hInfo, size, nullptr)) == STATUS_INFO_LENGTH_MISMATCH);

    if (!NT_SUCCESS(status))
    {
        CleanupResources(hInfo, procHandle);
        return nullptr; // Failed NtQuerySystemInformation.
    }

    for (unsigned int i = 0; i < hInfo->HandleCount; i++)
    {
        if (!IsHandleValid((HANDLE)hInfo->Handles[i].Handle)) continue;
        if (hInfo->Handles[i].ObjectTypeNumber != ProcessHandleType) continue;

        clientID.UniqueProcess = (HANDLE)hInfo->Handles[i].ProcessId;
        if (IsHandleValid(procHandle))
        {
            CloseHandle(procHandle);
        }
        status = NtOpenProcess(&procHandle, PROCESS_DUP_HANDLE, &objAttributes, &clientID);
        if (!IsHandleValid(procHandle) || !NT_SUCCESS(status)) continue;

        status = NtDuplicateObject(procHandle, (HANDLE)hInfo->Handles[i].Handle, GetCurrentProcess(), &HijackedHandle, PROCESS_ALL_ACCESS, 0, 0);
        if (!IsHandleValid(HijackedHandle) || !NT_SUCCESS(status)) continue;

        if (GetProcessId(HijackedHandle) == dwTargetProcessId)
        {
            CleanupResources(hInfo, procHandle);
            return HijackedHandle; // Found the target handle.
        }
        CloseHandle(HijackedHandle);
    }

    CleanupResources(hInfo, procHandle);
    return nullptr; // Handle not found.
}

