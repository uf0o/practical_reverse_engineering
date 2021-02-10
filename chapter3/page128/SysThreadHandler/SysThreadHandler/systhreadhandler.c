#include "ntddk.h"
#include "common.h"

void SysThreadStartSystem(PVOID StartContext);
void SysThreadStartIOCTLHandler(PVOID StartContext);
void SysThreadUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS SysThreadCreateClose(PDEVICE_OBJECT, PIRP Irp);
NTSTATUS SysThreadDeviceControl(PDEVICE_OBJECT, PIRP Irp);


NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {

	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status;
	PDEVICE_OBJECT deviceObject = NULL;

	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\SysThreadHandler");
	UNICODE_STRING symName = RTL_CONSTANT_STRING(L"\\??\\SysThreadHandler");


	status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &deviceObject);

	if (!NT_SUCCESS(status))
	{
		DbgPrint("Device object not created.\n");
		return status;
	}

	DriverObject->MajorFunction[IRP_MJ_CREATE] = SysThreadCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = SysThreadCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SysThreadDeviceControl;
	DriverObject->DriverUnload = SysThreadUnload;

	status = IoCreateSymbolicLink(&symName, &devName);

	if (!NT_SUCCESS(status))
	{
		DbgPrint("Symbolic link not created.\n");
		IoDeleteDevice(deviceObject);
	}

	return status;
}

void SysThreadUnload(PDRIVER_OBJECT DriverObject) {
	UNICODE_STRING symName = RTL_CONSTANT_STRING(L"\\??\\SysThreadHandler");
	IoDeleteSymbolicLink(&symName);
	IoDeleteDevice(DriverObject->DeviceObject);
}


NTSTATUS SysThreadCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS SysThreadDeviceControl(PDEVICE_OBJECT deviceObject,PIRP Irp)
	{
		PIO_STACK_LOCATION irpSp;
		NTSTATUS status = STATUS_SUCCESS, threadStatus;
		HANDLE hThreadA, hThreadB;

		UNREFERENCED_PARAMETER(deviceObject);

		irpSp = IoGetCurrentIrpStackLocation(Irp);

		if (irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SYSTHREAD_METHOD_BUFFERED)
		{
			// NULL ProcessHandle = System Thread created in SYSTEM(4) process context.
			threadStatus = PsCreateSystemThread(&hThreadA,THREAD_ALL_ACCESS,NULL,NULL,NULL,(PKSTART_ROUTINE)SysThreadStartSystem,NULL);

			if (!NT_SUCCESS(threadStatus))
			{
				DbgPrint("PsCreateSystemThread with NULL parameter failed.\n");
			}

			ZwClose(hThreadA);

			// ProccessHandle of caller process retrieved via NtCurrentProcess() macro = System Thread in Caller Process context.
			threadStatus = PsCreateSystemThread(&hThreadB,THREAD_ALL_ACCESS,NULL,NtCurrentProcess(),NULL,(PKSTART_ROUTINE)SysThreadStartIOCTLHandler,NULL);

			if (!NT_SUCCESS(threadStatus))
			{
				DbgPrint("PsCreateSystemThread with NtCurrentProcess parameter failed.\n");
			}

			ZwClose(hThreadB);
		}
		else
		{
			status = STATUS_INVALID_DEVICE_REQUEST;
		}

		Irp->IoStatus.Status = status;
		Irp->IoStatus.Information = 0;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return status;
	}

VOID SysThreadStartSystem(PVOID StartContext){
	UNREFERENCED_PARAMETER(StartContext);
	DbgPrint("SysThreadStartSystem has been called.\n");
	PsTerminateSystemThread(STATUS_SUCCESS);
}

VOID SysThreadStartIOCTLHandler(PVOID StartContext) {
	UNREFERENCED_PARAMETER(StartContext);
	DbgPrint("SysThreadStartIOCTLHandler has been called.\n");
	PsTerminateSystemThread(STATUS_SUCCESS);
}
