#include "pcbManager.h"

pcbManager:: pcbManager() 
{
    int i=0;
    pcb_count = 0;
    for(i=0; i<32; i++)
    {
	usage[i]=false; 
    }
}

void pcbManager:: assignPCB(pcb *input)
{
    int i=0;
    for(i=0; i<32; i++)
    {
	if(usage[i] == false)
	{
	    DEBUG('g',"pcbManager.cc pcb has been assigned\n");
	    pcbArray[i] = input;
	    usage[i]=true;
	    pcb_count++;
        currPid = i;
	    break;
	}
    } 
}

void pcbManager:: removePCB(int pid)
{
    int i=0;
    for(i=0; i<32; i++)
    {
	if((usage[i]==true) && (pcbArray[i]->getID() == pid))
	{
		pcbArray[i] = NULL;
		usage[i] = false;
     		pcb_count--;
		break;
        }
     }
}

bool pcbManager:: validPID(int pid)
{
    int i=0;
    for(i=0; i<32; i++)
    {
	if((pcbArray[i]!=NULL) && (pcbArray[i]->getID() == pid))
	{
        currPid = i;
	    return true;
	}
    } 
    return false;
}

void pcbManager:: setParentNull()
{
    int i;
    for(i=0; i<32; i++)
    {
	if((usage[i]==true) && (pcbArray[i]->getParent() != NULL))
	{
		pcbArray[i]->setParent(NULL);
        currPid = i;
    	}
    } 

}

int pcbManager:: getNumPCB()
{
    return pcb_count;
}

pcb* pcbManager:: getThisPCB(int pcbID)
{
    int i;
    for(i=0; i<32; i++)
    {
	if((usage[i]==true) && (pcbArray[i]->getID() == pcbID))
	{
	    currPid = i;
        return pcbArray[i];
	    break;
	}
    }
    return NULL;
}

UserOpenFile* pcbManager:: getUOFs(char* fileName) {
    for (int i=0; i < MAX_PCB; i++) {
        if (pcbArray[i] != NULL && pcbArray[i]->getID() == currPid) {
            UserOpenFile* files = pcbArray[i]->userOpenFiles; //get the UOFs of the current process's PCB
            //find fileName in current process's open files
            for (int j=0; j < MAX_USER_OPEN_FILES; j++) {
                if (&files[j] != NULL && strcmp(files[j].fileName, fileName)) {
                    return &files[j];
                }
            }
        }
    }
    return NULL;
}

OpenFileId pcbManager::Open(char* fileName) {
	int i=0;
	int j=0;
	int sysFileFreeIndex = -1;
    OpenFile* file = fileSystem->Open(fileName);
    
	//If the file could not be opened and was not previously opened
    if (file == NULL) {
        return NULL;
    }
    
	//Get the open system file, if it exists
    SysOpenFile* sysFile = NULL;
	while (i++ < MAX_SYS_OPEN_FILES) {
        if ( &(sysOpenFiles[i]) != NULL && strcmp(sysOpenFiles[i].fileName, fileName) ) {
            sysFile = &(sysOpenFiles[i]);
			break;
        }
    }
	
	//if the open system file does not exist, create and allocate it in the sysOpenFiles array, if possible
	if (sysFile == NULL) {
		i=0;
		while (i++ < MAX_SYS_OPEN_FILES) {
            if (&(sysOpenFiles[i]) == NULL) {
                sysFileFreeIndex = i;
                break;
            }
        }
        
		//If there is no more room left for opening system files...
        if (sysFileFreeIndex == -1) {
            return -1;
        }
        
		//otherwise, if there is room, allocate
        sysFile = new SysOpenFile();
		sysFile->openFile = file;
        sysFile->fileId = sysFileFreeIndex;
        sysFile->fileName = fileName;
        sysFile->numUserProcesses = 1;
	} else {
		sysFile->numProcessesAccessing++;
	}
	
    
    //add the new file to the process's user file array
	i=0;
    for (i++ < MAX_PCB) {
        if (pcbArray[i] != NULL && pcbArray[i]->getID() == currPid) {
            UserOpenFile* uofs = pcbArray[i]->userOpenFiles;
			j=0;
            for (j++ < MAX_USER_OPEN_FILES) {
                if (&(uofs[j]) == NULL) {
                    UserOpenFile* userFile = new UserOpenFile();
                    userFile->fileName = fileName;
                    userFile->fileOffset = 0;
                    userFile->sysOpenFileIndex = sysFile->fileId;
                    return sysFile->fileId;
                }
            }
        }
    }
    
    //at this point, since the user file could not be allocated in the process's user file array, remove the system open file
    delete sysFile;
	sysFile = 0;
    return -1;
}

OpenFileId pcbManager::Read(char* fileName) {
    
}