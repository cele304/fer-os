#include <stdio.h>
#include <signal.h>
#include <values.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

int *zajednickaVarijabla;
int id;
int broj;

struct brojArg {
    int i;
};

void *radnaDretva(void *number) {

    struct brojArg *n = (struct brojArg*)number;
    int br = n->i;

	printf("Pokrenuta radna dretva\n");
	int i = *zajednickaVarijabla;
	int uvecani=0;
	
	while ((int*) br > 0) {
		while (i == 0 && broj != 0) {
			uvecani = broj+1;
			printf("Radna dretva: procitan broj %d i uvecan na %d\n", broj, uvecani);
			*zajednickaVarijabla = uvecani;
			broj=0;
			--br;
		}
		i = *zajednickaVarijabla;
	}	
	printf("Zavrsila radna dretva\n");
}

void brisi(int signal) {
	(void) shmdt ((char *) zajednickaVarijabla);
	(void) shmctl (id, IPC_RMID, NULL);
	exit(0);
}

int main(int arg, char**args) {
	printf("Pokrenuta ulazna dretva\n");
	
	id = shmget(IPC_PRIVATE, sizeof(int), 0600);
	
	if (id == -1) {
		exit(1);
	}
	
	zajednickaVarijabla = (int *) shmat(id, NULL, 0);
	*zajednickaVarijabla = 0;
	
	int br=0, number=0, slucajni;
	number = atoi(args[1]);

    struct brojArg* argTmp = malloc(sizeof(struct brojArg));
    argTmp->i = number;

	pthread_t thr_id;
	
	switch(fork()) {
			case 0:
				printf("Pokrenut izlazni proces\n");
	            int i = *zajednickaVarijabla;
	            
	            while (number > 0) {
		            while (i != 0) {
			            FILE *dat = fopen("datoteka.txt", "a+");
			            fprintf(dat, "%d\n", i);
			            fclose(dat);
			            
			            printf("Izlazni proces: upisan broj %d u datoteku\n", i);
			            *zajednickaVarijabla = 0;
			            --number;
			            i = *zajednickaVarijabla;
		            }
		            i = *zajednickaVarijabla;
	            }     
	            printf("Zavrsio izlazni proces\n");
				exit(0);

			case -1:
				printf("Nije mogu�e stvoriti novi proces");

			default:
				pthread_create(&thr_id, NULL, radnaDretva, (void*) argTmp);
				sleep(1);	
				while(number > 0) {
					if (broj == 0) {
						srand((unsigned)time(NULL));
						slucajni = rand() % 100 + 1;
						printf("Ulazna dretva: broj %d\n", slucajni);
						broj = slucajni;
						--number;
						sleep(rand() % 5 + 1);
					}
				}
	}
	wait(NULL);
	printf("Zavrsila ulazna dretva\n");
	pthread_join(thr_id, NULL);
	
	struct sigaction act;
	act.sa_handler = brisi;
	sigaction(SIGINT, &act, NULL);
	
	brisi(0);
	return 0;
}
