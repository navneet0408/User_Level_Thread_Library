/*
 * scheduler.c
 *
 *  Created on: Apr 2, 2015
 *      Author: root
 */

#include "scheduler.h"
#include "datatypes.h"
#include "queue.h"
#include "mutex.h"
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

uthread_t currentTid = 0;
int isSchedulerInitialized=0; //Call init only once

//Signal handler for SIGALRM
//Finds the highest priority thread and swaps context
void scheduler(int signal)
{
	sigset_t s;
	sigaddset(&s, SIGALRM);
	sigprocmask(SIG_BLOCK, &s, NULL);

	handleSleepingTcb();
	ucontext_t current;
	getcontext(&current);
	Tcb *maxPriorityTcb = findMaxPriorityTcb();

	Tcb *runningTcb = findRunningTcb();

	runningTcb->context=current;
	runningTcb->state=Ready_State;
	maxPriorityTcb->state=Running_State;

	if(runningTcb !=maxPriorityTcb)
		swapcontext(&current, &maxPriorityTcb->context);
	sigprocmask(SIG_UNBLOCK, &s, NULL);
}

//Create a Tcb for main thread and add it to the Tcb Queue
void initMainTcb()
{
	Tcb *mainTcb = (Tcb *) malloc(sizeof(Tcb));
	memset(mainTcb, 0, sizeof(Tcb));
	getcontext(&mainTcb->context);
	mainTcb->currentPriority = HIGHEST_PRIORITY;
	mainTcb->staticPriority = HIGHEST_PRIORITY;
	mainTcb->state = Running_State;
	mainTcb->tid = ++currentTid;
	addTcb(mainTcb);
}

//Create a Tcb for idle thread and add it to the Tcb Queue
void initIdleTcb()
{
	Tcb *idleTcb = (Tcb *) malloc(sizeof(Tcb));
	void *stackPtr = malloc(STACK_SIZE * sizeof(void *));
	memset(idleTcb, 0, sizeof(Tcb));
	getcontext(&idleTcb->context);
	idleTcb->context.uc_stack.ss_sp = stackPtr;
	idleTcb->context.uc_stack.ss_size = STACK_SIZE * sizeof(void *);
	makecontext(&idleTcb->context, idleTask, 0);
	idleTcb->currentPriority = LOWEST_PRIORITY;
	idleTcb->staticPriority = LOWEST_PRIORITY;
	idleTcb->state = Ready_State;
	idleTcb->tid = ++currentTid;
	addTcb(idleTcb);
}

//Initialize the timer
void initTimer(long int sec, long int usec)
{
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	act.sa_handler = scheduler;
	sigaction(SIGALRM, &act, NULL);

	struct itimerval tv;
	tv.it_value.tv_sec = sec;
	tv.it_value.tv_usec = usec;
	tv.it_interval = tv.it_value;
	setitimer(ITIMER_REAL, &tv, NULL);
}

//Initialize the scheduler
void init(long int sec, long int usec)
{
	initMainTcb();
	initIdleTcb();
	initTimer(sec, usec);

}

//Wrapper function for threads...
//(To call uthread_exit implicitly, if not called by the thread
void wrapper(void *args)
{
	WrapperArgs *wArgs = (WrapperArgs *)args;
	void *retval = wArgs->start(wArgs->args);
	free(wArgs);
	uthread_exit(retval);
}

//Idle task
void idleTask()
{
	while(1)
	{
		sleep(1);
	}
}
