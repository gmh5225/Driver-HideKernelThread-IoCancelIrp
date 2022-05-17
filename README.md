# Driver-HideKernelThread-IoCancelIrp
not my code. only for saving
https://bbs.pediy.com/thread-272795.htm

## Principle
Set the thread entry address to IoCancelIrp and the context to the irp you created.
Then the thread will call IoCancelIrp. when called, will perform the operation of calcelling the IRP.
This will then call the IRP->CancelRoutine. so we just need to set our own thread entry address to IRP->CancelRoutine.

## How to detect?
- Detecting the start entry address which is IoCancelIrp.
- Inserting APC to system threads and walk the stack.
