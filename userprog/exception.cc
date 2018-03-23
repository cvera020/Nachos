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

#include <cstdio>
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "machine.h"
#include "thread.h"
#include "addrspace.h"


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
extern AddrSpace* addrSpace;

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
            DEBUG('R', "System Call: [%d] invoked Halt\n", currentThread->getPid());
            interrupt->Halt();
        } else if (type == SC_Yield) {
            DEBUG('R', "System Call: [%d] invoked Yield\n", currentThread->getPid());
            currentThread->setStatus(READY);
            currentThread->Yield();
        } else if (type == SC_Exit) {
            
            int exitCode = machine->ReadRegister(4);
            DEBUG('R', "System Call: [%d] invoked Exit with Exit Code &d\n", currentThread->getPid(), exitCode);

            // Set children's parent pointers to null
            List* children = currentThread->getChildren();
            Thread* child;
            while (children->IsEmpty()) {
                child = (Thread*) children->Remove();
                DEBUG('R', "Exit Code: %d now does not have a parent\n", child->getPid());
                child->removeParent();
            }

            // Remove itself from parent thread, if one exists, and set parent's status to child's
            Thread* parent = currentThread->getParent();
            if (parent != NULL) {
                parent->setStatus(currentThread->getStatus());
                if (!parent->getChildren()->RemoveItem(currentThread)) {
                    DEBUG('R', "Error with RemoveItem() call");
                }
                DEBUG('R', "Exit Code: %d has now been removed from its parent %d\n",
                      currentThread->getPid(), parent->getPid());
            } else {
                DEBUG('R', "Exit Code: %d has no parent thread associated\n", currentThread->getPid());
            }

            // Deallocate the process memory and remove from the page table
            addrSpace->DeallocatePhysicalPages(currentThread->getPid());

            // Finish the current thread
            currentThread->Finish();
<<<<<<< HEAD

=======
            
>>>>>>> 8e0d8869e96dc805e1dc7bd4a38345f01e6a1e8d
        } else if (type == SC_Join) {
            int childPid = machine->ReadRegister(4);
            int exitCode = 0;

            Thread* child = currentThread->getChild(childPid);

        } else if (type == SC_Exec) {

        } else if (type == SC_Fork) {

        }
    } else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}
