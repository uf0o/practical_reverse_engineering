#include <ntifs.h>
#include <ntddk.h>
#include "WorkitemCommon.h"

#define DRIVER_PREFIX "WorkItemTest: "

void WorkitemUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS WorkitemCreateClose(PDEVICE_OBJECT, PIRP Irp);
NTSTATUS WorkitemDeviceControl(PDEVICE_OBJECT, PIRP Irp);

VOID KWorkItemRoutine(IN DEVICE_OBJECT* DeviceObject, IN PVOID Context)
{
	UNREFERENCED_PARAMETER(DeviceObject);


	PIO_WORKITEM pIoWorkItem;

	pIoWorkItem = (PIO_WORKITEM)Context;

	KdPrint((DRIVER_PREFIX"KWorkItemRoutine running from [%p][%p]  \n", PsGetCurrentProcessId(), PsGetCurrentThreadId()));

	IoFreeWorkItem(pIoWorkItem);

	
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING /*RegistryPath*/) {
	KdPrint((DRIVER_PREFIX "DriverEntry called\n"));
	DriverObject->DriverUnload = WorkitemUnload;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = WorkitemCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = WorkitemCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = WorkitemDeviceControl;

	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\workitem");
	PDEVICE_OBJECT DeviceObject;
	NTSTATUS status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status)) {
		KdPrint((DRIVER_PREFIX "Error in IoCreateDevice (0x%08X)\n", status));
		return status;
	}
	DeviceObject->Flags |= DO_BUFFERED_IO;

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\workitem");
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(DeviceObject);
		KdPrint((DRIVER_PREFIX "Error in IoCreateSymbolicLink (0x%08X)\n", status));
		return status;
	}

	return STATUS_SUCCESS;
}

void WorkitemUnload(PDRIVER_OBJECT DriverObject) {
	KdPrint((DRIVER_PREFIX "Unload called\n"));
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\Workitem");
	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS WorkitemCreateClose(PDEVICE_OBJECT, PIRP Irp) {
	NTSTATUS status = STATUS_SUCCESS;
	if (IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_CREATE) {
	}
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, 0);
	return status;
}

NTSTATUS WorkitemDeviceControl(PDEVICE_OBJECT device, PIRP Irp) {
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto status = STATUS_SUCCESS;


	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_WORKITEM:
	{
		auto data = (JunkData*)stack->Parameters.DeviceIoControl.Type3InputBuffer;
		if (data == nullptr) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}
		KeSetBasePriorityThread(KeGetCurrentThread(), 1);
		PIO_WORKITEM pWorkItem;
		// Work item
		pWorkItem = IoAllocateWorkItem(device);
		IoQueueWorkItem(pWorkItem, KWorkItemRoutine, DelayedWorkQueue, pWorkItem);
		break;
	}

	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, 0);
	return status;
}

