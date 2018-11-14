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

static void ThreadFunc(int sctype);

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

        if (type == SC_Halt) {
            DEBUG('a', "Shutdown, initiated by user program.\n");
            DEBUG('R', "System Call: [%d] invoked Halt.\n", currentThread->getPid());
            interrupt->Halt();
        } else if (type == SC_Yield) {
            DEBUG('R', "System Call: [%d] invoked Yield.\n", currentThread->getPid());
            currentThread->setStatus(READY);
            currentThread->Yield();
        } else if (type == SC_Exit) {
            int exitCode = machine->ReadRegister(4);
            currentThread->setStatus(static_cast<ThreadStatus>(exitCode));
            
            DEBUG('R', "System Call: [%d] invoked Exit.\n", currentThread->getPid());
            DEBUG('R', "Process [%d] exits with [%d]\n", currentThread->getPid(), exitCode);
            
            // Set children's parent pointers to null
            List* children = currentThread->getChildren();
            Thread* child;
            while (children != NULL && !children->IsEmpty()) {
                child = (Thread*) children->Remove();
                DEBUG('D', "Exit Code: %d now does not have a parent\n", child->getPid());
                child->removeParent();
            }

            // Remove itself from parent thread, if one exists, and set parent's status to child's
            Thread* parent = currentThread->getParent();
            if (parent != NULL) {
             //   parent->setStatus(currentThread->getStatus());
                if (!parent->getChildren()->RemoveItem(currentThread)) {
                    DEBUG('D', "Error with RemoveItem() call\n");
                }
                DEBUG('D', "Exit Code: %d has now been removed from its parent %d\n",
                      currentThread->getPid(), parent->getPid());
            } else {
                DEBUG('D', "Exit Code: %d has no parent thread associated\n", currentThread->getPid());
            }

            // Deallocate the process memory and remove from the page table
            //addrSpace->DeallocatePhysicalPages(currentThread->getPid());

            DEBUG('D', "Exit: about to call finish()\n");
            // Finish the current thread
            currentThread->Finish();

        } else if (type == SC_Join) {
            DEBUG('R', "System Call: [%d] invoked Join.\n", currentThread->getPid());
            int childPid = machine->ReadRegister(4);
            int exitCode = 0;

            DEBUG('D', "Join: Child pid is %d\n", childPid);
            Thread* child = currentThread->getChild(childPid);
            DEBUG('D', "tried to get child\n");

            // If the child wasnt under this parent process, return an error
            if (child == NULL || childPid == currentThread->getPid()) {
                DEBUG('D', "No child found for pid %d\n");
                exitCode = -1;
            } else {
                DEBUG('D', "Join: Waiting for child %d to finish\n", child->getPid());
                // Else, keep on checking if the requested process is finished. if not, yield the current process
                while (child->getStatus() != ZOMBIE) {
                    currentThread->Yield();
                }
                // If the requested process finished, write the requested process exit id to register r2.
                exitCode = child->getStatus();
            }

            DEBUG('D', "Join: %d has finished the join call with exit code %d\n",
                  currentThread->getPid(), exitCode);

            machine->WriteRegister(2, exitCode);

        } else if (type == SC_Exec) {

        } else if (type == SC_Fork) {

            DEBUG('R', "System Call: [%d] invoked Fork.\n", currentThread->getPid());

            // PC register value
            int func = machine->ReadRegister(4);

            Thread* newThread = new Thread("child");
            currentThread->addChild(newThread);
            newThread->space = new AddrSpace(*currentThread->space);
   //         DEBUG('R', "Process [%d] Fork: start at address [%d] with [%d] pages memory", currentThread->getPid(), exitCode);

            // Save the user's state
            newThread->SaveUserState();

            // Set the registers for the new thread
     //       newThread->SetUserRegisterState(4,0);
            newThread->SetUserRegisterState(PCReg, func);
            newThread->SetUserRegisterState(NextPCReg, func + 4);

            DEBUG('D', "Fork: Forked from thread %d to %d\n", currentThread->getPid(), newThread->getPid());
            newThread->Fork(ThreadFunc, type);

            DEBUG('D', "Fork: pid after restore = %d\n", newThread->getPid());
            machine->WriteRegister(2, newThread->getPid());
        } else if (type == SC_Kill){
            DEBUG('D', "System Call: [%d] invoked Kill.\n", currentThread->getPid());
            int threadPid = machine->ReadRegister(4);
            int exitCode = 0;

            // Gets the thread associated with the pid
            Thread* thread = currentThread->getChild(threadPid);

            if (thread != NULL) {
                ExceptionHandler((ExceptionType)SC_Exit);
                machine->WriteRegister(2, 0);
                return;
            } else {
                machine->WriteRegister(2, -1);
            }

        }

	    if (type == SC_Yield || type == SC_Join || type == SC_Exec || type == SC_Fork) {
            machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
            machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
            machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4); //Add 4 bytes since this is instruction length
        }

    } else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}

static void ThreadFunc(int sctype)
{
    if (sctype == SC_Fork)
            currentThread->RestoreUserState();
    else if (sctype == SC_Exec) {
            if (currentThread->space != NULL) {
                currentThread->space->InitRegisters();
                currentThread->space->RestoreState();
            }
    }
    machine->Run();
}