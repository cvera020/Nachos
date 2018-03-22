// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "memorymanager.h"

#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch 
    
  private:
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
					// address space 

//======================================
//MemoryManager variables and functions
//======================================
  public:
    static void InitMemoryManager();
    
    static TranslationEntry* AllocatePhysicalPages(int, int);  //allocates physical pages associated
                                                  //with a process
    
    static bool DeallocatePhysicalPages(TranslationEntry*, int);    //deallocates physical pages associated
                                                 //with a process
    
    static MemoryManager memMan[MaxVirtPages]; //array of MemoryManagers; each element 
                                                //represents page mappings for a thread
    static bool isPhysicalPageInUse[NumPhysPages];   //keep track of which physical pages are currently in use
    static int totalPhysicalPagesUsed;      //keeps track of the amount of currently mapped
                                            //physical pages
};

#endif // ADDRSPACE_H
