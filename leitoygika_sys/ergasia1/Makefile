CC =gcc

CFLAGS =-c -Wall


a : alpha
	./alpha

b : beta
	./beta	

alpha: alpha.o
		$(CC) -o alpha alpha.o

beta: beta.o
		$(CC) -o beta beta.o



alpha.o: alpha.c 
		$(CC) $(CFLAGS) alpha.c

beta.o: beta.c 
		$(CC) $(CFLAGS) beta.c
