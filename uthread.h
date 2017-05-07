#define UTHREAD_QUANTA 5
#define SIGALRM 14

// thread states
typedef enum  {FREE, RUNNING, RUNNABLE, SLEEP, WAIT, MAINTHREAD, BLOCKED} uthread_state;

#define STACK_SIZE  4096
#define MAX_UTHREADS  64
#define MAX_BSEM  128

// uthread required functions
int uthread_init();

void uthread_schedule(void);

int uthread_sleep(int ticks);

int uthread_create(void (*start_func)(void *), void* arg);

void uthread_exit();

int uthread_self();

int uthread_join(int tid);


struct uthread{
        int tid;                        // thread id
        int esp;                        // stack pointer      
        int ebp;                        // base pointer     
        char *stack;                    // thread stack   
        uthread_state state;            // thread state 
        int waiting;                    // number of threads this thread is waiting for
        int sleep;                      // the tick when the thread should stop sleeping
};

struct binary_semaphore{ 
    int sid;
    int locked;
    int queue[MAX_UTHREADS]; //queue of waiting tid's
    int size;
    int inUse;               //Indicator whether the sem was allocated
};

 struct counting_semaphore{
	int value;
	int s1;
	int s2;         
};

int bsem_alloc();
void bsem_free(int sid);
void bsem_down(int sid);
void bsem_up(int sid);
struct counting_semaphore * makeC_Sem(int value);
void down(struct counting_semaphore *sem);
void up(struct counting_semaphore *sem);


 

