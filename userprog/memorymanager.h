#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "filesys.h"
#include "translate.h"
#include "machine.h"

#define MaxVirtPages            NumPhysPages

class MemoryManager {
  public:           
    int pid;    //multiple processes may be associated with this memory, but only
                //a parent process's pid will be stored here
    TranslationEntry* entries;   //Array of page entry pointers
    int numPagesMapped;             //Number of pages (TranslationEntry elements)
    
    MemoryManager();
    ~MemoryManager();
};

#endif // MEMORYMANAGER_H
