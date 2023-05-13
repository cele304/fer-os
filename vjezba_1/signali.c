#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

FILE *status;
int br;

void obradi_sigusr1(int signal) {
	printf("Primio signal SIGUSR1 %d\n", br);
    fflush(stdout);
}

void obradi_sigterm(int signal) {
	status = fopen("status.txt", "w+");
	fprintf(status, "%d\n", br);
	fclose(status);
	printf("Primio signal SIGTERM, pospremam prije izlaska iz programa\n");
   	printf("Program s PID=%ld zavrsio s radom\n", (long) getpid());
	exit(1);
}

int main() {
	struct sigaction act;
	
	act.sa_handler = obradi_sigusr1;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGTERM); 
	act.sa_flags = 0;
	sigaction(SIGUSR1, &act, NULL);
	
	act.sa_handler = obradi_sigterm;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTERM, &act, NULL);
	
	printf("Program s PID = %ld krenuo s radom\n", (long) getpid());

	status = fopen("status.txt", "a+");
	fscanf(status, "%d", &br);
    fclose(status);

	status = fopen("status.txt", "w+");
	fprintf(status, "%d\n", 0);
    fclose(status);
 
	if (br == 0) {
        FILE *obrada = fopen("obrada.txt", "a+");
        int tmp;
		while (fscanf(obrada, "%d", &tmp) == 1) {
			br = tmp;
		}
        fclose(obrada);
		br = sqrt(br);
	}
	
	FILE *obrada = fopen("obrada.txt", "a+");
	while(true) {
		++br;
		int x = br * br;
		fprintf(obrada, "%d\n", x);
		for (int i=0; i<5; i++) 
            sleep(1);
	}
    fclose(obrada);
	return 0;
}
