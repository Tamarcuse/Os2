#include "types.h"
#include "stat.h"
#include "user.h"
#include "uthread.h"

//test 

void join_func(void *other_thread){
	printf(2,"tid(%d) waiting to tid(%d) to finish\n", uthread_self(), (int)other_thread);
	uthread_join((int)(other_thread));
	printf(2,"tid(%d) finish to wait to tid(%d).. exiting..\n", uthread_self(), (int)other_thread);
}

void justWaitInLoop(void *ign){
	int i = 0;
	for (;i < 5000000; i++);
}

void join_tst(int num){
	int i = 0;
	int tmp = uthread_create((void*)justWaitInLoop, (void*) 0);
	
	for (;i < num; i++){
		tmp = uthread_create((void*)join_func, (void*) tmp);
	}
	
}

void printme(int a){    
    long i = 0;
    for (; i < 50000000; i++)
        if (i% 5000000 == 0){
            printf(2,"tid: %d says hello %d (%d/10)\n", uthread_self(), a, 1 + i/5000000);
        }
		printf(2,"tid: %d finished-------------------------------\n", uthread_self(), a, 1 + i/5000000);
}
void aLotOfTheadsRunning(int num){
	long i = 1;
    for (; i <= num; i++){
        uthread_create((void*)(printme), (void*)(i + 100));
    }
}    

int main(int argc, char *argv[]){
    uthread_init();
    //join_tst(10);
	
	
	aLotOfTheadsRunning(20);
	
	
	//check for to much threads
	aLotOfTheadsRunning(MAX_UTHREADS+1);
	
    while(1);
  return -1;
}