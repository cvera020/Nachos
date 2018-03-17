// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"

//Uncomment below to enable semaphore synchronization for SimpleThread(int))
//#define HW1_SEMAPHORES

//Uncomment below to enable locks synchronization for SimpleThread(int))
#define HW1_LOCKS

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

int SharedVariable;
Semaphore* s;
Semaphore* barrier;

Lock* lock;

void SimpleThread(int which) {

#ifdef HW1_SEMAPHORES
    if (s == 0 && which == 0) {
        s = new Semaphore("testSemaphore", 0);
        barrier = new Semaphore("testBarrier", 0);
    } else if (which == 0) {
        barrier->P();
    } else {
        s->P();
    }
#endif

#ifdef HW1_LOCKS
    if (lock == 0 && which == 0) {
        lock = new Lock("testLock");
    }
    lock->Acquire();
#endif

    int num, val;
    for (num = 0; num < 5; num++) {
        val = SharedVariable;
        printf("*** thread %d sees value %d\n", which, val);

#ifdef HW1_LOCKS
        lock->Release();
#endif
        
        currentThread->Yield();
        
#ifdef HW1_LOCKS
        lock->Acquire();
#endif
        
        SharedVariable = val + 1;
        
#ifdef HW1_LOCKS
        lock->Release();
#endif
        currentThread->Yield();
    }
    val = SharedVariable;
    printf("Thread %d sees final value %d\n", which, val);

#ifdef HW1_SEMAPHORES
    if (which == 0) {
        s->V();
        barrier->P();
    } else {
        barrier->V();
        s->P();
    }
    delete s;
    s = 0;
    delete barrier;
    barrier = 0;
#endif

#ifdef HW1_LOCKS
    
#endif
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1() {
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest(int n) {
    if (n == 0) {
        return;
    }

    switch (testnum) {
        case 1:
            ThreadTest1();
            break;
        default:
            printf("No test specified.\n");
            break;
    }

    if (n > 0) {
        ThreadTest(--n);
    }
}

