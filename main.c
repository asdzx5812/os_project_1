#define _GNU_SOURCE
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<sched.h>
#include<string.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/wait.h>
#include"mysort.h"
int main(){
	int flag_for_policy = -1; // 0 for FIFO
	char policy[5];
	int N;
	scanf("%s %d", policy, &N);

	char** Process_name = (char**)malloc(sizeof(char*) * N);
	int* R = (int*)malloc(sizeof(int) * N);
	int* T = (int*)malloc(sizeof(int) * N);
	
	int i;
	for(i = 0; i < N; i++){
		Process_name[i] = (char*)malloc(sizeof(char) * 33);
		scanf("%s %d %d", Process_name[i], &R[i], &T[i]);
	}

	cpu_set_t mask, mask1;
	CPU_ZERO(&mask);
	CPU_ZERO(&mask1);
	CPU_SET(0, &mask);
	CPU_SET(1,&mask1);

	if(sched_setaffinity(0, sizeof(cpu_set_t), &mask1) == -1){
		fprintf(stderr,"set affinity error\n");
		return -1;
	}

	struct sched_param param, param1;
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	param1.sched_priority = 0;
	if(sched_setscheduler(0, SCHED_FIFO, &param) == -1){
		fprintf(stderr, "set scheduler error\n");
		fprintf(stderr, "Message %s\n", strerror(errno));
		return -1;
	}

	if(strcmp("FIFO", policy) == 0){
		flag_for_policy = 0;
	}

	sort(flag_for_policy, Process_name, R, T, N);
	
	pid_t pid;
	int child_num = 0;
	int main_clock = 0;
	for(i = 0; i < N; i++){
		while(main_clock != R[i]){
			volatile unsigned long i; 
			for(i=0;i<1000000UL;i++);  
			main_clock++;
		}
		pid = fork();
	
		if(pid < 0){
			fprintf(stderr,"fork process %d failed\n", i);
			return -1;
		}
		else if(pid == 0){ //for child
			
			pid = getpid();
			fprintf(stderr, "%s %d\n", Process_name[i], pid);
			int round;
			for(round = 0; round < R[i]; round++){
				volatile unsigned long i; 
				for(i=0;i<1000000UL;i++);  
			}
			exit(0);
		}
		else{	//main process

			if(sched_setaffinity(pid, sizeof(cpu_set_t), &mask)){
				fprintf(stderr, "set process %d affinity to CPU 0 failed", i);
				return -1;
			}
		}
	}
	while(wait(NULL) > 0);
}
