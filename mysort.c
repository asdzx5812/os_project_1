#include<stdio.h>
#include<stdlib.h>
#include"mysort.h"
void swap(int i, int j, char** p, int* R, int* T){
	char* tmp = p[i];
	p[i] = p[j];
	p[j] = tmp;
	int int_tmp = R[i];
	R[i] = R[j];
	R[j] = int_tmp;
	int_tmp = T[i];
	T[i] = T[j];
	T[j] = int_tmp;
}
void sort(int policy, char** process_name, int* R, int* T, int N){
	int i, j;
	if(policy == 0){
		for(i = 0; i < N; i++){
			for(j = 1; j < N - i; j++){
				if(R[j - 1] > R[j]){
					swap(i,j,process_name,R,T);
				}
			
			}
		}
	}
	

}
