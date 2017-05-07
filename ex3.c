
#include "types.h"
#include "user.h"
#include "stat.h"
#include "uthread.h"

//int sem;
struct counting_semaphore * sem;

typedef void (*sighandler_t)(int);


void tthread1(void *arg) {
  int i;
  for(i = 0; i < 100; i++)
    printf(1, "THREAD2: %d \n",i);
}
void tthread2(void *arg) {
  int i;
  for(i = 0; i < 100; i++) 
    printf(1, "THREAD3: %d \n",i);
}
void tthread3(void *arg) {
  int i;
  for(i = 0; i < 100; i++)
    printf(1, "THREAD4: %d \n",i);
}

void semthread2(void *arg) {
  int i;
  
 // bsem_down(sem);
  down(sem);
  printf(1, "----THREAD3 LOCKED--- \n");
  for(i = 0; i < 100; i++) 
    printf(1, "THREAD3: %d \n",i);
  printf(1, "----THREAD3 UNLOCKED--- \n");
 // bsem_up(sem);
  up(sem);

  for(i = 100; i < 200; i++) 
    printf(1, "THREAD3: %d \n",i);
  
}

void semthread3(void *arg) {
  int i;
  printf(1, "----THREAD4 TRYING TO LOCK--- \n");
//  bsem_down(sem);
  down(sem);
  printf(1, "----THREAD4 LOCKED--- \n");

  for(i = 0; i < 50; i++) 
    printf(1, "THREAD4: %d \n",i);
  
  printf(1, "----THREAD4 UNLOCKED--- \n");
//  bsem_up(sem);
  up(sem);
  
    for(i = 100; i < 200; i++) 
    printf(1, "THREAD4: %d \n",i);
}



int
test4(){

    uthread_init();
    printf(1,"############# Init completed ############# \n");

    int t2 = uthread_create(tthread1,(void*)100);
    int t3 = uthread_create(semthread2,(void*)100);
    int t4 = uthread_create(semthread3,(void*)100);   
    printf(1,"t2 id= %d, t3 id= %d, t4 id= %d\n", t2,t3,t4);


    while(1);
    exit();
}





int
main(int argc, char *argv[]){
 //   sem = bsem_alloc();
    sem = makeC_Sem(1);
    printf(1,"############# sem descriptor =%d ############# \n",sem);
    test4();
    exit();
}
