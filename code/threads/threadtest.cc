// threadtest.cc 
//  Simple test case for the threads assignment.
//
//  Create two threads, and have them context switch
//  back and forth between themselves by calling Thread::Yield, 
//  to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"
#define MIN(a,b) (((a)<(b))?(a):(b));
int testnum = 1;


//----------------------------------------------------------------------
// Implementation of Traffic problem
//----------------------------------------------------------------------

enum dir {FWD, RHS, LHS};
enum path {NS_, SN_, EW_, WE_};
class car
{
public:
    Thread *th = new Thread("", 5);
    dir d;
    path p;
    char *name;
    car(dir di, path pa, char* n) {
        th = new Thread("", 0);
        d = di;
        p = pa;
        name = n;
    }
    static void waitAtSignal(int i) { // give it semaphore and let it stop at 'signal'. later main thread puts in l
        (reinterpret_cast<Semaphore*>(i))->P();
    }
    void waitSig(Semaphore *s) {
        th->Fork(waitAtSignal, reinterpret_cast<int>(s));
    }
};

class road {
    public:
    List * L1;
    List * L2;
    road() {
        L1 = new List();
        L2 = new List();
    }
};

class Part7 {
    public:
    road *NS;
    road *SN;
    road *EW;
    road *WE;

    Semaphore *NSS;
    Semaphore *SNS;
    Semaphore *EWS;
    Semaphore *WES;

    void insertCar(car *c) {
        switch(c->p) { // insert at head but remove at tail -> emulate queue
            case NS_:
                if (c->d==RHS || c->d==FWD  || c->d == LHS)
                    NS->L1->Append(reinterpret_cast<void*>(c));
                else
                    NS->L2->Append(reinterpret_cast<void*>(c));
                c->waitSig(NSS);
                printf(c->name);
                printf(" stopped at signal\n");
                currentThread->Yield(); 
                break;
            case SN_:
                if (c->d==RHS || c->d==FWD || c->d == LHS)
                    SN->L1->Append(reinterpret_cast<void*>(c));
                else
                    SN->L2->Append(reinterpret_cast<void*>(c));
                c->waitSig(SNS);
                printf(c->name);
                printf(" stopped at signal\n");
                currentThread->Yield();                
                break;
            case EW_:
                if (c->d==RHS || c->d==FWD || c->d == LHS)
                    EW->L1->Append(reinterpret_cast<void*>(c));
                else
                    EW->L2->Append(reinterpret_cast<void*>(c));
                c->waitSig(EWS);
                printf(c->name);
                printf(" stopped at signal\n");
                currentThread->Yield();
                break;
            case WE_:
                if (c->d==RHS || c->d==FWD || c->d == LHS)
                    WE->L1->Append(reinterpret_cast<void*>(c));
                else
                    WE->L2->Append(reinterpret_cast<void*>(c));
                c->waitSig(WES);
                printf(c->name);
                printf(" stopped at signal\n");
                currentThread->Yield();
                break;
            default:
                int hh = 0;

        }
    }


    void runSimulation() {
        int havepassed = 0;
        void* cur_car;
        path  c = NS_;
        for (int i = 0; i < 2 ; i++) { // cycle through signals -> put in ready queue -> but only one thread at a time?
            switch(c) { // insert at head but remove at tail -> emulate queue
            case NS_:
                printf("\nNORTH TO SOUTH SIGNAL TURNED GREEN! \n\n");
                    while (cur_car = NS->L1->Remove(), (havepassed > 5 || cur_car != NULL)) {
                        if (havepassed > 5) break;
                        havepassed++;
                        NSS->V();
                        printf(reinterpret_cast<car*>(cur_car)->name);
                        printf(" left signal\n");
                    }
                    havepassed=0;
                    currentThread->Yield();
                    printf("\nNORTH TO SOUTH SIGNAL TURNED RED! \n\n");

            case SN_:
                    printf("\nSOUTH TO NORTH SIGNAL TURNED GREEN! \n\n");
                    while (cur_car = SN->L1->Remove(), (havepassed > 5 || cur_car != NULL)) {
                         if (havepassed > 5) break;
                        havepassed++;
                        SNS->V();
                        printf(reinterpret_cast<car*>(cur_car)->name);printf(" left signal\n");
                    }
                    havepassed=0;
                    currentThread->Yield();
                    printf("\nSOUTH TO NORTH SIGNAL TURNED RED! \n\n");
            case EW_:
                    printf("\nEAST TO WEST SIGNAL TURNED GREEN! \n\n");
                    while (cur_car = EW->L1->Remove(), (havepassed > 5 || cur_car != NULL)) {
                         if (havepassed > 5) break;
                        havepassed++;
                        EWS->V();
                        printf(reinterpret_cast<car*>(cur_car)->name);printf(" left signal\n");
                    }
                    havepassed=0;
                    currentThread->Yield();
                    printf("\nEAST TO WEST SIGNAL TURNED RED! \n\n");
            case WE_:
                    printf("\nWEST TO EAST SIGNAL TURNED GREEN! \n\n");
                    while (cur_car = WE->L1->Remove(), (havepassed > 5 || cur_car != NULL)) {
                         if (havepassed > 5) break;
                        havepassed++;
                        WES->V();
                        printf(reinterpret_cast<car*>(cur_car)->name);printf(" left signal\n");
                    }
                    currentThread->Yield();
                    havepassed=0;
                    printf("\nWEST TO EAST SIGNAL TURNED RED! \n\n");
            default:
                int jj = 0;

        }
        }
    }

    Part7() {
        NS = new road();
        SN = new road();
        EW = new road();
        WE = new road();

        NSS = new Semaphore("", 0);
        SNS = new Semaphore("", 0);
        EWS = new Semaphore("", 0);
        WES = new Semaphore("", 0);
    }
};

void
TrafficTest()
{
    Part7 *p7 = new Part7();
    p7->insertCar(new car(FWD, NS_, "Mclaren F1"));
    p7->insertCar(new car(FWD, SN_, "Ferrari 453 Italia"));
    p7->insertCar(new car(FWD, EW_, "Suzuki Mehran"));
    p7->insertCar(new car(FWD, WE_, "Honda NSX"));
    p7->insertCar(new car(FWD, NS_, "Nissan Maxima"));
    p7->insertCar(new car(FWD, EW_, "A"));
    p7->insertCar(new car(FWD, EW_, "B"));
    p7->insertCar(new car(FWD, WE_, "C"));
    p7->insertCar(new car(FWD, SN_, "D"));
    p7->insertCar(new car(FWD, NS_, "E"));
    p7->insertCar(new car(FWD, WE_, "F"));
    p7->insertCar(new car(FWD, WE_, "G"));
    p7->insertCar(new car(FWD, WE_, "H"));
    p7->insertCar(new car(FWD, SN_, "A_car_second_gen"));
    p7->insertCar(new car(FWD, SN_, "B_car_second_gen"));
    p7->insertCar(new car(FWD, EW_, "C_car_second_gen"));
    p7->insertCar(new car(FWD, EW_, "D_car_second_gen"));
    p7->insertCar(new car(FWD, EW_, "E_car_second_gen"));
    p7->insertCar(new car(FWD, EW_, "F_car_second_gen"));
    p7->insertCar(new car(FWD, NS_, "G_car_second_gen"));
    p7->insertCar(new car(FWD, SN_, "H_car_second_gen"));
    p7->insertCar(new car(FWD, NS_, "A_car_second_gen"));
    p7->insertCar(new car(FWD, SN_, "B_car_second_gen"));
    p7->insertCar(new car(FWD, NS_, "C_car_second_gen"));
    p7->insertCar(new car(FWD, NS_, "D_car_second_gen"));
    p7->insertCar(new car(FWD, NS_, "E_car_second_gen"));
    p7->insertCar(new car(FWD, NS_, "F_car_second_gen"));
    p7->insertCar(new car(FWD, WE_, "G_car_second_gen"));
    p7->insertCar(new car(FWD, WE_, "H_car_second_gen"));
    p7->runSimulation();

    // JoinTest();
  //   ThreadTest1();
  //   Thread *t4 = new Thread("forked thread 4",true);
  //   t4->Fork(print, 4);
  //   t4->Join();
  //   // print();
  // switch (testnum) {
  //   case 1:
  //   ThreadTest1();
  //   break;
  //   case 2:
  //   SendReceiveProblem();
  //   break;
  //   case 5:
  //   ElevatorProblem();
  //   break;
  //   case 6:
  //   TrainProblem();
  //   break;
  //   default:
  //   printf("No test specified.\n");
  //   break;
  }




//----------------------------------------------------------------------
// Implementation of Train problem
//----------------------------------------------------------------------

const int NUM_TRAIN_STATIONS=10;
const int TRAIN_CAPACITY=50; 
const int NUM_TRAINS=5;
// const int STATION_CAPACITY=100;

Station* stations[NUM_TRAIN_STATIONS];
Train* trains[NUM_TRAINS];

Train::Train(int _name,int _currStation) {
  curr_station=_currStation;
  num_passengers=0;
  name=_name;
}

bool Train::UnloadPassengers() {
    printf("Train %d arrived at station %d...\n",name,curr_station);
    bool a=false;   // to keep track of whether passengers were unloaded
    // passengers who have curr station as destination station should leave. 
    for (ListElement*p=passengers_riding->get_head();p!=NULL;p=p->next) {
        Passenger* passenger = (Passenger*)p->item;
        if(passenger && passenger->GetDestinationStation()==curr_station) {
            // signal the incoming semaphore of passenger's destination station
            stations[curr_station]->incoming->V();
            printf("  Passenger %d left train %d\n",passenger->name,name);
            a=true;
            p->item=NULL;
            num_passengers--;
        }
    }
    return a;
}

bool Train::getNextPassengers() {
    // take in passengers as min of space or people waiting at the curent station
    bool b=false;
    int space=TRAIN_CAPACITY - num_passengers;
    int people_waiting_at_current_station=stations[curr_station]->get_num_passengers_waiting();
    int n=MIN(space,people_waiting_at_current_station);
    if (n<=TRAIN_CAPACITY) {    // capacity constraint
        for(int i=0;i<n;i++) {
            // signals this many passengers to board into train
            stations[curr_station]->outgoing->V();
            b=true;
            // move these passengers from station queue into train queue
            Passenger*p=stations[curr_station]->remove_passenger_waiting_from_head();
            printf("  Passenger %d entered train %d\n",p->name,name);
            passengers_riding->Append((void*)p);
            num_passengers++;
        }
    }
    return b;
}   

void Train::MoveToNextStation() {
    curr_station=(curr_station+1)%NUM_TRAIN_STATIONS;
    // printf("next station is %d",curr_station);
}

int Train::get_num_passengers_riding() {
    return num_passengers;
}


Station::Station(int _name) {
  incoming=new Semaphore("incoming",0);    // coming from some other station to this station
  outgoing=new Semaphore("outgoing",0);   // going from this station to some other station
  num_passengers_waiting=0;
  add_passenger_lock=new Lock("Passenger lock");
  passengers_waiting=new List();
  // station_number=station_num;
  name=_name;
}

void Station::AddPassengerWaiting(Passenger * p) {
  add_passenger_lock->Acquire();
  passengers_waiting->Append(p);
  num_passengers_waiting++;    
  add_passenger_lock->Release();
}

void* Station::RemovePassengerWaiting() {
    return passengers_waiting->Remove();
}

Passenger* Station::remove_passenger_waiting_from_head() {
    Passenger* passenger = (Passenger*) passengers_waiting->Remove();
    num_passengers_waiting--;
    return passenger;
}

int Station::get_num_passengers_waiting() {
    // printf("Num passengers waiting at station %d is %d\n",name,num_passengers_waiting);
    return num_passengers_waiting;
}

Passenger::Passenger(int _name, int fromStation,int toStation) {
    from=fromStation;
    to=toStation;
    name=_name;
}
int Passenger::GetDestinationStation() {
  return to;
}

int Passenger::GetDepartureStation() {
  return from;
}

bool passengers_waiting_at_platforms() {
    for(int i=0;i<NUM_TRAIN_STATIONS;i++) {
        if (stations[i]->get_num_passengers_waiting() > 0) {
            return true;
        }
    }
    return false;
}

bool passengers_riding_trains() {
    for (int i=0;i<NUM_TRAINS;i++) {
      if(trains[i]) {
        if(trains[i]->get_num_passengers_riding() >0) {
            return true;
        }
      } else {
        printf("train was null\n");
      }
        
    }
    return false;
}


void passenger1(int which) {
    int from=1;
    int to=4;
    Passenger * p=new Passenger(1,from,to);
    stations[from]->AddPassengerWaiting(p);
    printf("Passenger %d is wants to go from stations %d to station %d\n",which,to,from);
    stations[from]->outgoing->P();
    // printf("Passenger %d entered train\n",which);
    stations[to]->incoming->P();
    // printf("Passenger %d left train\n",which);
}
void passenger2(int which) {
    int from=0;
    int to=3;
    Passenger * p=new Passenger(2,from,to);
    stations[from]->AddPassengerWaiting(p);
    printf("Passenger %d is wants to go from stations %d to station %d\n",which,to,from);
    stations[from]->outgoing->P();
    // printf("Passenger %d entered train\n",which);
    stations[to]->incoming->P();
    // printf("Passenger %d left train\n",which);
}
void passenger3(int which) {
    int from=4;
    int to=8;
    Passenger * p=new Passenger(3,from,to);
    stations[from]->AddPassengerWaiting(p);
    printf("Passenger %d is wants to go from stations %d to station %d\n",which,to,from);
    stations[from]->outgoing->P();
    // printf("Passenger %d entered train\n",which);
    stations[to]->incoming->P();
    // printf("Passenger %d left train\n",which);
}
void passenger4(int which) {
    int from=9;
    int to=7;
    Passenger * p=new Passenger(4,from,to);
    stations[from]->AddPassengerWaiting(p);
    printf("Passenger %d is wants to go from stations %d to station %d\n",which,to,from);
    stations[from]->outgoing->P();
    // printf("Passenger %d entered train\n",which);
    stations[to]->incoming->P();
    // printf("Passenger %d left train\n",which);
}
void passenger5(int which) {
    int from=0;
    int to=9;
    Passenger * p=new Passenger(5,from,to);
    stations[from]->AddPassengerWaiting(p);
    printf("Passenger %d is wants to go from stations %d to station %d\n",which,to,from);
    stations[from]->outgoing->P();
    // printf("Passenger %d entered train\n",which);
    stations[to]->incoming->P();
    // printf("Passenger %d left train\n",which);
}

void train0(int train_num) {
    Train*p=new Train(train_num,train_num);
    trains[train_num]=p;
    while(passengers_waiting_at_platforms() || passengers_riding_trains()) {
        bool a=p->UnloadPassengers();
        bool b=p->getNextPassengers();
        if (!a && !b) {
            printf("\t...but did not stop\n");
        } 
        p->MoveToNextStation();
        currentThread->Yield();
    }
}

void train1(int train_num) {
    Train*p=new Train(train_num,train_num);
    trains[train_num]=p;
    while(passengers_waiting_at_platforms() || passengers_riding_trains()) {
        bool a=p->UnloadPassengers();
        bool b=p->getNextPassengers();
        if (!a && !b) {
            printf("\t...but did not stop\n");
        } 
        p->MoveToNextStation();
        currentThread->Yield();
    }
}

void train2(int train_num) {
    Train*p=new Train(train_num,train_num);
    trains[train_num]=p;
    while(passengers_waiting_at_platforms() || passengers_riding_trains()) {
        bool a=p->UnloadPassengers();
        bool b=p->getNextPassengers();
        if (!a && !b) {
            printf("\t...but did not stop\n");
        } 
        p->MoveToNextStation();
        currentThread->Yield();
    }
}

void train3(int train_num) {
    Train*p=new Train(train_num,train_num);
    trains[train_num]=p;
    while(passengers_waiting_at_platforms() || passengers_riding_trains()) {
        bool a=p->UnloadPassengers();
        bool b=p->getNextPassengers();
        if (!a && !b) {
            printf("\t...but did not stop\n");
        } 
        p->MoveToNextStation();
        currentThread->Yield();
    }
}

void train4(int train_num) {
    Train*p=new Train(train_num,train_num);
    trains[train_num]=p;
    while(passengers_waiting_at_platforms() || passengers_riding_trains()) {
        bool a=p->UnloadPassengers();
        bool b=p->getNextPassengers();
        if (!a && !b) {
            printf("\t...but did not stop\n");
        } 
        p->MoveToNextStation();
        currentThread->Yield();
    }
}



void TrainProblem() {
    DEBUG('t', "Entering Train Problem");

    // initializing stations
    for (int i=0;i<NUM_TRAIN_STATIONS;i++) {
        stations[i]=new Station(i);
    }

    // creating passenger threads
    Thread *p1 = new Thread("passenger",1);
    p1->Fork(passenger1, 1);
    Thread *p2 = new Thread("passenger",1);
    p2->Fork(passenger2, 2);
    Thread *p3 = new Thread("passenger",1);
    p3->Fork(passenger3, 3);
    Thread *p4 = new Thread("passenger",1);
    p4->Fork(passenger4, 4);
    Thread *p5 = new Thread("passenger",1);
    p5->Fork(passenger5, 5);

    Thread *t0 = new Thread("train",2);
    t0->Fork(train0, 0);

    Thread *t1 = new Thread("train",2);
    t1->Fork(train1, 1);

    Thread *t2 = new Thread("train",2);
    t2->Fork(train2, 2);

    Thread *t3 = new Thread("train",2);
    t3->Fork(train3, 3);

    Thread *t4 = new Thread("train",2);
    t4->Fork(train4, 4);

    // Thread *t5 = new Thread("Passenger 4",1);
    // t5->Fork(passenger4, 4);

    // Thread *t6 = new Thread("Passenger 5",1);
    // t6->Fork(passenger5, 4);

    // Thread *t1 = new Thread("Train",2);
    // t1->Fork(train_thread, 1);
}

//----------------------------------------------------------------------
// implementation of elevator problem
//----------------------------------------------------------------------

const int MAX_FLOORS=5;
const int EVELATOR_CAPACITY=20; 
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
        int space=EVELATOR_CAPACITY-curr_people;
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
        int space=EVELATOR_CAPACITY-curr_people;
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
// Implementation of Join
//----------------------------------------------------------------------


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


//----------------------------------------------------------------------
// Implementation of ALARM
//----------------------------------------------------------------------
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
  
Alarm *alam;

void print_alarm(int i) {
  printf("Thread %s going to sleep\n",currentThread->getName());
  alam->GoToSleep(i);
  printf("Thread %s woke up\n",currentThread->getName());
}

void AlarmClockExample() {
    alam = new Alarm();
    Thread *t1 = new Thread("forked thread 1",3);
    t1->Fork(print_alarm, 500000);
    Thread *t2 = new Thread("forked thread 2",2);
    t2->Fork(print_alarm, 2);
    Thread *t3 = new Thread("forked thread 3",1);
    t3->Fork(print_alarm, 3);
    // Thread *t4 = new Thread("forked thread 4",1);
    // t4->Fork(print_alarm, 5);
    // Thread *t5 = new Thread("forked thread 5",1);
    // t5->Fork(print_alarm, 7);
    // Thread *t6 = new Thread("forked thread 6",1);
    // t6->Fork(print_alarm, 2);
   
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

void sender_thread1(int which) {
    printf("Sending message from thread: %s: %d\n",currentThread->getName(),123);
    Send(p,123);
}

void receiver_thread1(int which) {
    int msg;
    Receive(p,&msg);
    printf("received message from thread %s: %d\n",currentThread->getName(),msg);
}

void sender_thread2(int which) {
    printf("Sending message from thread: %s: %d\n",currentThread->getName(),456);
    Send(p,456);
}

void receiver_thread2(int which) {
    int msg;
    Receive(p,&msg);
    printf("received message from thread %s: %d\n",currentThread->getName(),msg);
}

void SendReceiveProblem() {
    DEBUG('t', "Entering SendReceiveProblem");

    Thread *t2 = new Thread("forked thread 2",1);
    t2->Fork(receiver_thread1, 2);

    Thread *t3 = new Thread("forked thread 3",1);
    t3->Fork(receiver_thread2, 3);

    Thread *t1 = new Thread("forked thread 1",1);
    t1->Fork(sender_thread1, 1);

    Thread *t4 = new Thread("forked thread 4",1);
    t4->Fork(sender_thread2, 4);
}

//----------------------------------------------------------------------
// ThreadTest1
//  Set up a ping-pong between two threads, by forking a thread 
//  to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void SimpleThread(int which) {
    int num;

    for (num = 0; num < 5; num++) {
      printf("*** thread %d looped %d times\n", which, num);
      currentThread->Yield();
    }
}

void
ThreadTest1()
{

    DEBUG('t', "Entering ThreadTest1");
    Thread *t4 = new Thread("forked thread 4",2);
    t4->Fork(SimpleThread, 4);

    Thread *t1 = new Thread("forked thread 1",3);
    t1->Fork(SimpleThread, 1);
    
    Thread *t2 = new Thread("forked thread 2",1);
    t2->Fork(SimpleThread, 2);

    Thread *t3 = new Thread("forked thread 3",4);
    t3->Fork(SimpleThread, 3);

    
}

//----------------------------------------------------------------------
// ThreadTest
//  Invoke a test routine. use -1 to specifiy which test to run
//----------------------------------------------------------------------



void
ThreadTest()
{
  switch (testnum) {
    case 1:
    ThreadTest1();
    break;
    case 2:
    SendReceiveProblem();
    break;
    case 3:
    JoinTest();
    break;
    case 4:
    AlarmClockExample();
    break;
    case 5:
    ElevatorProblem();
    break;
    case 6:
    TrainProblem();
    break;
    case 7:
    TrafficTest();
    break;
    default:
    printf("No test specified.\n");
    break;
  }
}

