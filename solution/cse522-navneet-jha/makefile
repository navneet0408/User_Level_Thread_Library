libuthread.a: uthread.o queue.o scheduler.o mutex.o
	ar rc libuthread.a uthread.o queue.o scheduler.o mutex.o
	ranlib libuthread.a
	
uthread.o: uthread.c
	gcc -c -o uthread.o uthread.c

queue.o: queue.c
	gcc -c -o queue.o queue.c

scheduler.o: scheduler.c
	gcc -c -o scheduler.o scheduler.c

mutex.o: mutex.c
	gcc -c -o mutex.o mutex.c

clean:
	rm -rf *.o libuthread.a
