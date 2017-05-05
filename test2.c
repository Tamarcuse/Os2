#include "types.h"
#include "user.h"
#include "stat.h"
#include "uthread.h"



typedef void (*sighandler_t)(int);

void
test(int sigNum){
 printf(1,"\n=======================Signal Handler===================================\n Process id:  %d  Signal number: %d \n\n", getpid(),sigNum);
}
void
test1(){
    
    int j;
printf(1,"------------------TestEx1----------------- \n");

sighandler_t handler=(sighandler_t)test;
for(int i=0;i<32;i++){
 printf(1,"test  for1: i=%d\n",i);
 signal(i,handler);
}

for(int j=0;j<32;j++){
 sigsend(getpid(),j);
}
for(j=0; j<15; j++)
	  sleep(1);




}
void
test2h(int sigNum){
 printf(1,"\n=======================ALARM===================================\n Process id:  %d  Signal number: %d \n\n", getpid(),sigNum);
}

void
test2(){
alarm(15);
sleep(16);


}


void tthread1(void *arg) {
  int i;
   // printf(1, "heeeerrrrrreeee \n");
  //uthread_join(1);
    for(i = 0; i < 100; i++) {
    printf(1, "thread1: %d \n",i);
  }
  uthread_sleep(2000);

  for(i = 100; i < 200; i++) {
    printf(1, "thread1: %d \n",i);
  }
}

void tthread2(void *arg) {
  int i;
   // printf(1, "heeeerrrrrreeee \n");
  //uthread_join(1);
    for(i = 0; i < 100; i++) {
    printf(1, "thread2: %d \n",i);
  }
  uthread_sleep(200);

  for(i = 100; i < 200; i++) {
    printf(1, "thread2: %d \n",i);
  }
}

void tthread3(void *arg) {
  int i;
   // printf(1, "heeeerrrrrreeee \n");
  //uthread_join(1);
    for(i = 0; i < 100; i++) {
    printf(1, "thread1: %d \n",i);
  }

}



int
test4(){
printf(1,"------------------TestEx2----------------- \n");

uthread_init();

printf(1,"------------------TestEx12----------------- \n");

 printf(1,"test4-arg:%p\n",(void*)100);
            printf(1,"test4-start_func:%p\n",tthread1);

int t2 = uthread_create(tthread1,(void*)100);

int t3 = uthread_create(tthread2,(void*)100);
// printf(1,"------------------TestEx13----------------- \n");

int t4 = uthread_create(tthread3,(void*)100);   
printf(1,"t2 is: %d\n t3 is: %d \n t4 is: %d\n", t2,t3,t4);

printf(1,"t2 is: %d\n", t2);


while(1);
printf(1,"------------------TestEx12-end----------------- \n");


exit();
}

int
test5(){

    uthread_init();

    printf(1,"------------------TestEx12----------------- \n");

    uthread_create(tthread1,(void*)100);   


    while(1);
    printf(1,"------------------TestEx12-end----------------- \n");


    exit();
}





int
main(int argc, char *argv[]){
//test4();
test4();
exit();
}
