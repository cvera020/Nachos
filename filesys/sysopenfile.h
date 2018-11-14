#ifndef SYSOPENFILE_H
#define SYSOPENFILE_H

#include "utility.h"
#include "openfile.h"
#include "../userprog/syscall.h"

#define MAX_SYS_OPEN_FILES 32

class SysOpenFile {
  public:
    OpenFile* openFile;
    int fileId;
    char* fileName;
    int numUserProcesses;
};

#endif
