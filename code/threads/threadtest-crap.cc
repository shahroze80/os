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
int testnum = 1;
#define MIN(a,b) (((a)<(b))?(a):(b));


// ALARM
Lock *mut;  
class Alarm { // Note: Multiple alarm instances is undefined behaviour!
  Timer *timer;
  List *list;
  public:
    Alarm() {
      mut = new Lock("semaphore mutex");
      list = new List();
      timer = new Timer(updateTimesAndWakeUp, reinterpret_cast<int>(list), false); 
    }
    static void updateTimesAndWakeUp(int _list) {
        IntStatus oldLevel = interrupt->SetLevel(IntOff);
        mut->Acquire();
        List* listlocal = reinterpret_cast<List*>(_list);
        int time;
        Thread* t = (Thread*) listlocal->SortedRemove(&time);
        while (t != NULL) {
            if (time <= stats->totalTicks)
                scheduler->ReadyToRun(t);
            else {
                listlocal->SortedInsert((void*) t, time);
                break;
            }
            if (listlocal->IsEmpty()) break;
            t = (Thread*) listlocal->SortedRemove(&time);
        }
        mut->Release();
        (void) interrupt->SetLevel(oldLevel);
    }


    void GoToSleep(int howLong) {
        IntStatus oldLevel = interrupt->SetLevel(IntOff);
        list->SortedInsert(reinterpret_cast<void*>(currentThread), howLong+stats->totalTicks);
        currentThread->Sleep();
        (void) interrupt->SetLevel(oldLevel);
    } 
};
  



//----------------------------------------------------------------------
// implementation of elevator problem
//----------------------------------------------------------------------

const int MAX_FLOORS=5;
const int CAPACITY=20; 
Semaphore* S1[MAX_FLOORS];    // shows number of user willing to get in at floor i
Semaphore* S2[MAX_FLOORS];    // shows number of users willing to get off at floor i

int people_waiting_at_from_floor[MAX_FLOORS];
int people_going_to_TO_floor_from_below[MAX_FLOORS];
int people_going_to_TO_floor_from_above[MAX_FLOORS];
Lock * floor_lock=new Lock("Floor lock");    // this lock will be used when modfying two arrays repsentating people

void elevator_thread(int which) {
    int curr_people=0;   

    printf("Elevator going from bottom to top\n");
    for (int currFloor=0; currFloor<MAX_FLOORS; currFloor++) {
        printf("Elevator is at floor %d\n",currFloor);
        // letting users out of elevator
        for(int i=0;i<people_going_to_TO_floor_from_below[currFloor];i++) {
            curr_people--;
            S2[currFloor]->V();
        }
        int space=CAPACITY-curr_people;
        int n=MIN(space,people_waiting_at_from_floor[currFloor]);
        // letting users into elevator
        for(int i=0;i<n;i++) {
            curr_people++;
            S1[currFloor]->V();
        }
        currentThread->Yield();
    }

    printf("Elevator going from top to bottom\n");
    for (int currFloor=MAX_FLOORS-1; currFloor>0; currFloor--) {
        printf("Elevator is at floor %d\n",currFloor);
        // letting users out of elevator
        for(int i=0;i<people_going_to_TO_floor_from_above[currFloor];i++) {
            S2[currFloor]->V();
            curr_people--;
        }
        int space=CAPACITY-curr_people;
        // letting users into elevator
        int n=MIN(space,people_waiting_at_from_floor[currFloor]);
        for(int i=0;i<n;i++) {
            S1[currFloor]->V();
            curr_people++;
        }
        currentThread->Yield();
    }
}

void GoingFromTo(int fromFloor,int toFloor) {
    floor_lock->Acquire();
    people_waiting_at_from_floor[fromFloor]++;
    if(fromFloor<toFloor) {
        people_going_to_TO_floor_from_below[toFloor]++;
    } else {
        people_going_to_TO_floor_from_above[toFloor]++;
    }
    floor_lock->Release();

    S1[fromFloor]->P();
    printf("%s got into elevator\n",currentThread->getName());
    S2[toFloor]->P();
    printf("%s got out of elevator\n",currentThread->getName());
}

void user1(int which) {
    GoingFromTo(0,1);
}
void user2(int which) {
    GoingFromTo(1,2);
}
void user3(int which) {
    GoingFromTo(2,4);
}
void user4(int which) {
    GoingFromTo(4,1);
}
void user5(int which) {
    GoingFromTo(0,1);
}

void ElevatorProblem() {
    DEBUG('t', "Entering ElevatorProblem");

    // initializing semaphores. assume initially there are no people in the system.
    for (int i=0;i<MAX_FLOORS;i++) {
        S1[i]=new Semaphore("fromFloor",0);
        S2[i]=new Semaphore("toFloor",0);
    }
        
    for (int i=0;i<MAX_FLOORS;i++) {
        people_waiting_at_from_floor[i]=0;
        people_going_to_TO_floor_from_below[i]=0;
        people_going_to_TO_floor_from_above[i]=0;
    }

    Thread *t2 = new Thread("User 1",1);
    t2->Fork(user1, 2);

    Thread *t3 = new Thread("User 2",1);
    t3->Fork(user2, 3);

    Thread *t4 = new Thread("User 3",1);
    t4->Fork(user3, 4);

    Thread *t5 = new Thread("User 4",1);
    t5->Fork(user4, 4);

    Thread *t6 = new Thread("User 5",1);
    t6->Fork(user5, 4);

    Thread *t1 = new Thread("Elevator",2);
    t1->Fork(elevator_thread, 1);
}

//----------------------------------------------------------------------
// Send and receive problem
// implementation of port sending and receiving
//----------------------------------------------------------------------


Port *p=new Port(100);      // a shared port object 

void Send(Port *port, int msg) {
  port->lock->Acquire();
  while(port->noReceiverCheck()) {
    // means there is no consumer of message yet. wait
    port->noReceiver->Wait(port->lock);
  }
  port->produceMsg(msg);
  port->noSender->Signal(port->lock);
  port->lock->Release();
}

void Receive(Port *port, int *msg) {
  port->lock->Acquire();
  while(port->noSenderCheck()) {
    // means there is no producer yet. wait
    port->noSender->Wait(port->lock);
  }
  *msg=port->consumeMsg();
  port->noReceiver->Signal(port->lock);
  port->lock->Release();
}

void sender_thread(int which) {
    printf("Sending message from thread: %s: %d\n",currentThread->getName(),123);
    Send(p,123);
}

void receiver_thread(int which) {
    int msg;
    Receive(p,&msg);
    printf("received message from thread %s: %d\n",currentThread->getName(),msg);
}

void SendReceiveProblem() {
    DEBUG('t', "Entering SendReceiveProblem");

    Thread *t1 = new Thread("forked thread 1",1);
    t1->Fork(sender_thread, 1);

    Thread *t2 = new Thread("forked thread 2",1);
    t2->Fork(receiver_thread, 2);

    Thread *t3 = new Thread("forked thread 3",1);
    t3->Fork(receiver_thread, 3);

    Thread *t4 = new Thread("forked thread 4",1);
    t4->Fork(sender_thread, 4);
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void SimpleThread(int which) {
    int num;

    for (num = 0; num < 5; num++) {
      printf("*** thread %d looped %d times\n", which, num);
      currentThread->Yield();
    }
}

Alarm *alam;
void print(int i){
  printf("Before Sleep\n");
  
  // alam->init();
  alam->GoToSleep(i);
  printf("After waking up\n");
}
// void testJoin(int i) {
//   while (true) {
//   printf("Hello agian\n");
// }

void
JoinerFn(Thread *joinee)
{
  printf("Waiting for the other thread to Join.\n");
  joinee->Join();

  currentThread->Yield();
  currentThread->Yield();
  currentThread->Yield();
  currentThread->Yield();
  currentThread->Yield();
  currentThread->Yield();

  printf("Other thread has  finished, do continue.\n");

  return;
}

void
JoineeFn()
{
  for (int i = 0; i < 5; i++) {
    printf("WORKING....\n");
    currentThread->Yield();
  }
}

void
JoinTest()
{
  Thread *joiner = new Thread("PARENT THREAD", false);
  Thread *joinee = new Thread("CHILD", true);

  // fork the two threads
  joiner->Fork((VoidFunctionPtr) JoinerFn, (int) joinee);
  joinee->Fork((VoidFunctionPtr) JoineeFn, 0);

  // finish this thead
  printf("Forking complete.\n");
}
////////////////
void
ThreadTest1()
{

    DEBUG('t', "Entering ThreadTest1");
    Thread *t4 = new Thread("forked thread 4",2);
    t4->Fork(print, 4);

    Thread *t1 = new Thread("forked thread 1",3);
    t1->Fork(print, 1);
    
    Thread *t2 = new Thread("forked thread 2",1);
    t2->Fork(print, 2);

    Thread *t3 = new Thread("forked thread 3",4);
    t3->Fork(print, 3);

    
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine. use -1 to specifiy which test to run
//----------------------------------------------------------------------



void
ThreadTest()
{
    // JoinTest();
    // // while (true); 
    // Timer* timer1 = new Timer(print, 4000, true); 
    // Timer* timer2 = new Timer(print, 1, false); 
    // // timer = new Timer(print, 0, false); 
    // // Timer* timer2 = new Timer(print, 0, false); 
    // // timer = new Timer(print, 0, false); 
    // printf("Size of int: %ld bytes\n",sizeof(List*));
    // // printf("Size of float: %ld bytes\n",sizeof(floatType));

    alam = new Alarm();
    ThreadTest1();
    Thread *t4 = new Thread("forked thread 4",3);
    t4->Fork(print, 5);
    Thread *t2 = new Thread("forked thread 4",3);
    t2->Fork(print, 2);
    Thread *t1 = new Thread("forked thread 4",3);
    t1->Fork(print, 3);
    Thread *t7 = new Thread("forked thread 4",3);
    t7->Fork(print, 5);
    Thread *t8 = new Thread("forked thread 4",3);
    t8->Fork(print, 7);
    Thread *t6 = new Thread("forked thread 4",3);
    t6->Fork(print, 0);
    // t4->Join();
    // t6->Join();
    // print();
  // switch (testnum) {
  //   case 1:
  //   ThreadTest1();
  //   break;
  //   case 2:
  //   SendReceiveProblem();
  //   break;
  //   case 3:
  //   ElevatorProblem();
  //   break;
  //   default:
  //   printf("No test specified.\n");
  //   break;
  // }
}

