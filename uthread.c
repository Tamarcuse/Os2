#include "types.h"
#include "user.h"
#include "uthread.h"
#include "x86.h"

#define SIGALARM 14

struct uthread threadTable[MAX_UTHREADS];   // array of utherad structs
int next_tid = 1;                           // next tid to run
int runningThread;                          // current running thread index
int runnableThreads = 0;                    // Counter for runnables
int conSwitchCounter = 0;                   // Counter for context switches


const char* getState(uthread_state state) 
{
    switch (state) 
    {
        case MAINTHREAD: return "MAINTHREAD";
        case FREE: return "FREE";
        case RUNNING: return "RUNNING";
        case RUNNABLE: return "RUNNABLE";
        case SLEEP: return "SLEEP";
        case WAIT: return "WAIT";
        case BLOCKED: return "BLOCKED";
    }
    return 0;
}

void printThTable(){
    alarm(0);
    int i;
    printf(1, "\n\n $$$ Thread Table $$$:\n");
    for(i = 0; i < 10; i++){
        printf(1, "Thread %d: %s\n", i, getState(threadTable[i].state));
    }
    alarm(UTHREAD_QUANTA);
}

static void 
execute_thread(void (*start_func)(void *), void* arg){
  alarm(UTHREAD_QUANTA);
  start_func(arg);
  uthread_exit();
}

int
 uthread_init()
{
    printf(1, "** INIT USER LEVEL THREAD **\n");
    // set all threads state to FREE and make them wait for no thread.
    int i;
    for(i = 0; i < MAX_UTHREADS; ++i){
        threadTable[i].state = FREE;
        threadTable[i].waiting = -1;
        threadTable[i].sleep = -1;
    }

    // initialize main thread
    runningThread = 0;
    // set tid and stack to null
    threadTable[runningThread].tid = 0;
    threadTable[runningThread].stack = 0; 
    // main thread is ready to run
    threadTable[runningThread].state = MAINTHREAD;

    // register the SIGALARM to uthread_schedule func
    if(signal(SIGALARM, (sighandler_t)uthread_schedule) != 0){
        //signal error
        return -1;
    }

    // execute the alarm syscall with UTHREAD_QUANTA
    alarm(UTHREAD_QUANTA);
    return 0;
}

int 
uthread_create(void (*func)(void *), void* arg)
{
// disable thread switching
  alarm(0);
  
  printf(1, "Creating Thread\n");

  int i;

  // search for free thread to load
  for(i = 0; i < MAX_UTHREADS; ++i){
    if (threadTable[i].state == FREE){
      goto load_t;
    }
  }

  // case no free threads available
  // enable thread switching
  alarm(UTHREAD_QUANTA);
  return -1;

load_t:
  // next free tid to assign
  threadTable[i].tid = next_tid;
  // update next tid
  next_tid++;
  // allocate stack for thread and returns the stack top to .stack field
  threadTable[i].stack = malloc(STACK_SIZE);

  // load thread execute function to stack with arguments and return value
  *( (int*)(threadTable[i].stack + STACK_SIZE - 3*sizeof(int)) ) = 0; 
  *( (int*)(threadTable[i].stack + STACK_SIZE - 2*sizeof(int)) ) = (int)func;
  *( (int*)(threadTable[i].stack + STACK_SIZE - sizeof(int)) ) = (int)arg;

  // set thread state to RUNNABLE, ready to run
  threadTable[i].state = RUNNABLE;
  runnableThreads++;

  // set esp and ebp to null, while thread is not in RUNNING state yet
  threadTable[i].esp = 0;
  threadTable[i].ebp = 0;
  

  // enable thread switching
  printf(1, "creation method: returning.. \n");
  alarm(UTHREAD_QUANTA);

  return threadTable[i].tid;
}

void 
uthread_exit(void)
{
    alarm(0);
    int i;
    // disable thread switching

    printf(1, "#######################\n thread %d Exit\n#######################\n", runningThread);

    // deallocate thread memory
    if (threadTable[runningThread].stack != 0) {
        free(threadTable[runningThread].stack);
    }

    // deallocate thread from thread pool
    threadTable[runningThread].state = FREE;

    // if any thread is waiting for current thread, get them back to RUNNABLE state
    for(i = 0; i < MAX_UTHREADS; ++i){
        if (threadTable[i].waiting == runningThread){
            threadTable[i].waiting = -1;
            threadTable[i].state = RUNNABLE;
        }   
    }

    // Check if there are any other threadTable that can be switched to
    //printThTable();
    for(i = 0; i < MAX_UTHREADS; i++) {
        if (threadTable[i].state != FREE && threadTable[i].state != MAINTHREAD) {
            // found thread that is eligible to run.
            sigsend(getpid(), SIGALARM);
        }
    }
    // no ready to run threads, exit
    exit();
}

void 
uthread_schedule(void)
{
    //switch alarm(0) to cancel thread switching while scheduling 
    alarm(0);
    
    printf(1, "\nSchedule\n");
    int curTick = uptime();    
    int oldRunningThread = runningThread; // save running thread to change it's state after changing the running thread

    //save current thread esp and ebp
    asm("movl %%esp, %0;" : "=r" (threadTable[runningThread].esp));
    asm("movl %%ebp, %0;" : "=r" (threadTable[runningThread].ebp));

    //load the new thread

    //pick the next thread index
    
    int nextThread = (runningThread + 1) % MAX_UTHREADS;
    
    //Wake up sleeping Threads:
    int j;
    for(j = 0; j < MAX_UTHREADS; j++){
        if(threadTable[j].state == SLEEP){
            if(threadTable[j].sleep <= curTick){ // should wake up
               threadTable[j].state = RUNNABLE; 
               printf(1,"Thread %d woke up!\n", j);
            }
        }
    }
    
    int noRunnables = 1;
    int sleepers = 0;
    
    //if current thread was running, change its state to RUNNABLE
    if (threadTable[oldRunningThread].state == RUNNING) {
        threadTable[oldRunningThread].state = RUNNABLE;
    }
    
    for(j = nextThread ; j != oldRunningThread; j = (j + 1) % MAX_UTHREADS){    
        if(threadTable[j].state == RUNNABLE){
            runningThread = j;
            noRunnables = 0;
            goto FOUND_RUNNABLE;
        }
    }
    
    // check the last one the for loop skips:
    if(threadTable[oldRunningThread].state == RUNNABLE){
        runningThread = j;
        noRunnables = 0;
        goto FOUND_RUNNABLE;
    }
    
    //if noRunnables check for sleepers:
    if(noRunnables){
        for(j = 0; j < MAX_UTHREADS; j++){
            if(threadTable[j].state == SLEEP){
                sleepers = 1;
                break;
            }
        }
        
        // wake up the min sleeper:
        if(sleepers){ 
            int sleepTid = -1;  // tid of the thread to be woken
            int sleepTime; // min sleep time
            for(j = 0; j < MAX_UTHREADS; j++){
                if(threadTable[j].state == SLEEP){
                    sleepTime = threadTable[j].sleep;
                    sleepTid = j;
                    break;
                }
            }
            for(; j < MAX_UTHREADS; j++){
                if(threadTable[j].state == SLEEP && threadTable[j].sleep < sleepTime){
                    sleepTime = threadTable[j].sleep;
                    sleepTid = j;
                }
            }
            printf(1,"Thread %d woke up!\n", sleepTid);
            runningThread = sleepTid;
            noRunnables = 0;
            goto FOUND_RUNNABLE;
        }
    }
    
FOUND_RUNNABLE:                     

    //current switched thread is move to RUNNING mode, next to be execute
    threadTable[runningThread].state = RUNNING;
    printf(1,"\n runningThread is: %d\n", runningThread);
    

    //if esp is null, thread is running its first time, so we have to load a function to it 
    if(threadTable[runningThread].esp == 0) {
        asm("movl %0, %%esp;" : : "r" (threadTable[runningThread].stack + STACK_SIZE - 3*sizeof(int)));
        asm("movl %0, %%ebp;" : : "r" (threadTable[runningThread].stack + STACK_SIZE - 3*sizeof(int)));
        //jump to execute function to run the thread own function
        asm("jmp *%0;" : : "r" (execute_thread));
    } 
    else{
        //restore thread stack
        asm("movl %0, %%esp;" : : "r" (threadTable[runningThread].esp));
        asm("movl %0, %%ebp;" : : "r" (threadTable[runningThread].ebp));
    }
    
    alarm(UTHREAD_QUANTA); // reset alarm
}



int 
uthread_self(void)
{
  // return current running thread tid
  return threadTable[runningThread].tid;
}

int  
uthread_join(int tid)
{
    int thPosition, goodTid = 0;

    // if tid is not declared yet or is a negative number, error occured, return
    if(tid >= next_tid || tid < 0){
        return -1;
    }

    for(thPosition = 0; thPosition < MAX_UTHREADS; thPosition++){
        if(threadTable[thPosition].tid == tid){
            goodTid = 1;
            break;
        }
    }

    if(goodTid){
        int notFreeOrMain = threadTable[thPosition].state != FREE && threadTable[thPosition].state != MAINTHREAD;
        if(threadTable[runningThread].waiting == -1 && notFreeOrMain){
            threadTable[runningThread].waiting = tid;
            threadTable[runningThread].state = WAIT;
            printf(1, "\n   Thread %d is now waiting on Thread %d\n", runningThread, tid);
        }
        uthread_schedule();
        return tid;
    }
    else{
        return -1;
    }
}

int uthread_sleep(int ticks){
    int curTick = uptime();
    if(ticks < 1){
        return -1;
    }
    threadTable[runningThread].state = SLEEP;
    threadTable[runningThread].sleep = curTick + ticks;
    printf(1, "\n  Thread %d is now aSleep \n", runningThread);
    uthread_schedule();
    return 0;
}

// ********************************************************************** 
// ************************** BINARY SEMAPHORE **************************
// **********************************************************************

// struct binary_semaphore bSemTable[MAX_BSEM];   // array of semaphores 
// int next_sid = 1;
// 
// int enqueue(int tid, int sid){
//     int i, toEnqueue = -1;;
//     for(i = 0; i < MAX_BSEM; i++){
//         if(bSemTable[i].sid == sid){
//             toEnqueue = i;
//             break;
//         }
//     }
//     if (bSemTable[toEnqueue].size==MAX_UTHREADS){
//         return -1;
//     }
//     else{
//         bSemTable[toEnqueue].queue[bSemTable[toEnqueue].size++] = tid; //put the tid in the queue
//         return 0;
//     }
// }
// 
// int dequeue(int sid){
//     int i, toDeq = -1;
//     for(i = 0; i < MAX_BSEM; i++){
//         if(bSemTable[i].sid == sid){
//             toDeq = i;
//             break;
//         }
//     }    
//     if (bSemTable[toDeq].size == 0){
//         return 0;
//     }
//     int tid = bSemTable[toDeq].queue[0];
//     bSemTable[toDeq].size--;
// 
//     for(i = 0; i < bSemTable[toDeq].size; i++){     // fix queue
//         bSemTable[toDeq].queue[i] = bSemTable[toDeq].queue[i+1];
//     }
//     return tid;
// }
// 
// int bsem_alloc(){
//     int i, freeSpot = -1;
//     for(i = 0; i < MAX_BSEM; i++){
//         if(bSemTable[i].sid == 0){
//             freeSpot = i;
//         }
//     }
//     
//     if(freeSpot == -1){ return -1;}     // Max Semaphores
//     
//     //Create new Semaphore
//     struct binary_semaphore newSem;
//     newSem.sid = next_sid;
//     next_sid++;                         // update next sid
//     //initlock(&newSem.lock, "semLock");
//     newSem.size = 0;                    
//     newSem.value = 1; 
//     for(i = 0; i < MAX_UTHREADS; i++){
//         newSem.queue[i] = 0;
//     }
//     
//     bSemTable[freeSpot] = newSem;
//     
//     return next_sid-1;   
// }
// 
// void bsem_free(int sid){
//     int i, toFree = -1;
//     for(i = 0; i < MAX_BSEM; i++){
//         if(bSemTable[i].sid == sid){
//             toFree = i;
//             break;
//         }
//     }
//     if(toFree == -1) {return;}
//     struct binary_semaphore emptySem;
//     bSemTable[toFree] = emptySem;
//     return;
// }
// 
// void bsem_down(int sid){
//     int i, toDown = -1;
//     for(i = 0; i < MAX_BSEM; i++){
//         if(bSemTable[i].sid == sid){
//             toDown = i;
//             break;
//         }
//     }
//      if(toDown == -1) {return;}
//          
//      //acquire(&bSemTable[toDown].lock);
//      if(bSemTable[toDown].value <= 0){
//          threadTable[runningThread].state = BLOCKED;
//          enqueue(runningThread, toDown);
//      }
//      else{
//         bSemTable[toDown].value--;
//         alarm(UTHREAD_QUANTA);
//      }
// }
// 
// void bsem_up(int sid){
//     int i, toUp = -1;
//     for(i = 0; i < MAX_BSEM; i++){
//         if(bSemTable[i].sid == sid){
//             toUp = i;
//             break;
//         }
//     }
//      if(toUp == -1) {return;}
//      
//      int WakeTid = dequeue(bSemTable[toUp].sid);
//      if(WakeTid){ //If queue is not empty
//          for(i = 0; i < MAX_UTHREADS; i++){
//              if(threadTable[i].tid == WakeTid){
//                  threadTable[i].state = RUNNABLE;
//                  //release(&bSemTable[toUp].lock);
//                  uthread_schedule();
//                  break;
//              }
//          }
//      }
//      else{
//          bSemTable[toUp].value++;
//      }
//      //release(&bSemTable[toUp].lock);
// }




