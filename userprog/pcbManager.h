#ifndef PCBMANAGER_H
#define PCBMANAGER_H

#include "../filesys/useropenfile.h"
#include "pcb.h"
class pcb;

#define MAX_PCB 20

class pcbManager
{

    public: 
	pcbManager();
	void assignPCB(pcb *input);
	void removePCB(int pid);
	int getNumPCB();
	bool validPID(int pid); 
	void setParentNull();
	pcb* getThisPCB(int pcbID);
        UserOpenFile* getUOFs(char* fileName);
        
    private:
    	pcb *pcbArray[MAX_PCB];
	bool usage[MAX_PCB];
	int pcb_count;
    int currPid;
};
#endif
