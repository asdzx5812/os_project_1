default :
	gcc -Wall -std=gnu99 -c mysort.c 
	gcc -Wall -std=gnu99 -c main.c
	gcc -Wall -std=gnu99 -c timeunit.c
	gcc -Wall -std=gnu99 -c queue.c
	gcc -Wall -std=gnu99 main.o mysort.o timeunit.o queue.o

clean :
	rm main.o mysort.o a.out timeunit.o queue.o
