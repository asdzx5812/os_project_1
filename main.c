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
#include<sys/syscall.h>
#include"mysort.h"
#include"timeunit.h"
#include"queue.h"

#define TIME_QUANTUM 500
int main(){
	//0 : FIFO
	//1 : RR
	//2 : SJF
	//3 : PSJF
	int flag_for_policy = -1; 
	
	//a list for CPU ready queue
	//(the head in this queue is the current running process at CPU1)
	struct Queue* list = createQueue();
	
	//read data
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

	//Set CPU information
	cpu_set_t mask, mask1;
	CPU_ZERO(&mask);
	CPU_ZERO(&mask1);
	CPU_SET(0, &mask);
	CPU_SET(1,&mask1);
	//set the main process bound on CPU 1
	if(sched_setaffinity(0, sizeof(cpu_set_t), &mask1) == -1){
		fprintf(stderr,"set affinity error\n");
		return -1;
	}

	//param is max-1 priority
	struct sched_param param, parammax;
	parammax.sched_priority = sched_get_priority_max(SCHED_FIFO);
	param.sched_priority = parammax.sched_priority - 1;
	
	//Set the scheduler of main process  is FIFO(child will inherit this property)
	if(sched_setscheduler(0, SCHED_FIFO, &param) == -1){
		fprintf(stderr, "set scheduler error\n");
		fprintf(stderr, "Message %s\n", strerror(errno));
		return -1;
	}
	//assign value to flag_for_policy (used to change the action in the while below)
	if(strcmp("FIFO", policy) == 0)
		flag_for_policy = 0;
	else if(strcmp("RR", policy) == 0)
		flag_for_policy = 1;
	else if(strcmp("SJF", policy) == 0)
		flag_for_policy = 2;
	else if(strcmp("PSJF", policy) == 0)
		flag_for_policy = 3;
	else
		fprintf(stderr,"Wrong poicy!\n");
	//shared variable end_flag between main and child 
	//is used to inform child to terminate
	int* end_flag;
	const int shareSize = sizeof(int);
	int segmentId = shmget(IPC_PRIVATE, shareSize, S_IRUSR | S_IWUSR);
	end_flag = (int*)shmat(segmentId, NULL, 0);
	*end_flag = 0;
	//sort it, then we can arrive the process in order
	sort(Process_name, R, T, N);
	if(flag_for_policy == 2 || flag_for_policy == 3)
		sort_SJF(Process_name, R, T, N);
	
	//pid is for fork()
	//num_process_done counts how many processes had been terminated
	//num_process_arrive counts how many processes had arrived
	int num_process_done = 0, num_process_arrive = 0;
	pid_t pid;

	//main_clock is the clock of main process
	//cure_process_clock is used to check RR quantum time
	int main_clock = 0, cur_process_clock = 0; 
	bool check = false;
	//begin to schedule
	while(num_process_done < N){

		//if there is a process arrive at this time, add it to the ready queue(list)
		while(num_process_arrive < N && main_clock == R[num_process_arrive]){
			pid = fork();			
			if(pid < 0){
				fprintf(stderr,"fork process %d failed\n", num_process_arrive);
				return -1;
			}
			else if(pid == 0){ //for child
				long starttime = syscall(335);
				pid = getpid();
				fprintf(stdout, "%s %d\n", Process_name[num_process_arrive], pid);
				fflush(stdout);	
				//infinite loop, when be informed that should be terminated
				//then terminate it
				while(1){
					if(*end_flag){ 
						*end_flag = 0;
						break;
					}
				}
				long endtime = syscall(335);
				syscall(334,pid,starttime,endtime);
				exit(0);
			}
			else{	//main process
				//add it to ready queue(list)
				if(Queue_is_empty(list))
					check = true;
				enQueue(list,pid,T[num_process_arrive]);
				//set the child process bound to CPU 0
				//it means that the child be added to ready queue(CPU 0) 
				// -if there is no process running, the child begin to run
				// -else wait in ready queue
				//before child bound to CPU 0, it is in ready queue(CPU 1),
				//so it will be blocked until it call this function
				if(sched_setaffinity(pid, sizeof(cpu_set_t), &mask)){
					fprintf(stderr, "set process %d affinity to CPU 0 failed", num_process_arrive);
					return -1;
				}
				if(check){
					check = false;
					if(sched_setparam(pid, &parammax) == -1){
						fprintf(stderr, "set param error\n");
						return -1;
					}
				}
				num_process_arrive++;
			}			
		}

		//the reason why this block move above wait_a_unit() is 
		//in order to deal with RR (need add arrive process in ready queue
		//then switch process)
		if(main_clock != 0 && !Queue_is_empty(list)){
			//the running process should be terminate
			if(list->head->remain_time == 0 && (flag_for_policy == 0 || flag_for_policy == 1)){ 
				deQueue(list);
				if(!Queue_is_empty(list))
					if(sched_setparam(list->head->pid, &parammax) == -1)
						fprintf(stderr, "set param error\n");

				*end_flag = 1;
				num_process_done++;
				cur_process_clock = 0;
				//synchronize with child process
				//wait until the child process get the information(terminate information)
				while(*end_flag == 1); 
			} 
			else if(list->head->remain_time == 0){
				deQueue(list);
				if(!Queue_is_empty(list)){
					
				//	fprintf(stderr,"%d\n", list->head->pid);
					findshortest(list);
				//	fprintf(stderr,"%d\n", list->head->pid);
					

					if(sched_setparam(list->head->pid, &parammax) == -1)
						fprintf(stderr, "set param error\n");
				}

				*end_flag = 1;
				num_process_done++;
				cur_process_clock = 0;
				while(*end_flag == 1);
			
			}//RR timequantum check
			else if(flag_for_policy == 1 && (cur_process_clock == TIME_QUANTUM)){
				cur_process_clock = 0;
				//fprintf(stderr, "switch!!\n");
				//change the priority of the running process
				//these two instructions will cause the running process be
				//added to the end of ready queue(CPU 1)
				int current_pid = list->head->pid;
				mvHead(list);
				if(sched_setparam(list->head->pid, &parammax) == -1)
					fprintf(stderr, "set param error\n");
				if(sched_setparam(current_pid, &param) == -1)
					fprintf(stderr, "set param error\n");
			}
			else if(flag_for_policy == 3){
				if(!Queue_is_empty(list)){
					struct Node* tmp = list->head;	
					findshortest(list);
					if(list->head != tmp){
						if(sched_setparam(list->head->pid, &parammax) == -1)
							fprintf(stderr, "set param error\n");
						if(sched_setparam(tmp->pid, &param) == -1)
							fprintf(stderr, "set param error\n");
						cur_process_clock = 0;
					}
				}
			}
		}
		//a unit time
		wait_a_unit();
		main_clock++;
//		fprintf(stderr, "mainclock:%d\n", main_clock);
		if(!Queue_is_empty(list)){
			cur_process_clock++;
			list->head->remain_time--;
		}
	}
	//wait all child process exit, avoid zombie processes
	while(wait(NULL) > 0);
	
	return 0;
}
