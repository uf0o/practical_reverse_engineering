#include <Windows.h>
#include <stdio.h>
#include "..\SysThreadHandler\Common.h"


HANDLE hDevice = NULL;
BOOL DeviceRequest;
ULONG bytesReturned;

int Error(const char* msg) {
	printf("%s (%u)\n", msg, GetLastError());
	return 1;
}

int main() {
	HANDLE hDevice = CreateFile(L"\\\\.\\SysThreadHandler", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
		return Error("Failed to open device");

	DeviceRequest = DeviceIoControl(hDevice,(DWORD)IOCTL_SYSTHREAD_METHOD_BUFFERED,NULL,0,NULL,0,&bytesReturned,NULL);

	if (!DeviceRequest)
		return Error("Failed to open device");

	CloseHandle(hDevice);
	return 0;
}

