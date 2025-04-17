#include <ntddk.h>
#include <wdm.h>
#include <intrin.h>
#include "Header.h"



void DriverExit(_In_ PDRIVER_OBJECT DriverObject);

NTSTATUS DriverCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS DriverDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

extern "C" NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegisteryPath) {
	UNREFERENCED_PARAMETER(RegisteryPath);
	NTSTATUS status;
	PDEVICE_OBJECT DeviceObject;

	DriverObject->DriverUnload = DriverExit;
	
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateClose;

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDeviceControl;

	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\Translator");
	status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status)) {
		KdPrint(("[Translator] Error while creating the device  , status is 0x%08X\n", status));
		return status;
	}

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\Translator");
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status)) {
		KdPrint(("[Translator] Error while creating the device , status is 0x%08X\n", status));
		return status;
	}

	return status;

}


bool  CheckPagingMode() {
	CR0 cr0;
	CR4 cr4;

	cr0.flags = __readcr0();
	cr4.flags = __readcr4();

	if (cr0.pg && !cr4.la57) return TRUE;
	KdPrint(("[Translator] Probably paging mode is la57 , not going to translate address"));
	return FALSE;
}

UINT64 Translate(UINT64 VirtualAddress) {
	CR3 cr3;
	cr3.flags = __readcr3();
	VIRTUAL_ADDRESS vAddr;
	vAddr.Address = VirtualAddress;
	PHYSICAL_ADDRESS pml4TablePhysical;

	PHYSICAL_ADDRESS pa = MmGetPhysicalAddress((PVOID)VirtualAddress);
	KdPrint(("PA: 0x%llx\n", pa.QuadPart));

	pml4TablePhysical.QuadPart = cr3.flags & PML4_TABLE_PHYSICAL_ADDRESS_MASK;
	PML4E_64* pml4TableVirtual = (PML4E_64*)MmGetVirtualForPhysical(pml4TablePhysical);

	if (!pml4TableVirtual) {
		KdPrint(("[Translator] Error getting virtual address of PML4 Table\n"));
		goto Error;
	}

	KdPrint(("[Translator] pml4 Table at %p\n", pml4TableVirtual));

	PML4E_64 pml4e = pml4TableVirtual[vAddr.pml4_index];

	PHYSICAL_ADDRESS pdptTablePhysical;
	pdptTablePhysical.QuadPart = pml4e.flags & PDPT_TABLE_PHYSICAL_ADDRESS_MASK;
	PDPTE_64* pdptTableVirtual = (PDPTE_64*)MmGetVirtualForPhysical(pdptTablePhysical);

	if (!pdptTableVirtual) {
		KdPrint(("[Translator] Error getting virtual address of PDPT Table\n"));
		goto Error;
	}
	PDPTE_64 pdpte = pdptTableVirtual[vAddr.pdpt_index];
	if (pdpte.ps) {
		return (pdpte.flags & PDPTE_1GB_64_PAGE_FRAME_NUMBER_FLAG) | (vAddr.Address & 0x3fffffff);
	}
	PHYSICAL_ADDRESS pdTablePhysical;
	pdTablePhysical.QuadPart = pdpte.flags & PD_TABLE_PHYSICAL_ADDRESS_MASK;
	PDE_64* pdTableVirtual = (PDE_64*)MmGetVirtualForPhysical(pdTablePhysical);
	if (!pdTableVirtual) {
		KdPrint(("[Translator] Error getting virtual address of PD Table\n"));
		goto Error;
	}
	PDE_64  pde = pdTableVirtual[vAddr.pd_index];
	if (pde.ps) {
		return (pde.flags & PDE_2MB_64_PAGE_FRAME_NUMBER_FLAG) | (vAddr.Address & 0x1fffff);
	}
	PHYSICAL_ADDRESS ptTablePhysical;
	ptTablePhysical.QuadPart = pde.flags & PT_TABLE_PHYSICAL_ADDRESS_MASK;
	PTE_64* ptTableVirtual = (PTE_64*)MmGetVirtualForPhysical(ptTablePhysical);
	if (!ptTableVirtual) {
		KdPrint(("[Translator] Error getting virtual address of PT Table\n"));
		goto Error;
	}
	PTE_64 pte = ptTableVirtual[vAddr.pt_index];
	return (pte.flags & PT_TABLE_PHYSICAL_ADDRESS_MASK) | (vAddr.offset);

	Error:
		return 0;
}

void  DriverExit(_In_ PDRIVER_OBJECT DriverObject) {

	NTSTATUS status;

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\Translator");
	status = IoDeleteSymbolicLink(&symLink);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Error while deleting symbolic link\n"));
	}
	IoDeleteDevice(DriverObject->DeviceObject);

}

NTSTATUS DriverCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


NTSTATUS DriverDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);


	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto status = STATUS_SUCCESS;

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_TRANSLATOR_TRANSLATE_ADDRESS: {
			auto data = (PINPUT)stack->Parameters.DeviceIoControl.Type3InputBuffer;
			if (data !=nullptr) {
				if (data->vAddress == 0) {
					status = STATUS_INVALID_PARAMETER;
					goto Finish;
				}
				if (!CheckPagingMode()) {
					status = STATUS_INVALID_PARAMETER;
					goto Finish;
				}
				POUTPUT pOutput= (POUTPUT)Irp->AssociatedIrp.SystemBuffer;
				pOutput->pAddress = Translate(data->vAddress);
			}else
				status = STATUS_INVALID_PARAMETER;
			break;
		}
		default:
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
	}

	Finish:
	Irp->IoStatus.Status = STATUS_SUCCESS;
	(NT_SUCCESS(status)) ? Irp->IoStatus.Information = sizeof(OUTPUT) : 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}