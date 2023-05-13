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

#define BR_VOZNJI 2
#define BR_POSJETITELJA 4

sem_t *ulaze;
sem_t *usli;
sem_t *izlaze;
sem_t *izasli;


void ringispil() {
	int br_posjetitelja = BR_POSJETITELJA;
	int br_voznji = BR_VOZNJI;
	while(br_voznji > 0) {
		printf("Dobrodosli na novi vrtuljak!\n");

		for (int i=1; i<=br_posjetitelja; i++) {
			sem_post(ulaze);
			sleep(1);
		}

		for (int i=1; i<=br_posjetitelja; i++) {
			printf("Usao %d. posjetitelj\n", i);
			sem_wait(usli);
		}

		printf("Pokrecem vrtuljak\n");
		printf("BRM BRM\n");

		sleep(2);

		printf("BRM BRM\n");
		printf("Zaustavljam vrtuljak\n");

		for (int i=1; i<=br_posjetitelja; i++) {
			sem_post(izlaze);
			sleep(1);
		}

		for (int i=1; i<=br_posjetitelja; i++) {
			printf("Izasao %d. posjetitelj\n", i);
			sem_wait(izasli);
		}

		br_voznji--;
		printf("Dovidenja i dodite nam opet!\n");
	}
	exit(0);
}

void posjetitelj() {
	int br_voznji = BR_VOZNJI;
	while(br_voznji > 0) {

		sem_wait(ulaze);
		printf("Dosao sam i cekam red\n");

		sem_post(usli);
		printf("Usao sam i sjeo sam\n");

		sem_wait(izlaze);
		printf("Ustajem se i izlazim\n");

		sem_post(izasli);
		printf("Izasao sam\n");

		br_voznji--;
	}
	exit(0);
}


void obradi_sigint(int signal) {
	exit(1);
}

int main() {

	int br_posjetitelja = BR_POSJETITELJA;

	int id1 = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
	int id2 = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
	int id3 = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
	int id4 = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);

	ulaze = shmat(id1, NULL, 0);
	usli = shmat(id2, NULL, 0);
	izlaze = shmat(id3, NULL, 0);
	izasli = shmat(id4, NULL, 0);

	shmctl (id1, IPC_RMID, NULL);
	shmctl (id2, IPC_RMID, NULL);
	shmctl (id3, IPC_RMID, NULL);
	shmctl (id4, IPC_RMID, NULL);

	sem_init (ulaze, 1, 0);
	sem_init (usli, 1, 0);
	sem_init (izlaze, 1, 0);
	sem_init (izasli, 1, 0);



	for (int i=1; i<=br_posjetitelja; i++) {
		switch(fork()) {
			case 0:
				posjetitelj();
				exit(0);
			case -1:
				printf("Greska!");
			default:
				break;
		}
	}

	ringispil();

	struct sigaction act;
	act.sa_handler = obradi_sigint;
	sigemptyset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);

	sem_destroy(ulaze);
	sem_destroy(usli);
	sem_destroy(izlaze);
	sem_destroy(izasli);

	shmdt(ulaze);
	shmdt(usli);
	shmdt(izlaze);
	shmdt(izasli);

	return 0;

}
