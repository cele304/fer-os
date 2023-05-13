#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <values.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#define N 2 //prag pustanja druge vrste
#define BR_LINUX 3
#define BR_MS 4

pthread_mutex_t monitor;
pthread_cond_t uv[2];

int br[2] = {0,0};
int ceka[2] = {0,0};
int siti[2] = {0,0};
char *programeri[] = {"Microsoft", "Linux"};

struct args {
    int vrsta;
};


void *programer(void *p) {
int vrsta = ((struct args*)p)->vrsta;

pthread_mutex_lock(&monitor);
printf("%s programer ceka na ulaz u restoran\n", programeri[vrsta]);
ceka[vrsta]++;
while(br[1-vrsta] > 0 || (siti[vrsta] >= N  && ceka[1-vrsta] > 0)){
pthread_cond_wait(&uv[vrsta], &monitor);
}
printf("%s programer ulazi u restoran\n", programeri[vrsta]);
br[vrsta]++;
ceka[vrsta]--;
siti[1-vrsta] = 0;
siti[vrsta]++;
pthread_mutex_unlock(&monitor);

printf("%s jede u restoranu\n", programeri[vrsta]);
sleep(2);

pthread_mutex_lock(&monitor);
br[vrsta]--;
if (br[vrsta] == 0) {
pthread_cond_broadcast(&uv[1-vrsta]);
}
printf("%s programer izlazi iz restorana\n", programeri[vrsta]);
pthread_mutex_unlock(&monitor);

return NULL;
}


int main() {
pthread_t t[BR_MS + BR_LINUX];
int i;
pthread_mutex_init(&monitor, NULL);
pthread_cond_init(&uv[0], NULL);
pthread_cond_init(&uv[1], NULL);

struct args *argument1 = (struct args *)malloc(sizeof(struct args));
    argument1->vrsta = 1;

struct args *argument2 = (struct args *)malloc(sizeof(struct args));
    argument2->vrsta = 0;

for (i = 0; i < BR_MS + BR_LINUX; i++) {
    if(i < BR_MS)
        pthread_create(&t[i], NULL, programer, (void *) argument1);
    else {
        pthread_create(&t[i], NULL, programer, (void *) argument2);
        }
    }
    for (i = 0; i < BR_MS + BR_LINUX; i++) {
        pthread_join(t[i], NULL);
    }

pthread_mutex_destroy(&monitor);
pthread_cond_destroy(&uv[0]);
pthread_cond_destroy(&uv[1]);

return 0;
}
