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
#include<sys/resource.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/stat.h>
#include<stdbool.h>
#include"mysort.h"
#include"timeunit.h"
#include"queue.h"

#define TIME_QUANTUM 500
int main(){
	int flag_for_policy = -1; // 0 for FIFO
	char policy[5];
	int N;
	scanf("%s %d", policy, &N);
	struct Queue* list = createQueue();
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

	struct sched_param param, param1, param2;//param is max priority, param2 is max-1 priority, param1 is zero priority
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	param2.sched_priority = param.sched_priority - 1;
	param1.sched_priority = 0;
	
	if(sched_setscheduler(0, SCHED_FIFO, &param) == -1){
		fprintf(stderr, "set scheduler error\n");
		fprintf(stderr, "Message %s\n", strerror(errno));
		return -1;
	}

	if(strcmp("FIFO", policy) == 0){
		flag_for_policy = 0;
	}
	else if(strcmp("RR", policy) == 0){
		flag_for_policy = 1;
	}
	//shared variable
	int* end_flag;
	const int shareSize = sizeof(int);
	int segmentId = shmget(IPC_PRIVATE, shareSize, S_IRUSR | S_IWUSR);
	end_flag = (int*)shmat(segmentId, NULL, 0);
	////
	sort(Process_name, R, T, N);
	int num_process_done = 0, num_process_arrive = 0;
	pid_t pid;
	// main_clock is the clock of main process
	// cure_process_clock is used to check RR quantum time
	int main_clock = 0, cur_process_clock = 0; 
	while(num_process_done < N){
			while(num_process_arrive < N && main_clock == R[num_process_arrive]){
				fprintf(stderr, "fork %s\n", Process_name[num_process_arrive]);
				pid = fork();
					
				if(pid < 0){
					fprintf(stderr,"fork process %d failed\n", num_process_arrive);
					return -1;
				}
				else if(pid == 0){ //for child
			
					pid = getpid();
					fprintf(stderr, "%s %d\n", Process_name[num_process_arrive], pid);
					while(1){
						if(*end_flag){
							*end_flag = 0;
							break;
						}
					}
					fprintf(stderr, "%s %d end\n", Process_name[num_process_arrive], pid);
					exit(0);
				}
				else{	//main process
					enQueue(list,pid,T[num_process_arrive]);	
					/*
						if(sched_setparam(first, &param2) == -1)
							fprintf(stderr, "set param error\n");
						if(sched_setparam(first, &param) == -1)
							fprintf(stderr, "set param error\n");
					*/
				//	fprintf(stderr,"%s %d\n",Process_name[i],a);
					if(sched_setaffinity(pid, sizeof(cpu_set_t), &mask)){
						fprintf(stderr, "set process %d affinity to CPU 0 failed", num_process_arrive);
						return -1;
					}
					num_process_arrive++;
				}			
			}
			if(main_clock != 0 && !Queue_is_empty(list)){
				cur_process_clock++;
				list->head->remain_time--;
				if(list->head->remain_time == 0){
					deQueue(list);
					*end_flag = 1;
					num_process_done++;
					cur_process_clock = 0;
					while(*end_flag == 1);
				} // RR timequantum check
				else if(flag_for_policy == 1 && (cur_process_clock == TIME_QUANTUM)){
					fprintf(stderr, "timeout switch process!!\n"); 
					cur_process_clock = 0;
					if(sched_setparam(list->head->pid, &param2) == -1)
						fprintf(stderr, "set param error\n");
					if(sched_setparam(list->head->pid, &param) == -1)
						fprintf(stderr, "set param error\n");
					mvHead(list);
				}	
			}

			wait_a_unit();
			main_clock++;
	}

	while(wait(NULL) > 0);
}
