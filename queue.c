#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include"queue.h"

struct Node* newNode(int pid, int remain_time){
	struct Node* tmp = (struct Node*)malloc(sizeof(struct Node));
	tmp->pid = pid;
	tmp->remain_time = remain_time;
	tmp->next = NULL;
	return tmp;
}

struct Queue* createQueue(){
	struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
	q->head = NULL;
	q->tail = NULL;
	return q;
}

void enQueue(struct Queue* q, int pid, int remain_time){
	struct Node* tmp = newNode(pid, remain_time);
	if(q->tail == NULL){
		q->head = tmp;
		q->tail = tmp;
		return;
	}
	q->tail->next = tmp;
	q->tail = tmp;
}
void deQueue(struct Queue* q){
	if(q->head == NULL)
		return;	
	struct Node* tmp = q->head;
	q->head = q->head->next;
	if(q->head == NULL)
		q->tail = NULL;
	
	free(tmp);
}
void mvHead(struct Queue* q){
	if(q->head->next == NULL)
		return;
	struct Node* tmp = q->head;
	q->head = q->head->next;
	q->tail->next = tmp;
	q->tail = tmp;
	tmp->next = NULL;
}
bool Queue_is_empty(struct Queue* q){
	if(q->head == NULL)
		return true;
	return false;
}
void findshortest(struct Queue* q){
	struct Node* tmp = q->head;
	struct Node* shortest = q->head;
//	fprintf(stderr, "findshort begin\n");	
	while(tmp != NULL){
		if(tmp->remain_time < shortest->remain_time){
			shortest = tmp;
		}
		tmp = tmp->next;
	}
//	fprintf(stderr, "the shortest node has found\n");
	if(shortest == q->head)
		return;
	else if(shortest == q->tail){
		tmp = q->head;
//		fprintf(stderr, "begin\n");
		while(tmp != NULL){
			if(tmp->next == shortest)
				break;
			tmp = tmp->next;
		}	
//		fprintf(stderr, "end\n");
		shortest->next = q->head;
		q->tail = tmp;
		q->tail->next = NULL;
		q->head = shortest;
//		fprintf(stderr, "ssssssssss %d\n", q->head->pid);
	}
	else{
		tmp = q->head;
		while(tmp != NULL){
			if(tmp->next == shortest)
				break;
			tmp = tmp->next;
		}	
		tmp->next = shortest->next;
		shortest->next = q->head;
		q->head = shortest;
	}
}
