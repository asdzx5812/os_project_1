#include<stdio.h>
#include<stdlib.h>
#include"timeunit.h"
void wait_a_unit(){
	volatile unsigned long i;
	for(i = 0; i < 1000000UL; i++);
}

