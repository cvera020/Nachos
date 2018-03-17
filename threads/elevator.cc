#include "copyright.h"
#include "system.h"
#include "synch.h"

int numberFloors;
bool isActive;
int numInElevator;

int totalPeople; //describes how many people have requested the elevator so far;

void
Elevator(int numFloors) {
    numberFloors = numFloors;
    isActive = false;
    totalPeople = 0;
}

void
ArrivingGoingFromTo(int atFloor, int toFloor) {
    ++totalPeople;
    printf("Person %d wants to go to floor %d from floor %d.", totalPeople, toFloor, atFloor);
    
    
    
    
    Thread *person = new Thread("forked thread");
}

// This method will run the thread logic for persons. The thread will sleep until
// the elevator reaches the person's level and is not full, in which case the thread
// wakes up and proceeds to completion 
void
RideElevator() {
    
    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

void
MoveElevator(int currFloor, toFloor) {
    for (int i = 0; i < 50; i++) {
    }
}