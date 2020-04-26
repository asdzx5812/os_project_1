#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sched.h>
#include<string.h>
#include<sys/time.h>
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
	if(strcmp("FIFO", policy) == 0){
		flag_for_policy = 0;
	}
	sort(flag_for_policy, Process_name, R, T);
	
}
