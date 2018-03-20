// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "machine.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
extern Machine* machine;

void
ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2);

    if (which == SyscallException) {
        if (type == SC_Yield || type == SC_Exit || type == SC_Join ||
                type == SC_Exec || type == SC_Fork) {
            machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
            machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
            machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4); //Add 4 bytes since this is instruction length
        }

        if (type == SC_Halt) {
            DEBUG('a', "Shutdown, initiated by user program.\n");
            DEBUG('u', "System Call: %d invoked Halt\n", currentThread->getPid());
            interrupt->Halt();
        } else if (type == SC_Yield) {
            DEBUG('u', "System Call: %d invoked Yield\n", currentThread->getPid());
            currentThread->setStatus(READY);
            currentThread->Yield();
        } else if (type == SC_Exit) {
            int exitCode = machine->ReadRegister(4);
            DEBUG('D', "Exit Code: %d\n", exitCode);
            
        } else if (type == SC_Join) {

        } else if (type == SC_Exec) {

        } else if (type == SC_Fork) {

        }
    } else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}
