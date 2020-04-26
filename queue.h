#ifndef QUEUE_H_
#define QUEUE_H_

struct Node{
	int pid;
	int remain_time;
	struct Node* next;
};

struct Queue{
	struct Node* head;
	struct Node* tail;
};

struct Node* newNode(int pid, int remain_time);

struct Queue* createQueue();

void enQueue(struct Queue* q, int pid, int remain_time);

void deQueue(struct Queue* q);

#endif
