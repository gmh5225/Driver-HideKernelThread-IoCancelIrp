#include <ntddk.h>
 
VOID
DrvCreateThreadCancel(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
    PVOID StartContext = NULL;
    PKSTART_ROUTINE StartRoutine = NULL;
 
    if (pIrp)
    {
        IoReleaseCancelSpinLock(PASSIVE_LEVEL);
        StartContext = pIrp->UserBuffer;
        StartRoutine = (PKSTART_ROUTINE)pIrp->MdlAddress;
        IoFreeIrp(pIrp);
 
        if (StartRoutine)
        {
            StartRoutine(StartContext);
        }
    }
 
    PsTerminateSystemThread(STATUS_SUCCESS);
    return;
}
 
NTSTATUS
DrvCreateSystemThread(PHANDLE ThreadHandle,
                      ULONG DesiredAccess,
                      POBJECT_ATTRIBUTES ObjectAttributes,
                      HANDLE ProcessHandle,
                      PCLIENT_ID ClientId,
                      PKSTART_ROUTINE StartRoutine,
                      PVOID StartContext)
{
    PIRP pIrp = NULL;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
 
    do
    {
        pIrp = IoAllocateIrp(2, FALSE);
        if (!pIrp)
        {
            break;
        }
        IoSetNextIrpStackLocation(pIrp);
 
        pIrp->UserBuffer = StartContext;
        pIrp->MdlAddress = (PMDL)StartRoutine;
        IoSetCancelRoutine(pIrp, DrvCreateThreadCancel);
        Status = PsCreateSystemThread(ThreadHandle,
        DesiredAccess, ObjectAttributes, ProcessHandle,
        ClientId, (PKSTART_ROUTINE)IoCancelIrp, (PVOID)pIrp);
    } while (FALSE);
 
    if (NT_ERROR(Status) && pIrp)
    {
        IoFreeIrp(pIrp);
        pIrp = NULL;
    }
 
    return Status;
}
 
VOID
StartRoutine(PVOID StartContext)
{
    DbgPrint((PCSTR)StartContext);
}
 
NTSTATUS
DriverEntry(PDRIVER_OBJECT pDrvObj,
            PUNICODE_STRING pRegPath)
{
    HANDLE hThread = NULL;
    OBJECT_ATTRIBUTES Oba ={0};
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
 
    InitializeObjectAttributes(&Oba,
    NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
    Status = DrvCreateSystemThread(&hThread,
                                   THREAD_ALL_ACCESS,
                                   &Oba,
                                   NULL,
                                   NULL,
                                   StartRoutine,
                                   "$$StartContext");
    if (NT_SUCCESS(Status))
    {
        ZwClose(hThread);
    }
 
    return Status;
}
