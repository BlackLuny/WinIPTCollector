#include <ntifs.h>
#include "../share/itf/ioctl_code.h"
#include "../share/itf/itf.h"
#include "ARCH.h"
#include "ConfigMain.h"

void* operator new(size_t size) {
	return ExAllocatePool(NonPagedPool, size);
}
void operator delete(void* addr, size_t size) {
	size = size;
	return ExFreePool(addr);
}

struct DevExtInfo {
	UNICODE_STRING symbolicName;
};

HANDLE g_serverPID = 0;
// 监控进程退出，清理内存
VOID ProcessMonitorCallBack(
	_In_ HANDLE ParentId,
	_In_ HANDLE ProcessId,
	_In_ BOOLEAN Create
)
{
	ParentId = ParentId;
	if (!Create) {
		// 进程即将退出
		if (ProcessId == g_serverPID) {
			// 清理资源
			StopPtMain();
			g_serverPID = 0;
		}
	}
}

void SetServerPID(HANDLE pid)
{
	g_serverPID = pid;
}

void AddProcessNotify()
{
	PsSetCreateProcessNotifyRoutine(ProcessMonitorCallBack, FALSE);
}
void RmvProcessNotify()
{
	PsSetCreateProcessNotifyRoutine(ProcessMonitorCallBack, TRUE);
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	PDEVICE_OBJECT pDev;
	RmvProcessNotify();
	KdPrint(("Enter DriverUnload\n"));
	pDev = DriverObject->DeviceObject;//由驱动对象得到设备对象

	DevExtInfo* pDevExt = (DevExtInfo*)pDev->DeviceExtension;
	//删除符号链接
	UNICODE_STRING pLinkName = pDevExt->symbolicName;
	auto status = IoDeleteSymbolicLink(&pLinkName);
	if (!NT_SUCCESS(status)) {
		KdPrint(("IoDeleteSymbolicLink fail\n"));
	}
	//删除设备对象
	IoDeleteDevice(pDev);
}

NTSTATUS CreateSymbolic(PDRIVER_OBJECT DriverObject)
{
	// 创建设备
	UNICODE_STRING deviceName;
	RtlInitUnicodeString(&deviceName, L"\\Device\\WinIPTCollecctor");
	PDEVICE_OBJECT pDev;
	DevExtInfo* pDevExt;
	auto status = IoCreateDevice(DriverObject, sizeof(DevExtInfo), &deviceName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pDev);
	if (!NT_SUCCESS(status)) {
		goto _Error_1;
	}
	pDevExt = (DevExtInfo*)pDev->DeviceExtension;

	// 创建符号链接
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\WinIPTCollecctor");
	pDevExt->symbolicName = symLinkName;
	status = IoCreateSymbolicLink(&symLinkName, &deviceName);
	if (!NT_SUCCESS(status)) {
		goto _Error_2;
	}
	return status;

_Error_2:
	IoDeleteDevice(pDev);
_Error_1:
	return status;
}


void DispatchForReadMSR(void* setupInfo, void* rst, unsigned int& rstLen)
{
	PtReadMsr* info = (PtReadMsr*)setupInfo;
	PtReadMsrRst* readRst = (PtReadMsrRst*)rst;
	readRst->rst = ARCH::ReadMsr(info->msrId);
	rstLen = sizeof(PtReadMsrRst);
}
void DispatchForSetup(void* setupInfo, void* rst, unsigned int& rstLen)
{
	if (g_serverPID == 0) {
		DbgPrint("Please set server pid!\n");
		return;
	}
	SetupPtMain(setupInfo, rst);
	rstLen = sizeof(PtSetupRst);
}

void DispatchForSetupServerPID(void* setupInfo, void* rst, unsigned int& rstLen)
{
	PtSetupServerPid* info = (PtSetupServerPid*)setupInfo;
	g_serverPID = (HANDLE)info->pid;
	(*(PtSetupServerPidRsp*)rst).rst = 0;
	rstLen = sizeof(PtSetupServerPidRsp);
}
NTSTATUS DefaultDispatchFunc(_In_ struct _DEVICE_OBJECT* DeviceObject,
	_Inout_ struct _IRP* Irp)
{
	DeviceObject = DeviceObject;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0; // bytes xfered
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS CleanUpDispatchFunc(_In_ struct _DEVICE_OBJECT* DeviceObject,
	_Inout_ struct _IRP* Irp)
{
	DeviceObject = DeviceObject;
	DbgPrint("Clean Up\n");
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0; // bytes xfered
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
NTSTATUS IRPDispatchFunc(_In_ struct _DEVICE_OBJECT* DeviceObject,
	_Inout_ struct _IRP* Irp)
{
	DeviceObject = DeviceObject;
	DbgPrint("Enter IRPDispatchFunc\n");
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	//得到输入缓冲区大小
	ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
	cbin = cbin;
	//得到输出缓冲区大小
	//ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;

	//得到IOCTRL码
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;

	//获取输入缓冲区，IRP_MJ_DEVICE_CONTROL的输入都是通过buffered io的方式
	char* inBuf = (char*)Irp->AssociatedIrp.SystemBuffer;
	inBuf = inBuf;
	// 操作输出缓冲区,输出缓冲区和输入缓冲区是一个缓冲区
	//UCHAR* OutputBuffer = (UCHAR*)Irp->AssociatedIrp.SystemBuffer;
	//设置实际操作输出缓冲区长度
	char outPutBuff[4096] = { 0 };
	unsigned int outputLen = 0;
	switch (code) {
	case IOCTL_SETUP_PT:
		DispatchForSetup((void*)inBuf, outPutBuff, outputLen);
		break;
	case IOCTL_SETUP_SERVER_PID:
		DispatchForSetupServerPID((void*)inBuf, outPutBuff, outputLen);
		break;
	case IOCTL_READ_MSR:
		DispatchForReadMSR((void*)inBuf, outPutBuff, outputLen);
		break;
	}
	memcpy(inBuf, outPutBuff, outputLen);
	// 完成IRP
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = outputLen; // bytes xfered
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	DbgPrint("Leave IRPDispatchFunc\n");
	return status;
}


extern "C" {

	NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegisterPath)
	{
		RegisterPath = RegisterPath;
		DriverObject->DriverUnload = DriverUnload;
		DriverObject->MajorFunction[IRP_MJ_CREATE] = DefaultDispatchFunc;
		//DriverObject->MajorFunction[IRP_MJ_CLEANUP] = CleanUpDispatchFunc;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IRPDispatchFunc;
		AddProcessNotify();
		CreateSymbolic(DriverObject);
		return STATUS_SUCCESS;
	}
}