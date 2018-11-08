#ifndef TABLE_H
#define TABLE_H

#include "synch.h"
#define MAX_NUM_PROCESSES 100


class Table {
public:
    Table(int size);
    ~Table();
    int Alloc(); //Add process to array
    void *Get(int index);
    void Release(int index);
private:
    void** tab;
    bool* inUse;
    int tabsize;
    Lock* lock;
};

#endif /* TABLE_H */

