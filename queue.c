#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include"queue.h"

struct Node* newNode(int pid, int remain_time){
	struct Node* tmp = (struct Node*)malloc(sizeof(struct Node));
	tmp->pid = pid;
	tmp->remain_time = remain_time;
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
