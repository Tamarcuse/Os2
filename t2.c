#include "types.h"
#include "user.h"
#include "stat.h"
#include "uthread.h"


void tthread1(void *arg) {
  int i;
  for(i = 0; i < 50; i++) {
    printf(1, " 1 %d \n",i);
  }
  printf(1, "SLEEP\n");
  sleep(5);
  for(i = 50; i < 100; i++) {
    printf(1, " 1 %d \n",i);
  }
}

void tthread2(void *arg) {
  int i;
  for(i = 0; i < 50; i++) {
    printf(1, " 2 %d \n",i);
  }
    printf(1, "SLEEP\n");
  sleep(5);
  for(i = 50; i < 100; i++) {
    printf(1, " 2 %d \n",i);
  }
}

void tthread3(void *arg) {
  int i;
  for(i = 0; i < 50; i++) {
    printf(1, " 3 %d \n",i);
  }
    printf(1, "SLEEP\n");
  sleep(5);
  for(i = 50; i < 100; i++) {
    printf(1, " 3 %d \n",i);
  }
}

int main(int argc, char *argv[]) {
    printf(1,"-=-=-=-=-=-=-=-=-=-=- TEXT USER LEVEL THREAD -=-=-=-=-=-=-=-=-=-=-\n");
    uthread_init();
    uthread_create(tthread1, (void*)555);
    uthread_create(tthread2, (void*)555);
    uthread_create(tthread3, (void*)555);

    while(1);

    exit();
}