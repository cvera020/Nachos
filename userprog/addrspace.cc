// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "machine.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif

bool AddrSpace::isPhysicalPageInUse[NumPhysPages];   //keep track of which physical pages are currently in use
int AddrSpace::totalPhysicalPagesUsed;
MemoryManager* AddrSpace::memMan[MaxVirtPages];

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AllocatePhysicalPage
//      Return a translation entry. Saves the information
//      about the translation mapping in a memory manager
//
//  amountReq   - number of pages to be mapped
//  pid         - pid of process to which the mapping will be associated to in
//              the memory manager
//----------------------------------------------------------------------

void
AddrSpace::AllocatePhysicalPages(TranslationEntry* en, int amountReq, int numCodePages, 
        int numDataPages, int numBssPages, int pid) {
    DEBUG('D', "Starting to allocate physical pages!\n");
    
    //check that the TranslationEntry pointer, en, is not null
    ASSERT(en != NULL);
    
    //check if there are enough unused physical pages
    ASSERT(totalPhysicalPagesUsed + amountReq <= NumPhysPages);
    
    //currently only support 1-1 mapping from pages to frames
    ASSERT (MaxVirtPages == NumPhysPages);
    
    //create virtual to physical page mappings
    int numContiguousOpen = 0;
    for (int i=0; i < NumPhysPages - amountReq; i++) {
        //check the number of pages that are contiguous from the ith page
        for (int j=i; j < amountReq + i; j++) {
            if (!isPhysicalPageInUse[i]) {
                numContiguousOpen++;
                if (numContiguousOpen == amountReq) {
                    break;
                }
            } else {
                break;
            }
        }
        
        if (numContiguousOpen == amountReq) {
            int virtPageIndex = 0;
            for (int j=i; j < numContiguousOpen + i; j++) {
                isPhysicalPageInUse[j] = true;
                en[virtPageIndex].virtualPage = virtPageIndex;
                en[virtPageIndex].physicalPage = j;
                en[virtPageIndex].valid = TRUE;
                en[virtPageIndex].use = FALSE;
                en[virtPageIndex].dirty = FALSE;
                en[virtPageIndex].readOnly = FALSE;
                virtPageIndex++;
            }
            
            //zero out the newly allocated physical memory
            DEBUG('D', "About to zero out physical page %d..\n", i);
            bzero((machine->mainMemory+i*PageSize), PageSize*amountReq);
            DEBUG('D', "Done zeroing out physical page %d..\n", i);
            
            break;
        } else {
            i += numContiguousOpen - 1;
            numContiguousOpen=0;
        }
    }
    
    ASSERT(numContiguousOpen == amountReq);
    
    //add mapping to memory manager array
    int memManPageAllocated = -1;
    for (int i=0; i < MaxVirtPages; i++) {
        if (memMan[i] == NULL) {
            memManPageAllocated = i;
            memMan[i] = new MemoryManager();
            memMan[i]->entries = en;
            memMan[i]->pid = pid;
            memMan[i]->numPagesMapped = amountReq;
            memMan[i]->numCodePages = numCodePages;
            memMan[i]->numDataPages = numDataPages;
            memMan[i]->numBssPages = numBssPages;
            
            DEBUG('R', "Loaded Program: [%d] code | [%d] data | [%d] bss\n", 
                    numCodePages, numDataPages, numBssPages);
            break;
        }
    }
    ASSERT(memManPageAllocated != -1);
    
    totalPhysicalPagesUsed += amountReq;
}

//----------------------------------------------------------------------
// DeallocatePhysicalPage
//      Deallocates physical memory associated with a process.
//----------------------------------------------------------------------

bool
AddrSpace::DeallocatePhysicalPages(int pid) {
    DEBUG('D', "Deallocate all physical pages associated with pid %d!\n", pid);
    for (int i=0; i < NumPhysPages; i++) {
        if (memMan[i] != NULL && memMan[i]->pid == pid) {
            isPhysicalPageInUse[i] = false;
            totalPhysicalPagesUsed -= memMan[i]->numPagesMapped;
            memMan[i]->entries = NULL;
            delete [] memMan[i]->entries;
            delete memMan[i];
            return true;
        }
    }
    return false;
}

//----------------------------------------------------------------------
// InitMemoryManager
//      Return a physical page to map a virtual page. If there are no
//      free physical pages, return -1
//----------------------------------------------------------------------

void
AddrSpace::InitMemoryManager() {
    for (int i=0; i < MaxVirtPages; i++) {
        isPhysicalPageInUse[i] = false;
    }
    totalPhysicalPagesUsed = 0;
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int size;
    
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    int numCodePages = divRoundUp(noffH.code.size, PageSize);
    int numDataPages = divRoundUp(noffH.initData.size, PageSize);
    int numBssPages = divRoundUp(noffH.uninitData.size, PageSize);
    numPages = numCodePages+numDataPages+numBssPages;
    size = numPages*PageSize;

    DEBUG('D', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
    
// first, set up the translation; physical pages will be initially zeroed out
    pageTable = new TranslationEntry[numPages];
    AllocatePhysicalPages(pageTable, numPages, numCodePages, numDataPages, numBssPages, currentThread->getPid());
    
// then, copy in the code and data segments into memory
    int physicalAddress;
    DEBUG('R', "Loaded Program: [%d] code | [%d] data | [%d] bss\n", 
                    noffH.code.size, noffH.initData.size, numBssPages);
    if (noffH.code.size > 0) {
        DEBUG('D', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
        physicalAddress = pageTable[noffH.code.virtualAddr].physicalPage;
        executable->ReadAt(&(machine->mainMemory[physicalAddress]),
			noffH.code.size, noffH.code.inFileAddr);
                
    }
    if (noffH.initData.size > 0) {
        DEBUG('D', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
        physicalAddress = pageTable[noffH.initData.virtualAddr].physicalPage;
        executable->ReadAt(&(machine->mainMemory[physicalAddress]),
			noffH.initData.size, noffH.initData.inFileAddr);
    }
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Deallocate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    DEBUG('D', "Deallocating all pages\n");
    DeallocatePhysicalPages(currentThread->getPid());
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

