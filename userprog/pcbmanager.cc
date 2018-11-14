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