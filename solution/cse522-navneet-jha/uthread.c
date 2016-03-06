/*
 * uthread.c
 *
 *  Created on: Apr 1, 2015
 *      Author: root
 */

#include "datatypes.h"
#include "scheduler.h"
#include "queue.h"
#include "mutex.h"
#include <stdlib.h>
#include <string.h>


//Create a new thread... initialize the scheduler if not already initialized
int uthread_create(uthread_t *tid, void *(*start)(void *), void *args, int priority)
{
	if(isSchedulerInitialized==0)
	{
		isSchedulerInitialized=1;
		init(0, 500);
	}
	Tcb *threadTcb = (Tcb *) malloc(sizeof(Tcb));
	if(threadTcb==NULL)
		return -1;
	WrapperArgs *wArgs = (WrapperArgs *) malloc(sizeof(WrapperArgs));
	if(wArgs == NULL)
		return -1;
	wArgs->args=args;
	wArgs->start = start;
	void *stackPtr = malloc(STACK_SIZE * sizeof(void *));
	if(stackPtr == NULL)
		return -1;
	memset(threadTcb, 0, sizeof(Tcb));
	if( getcontext(&threadTcb->context) !=0)
		return -1;
	threadTcb->context.uc_stack.ss_sp = stackPtr;
	threadTcb->context.uc_stack.ss_size = STACK_SIZE * sizeof(void *);
	makecontext(&threadTcb->context, (void (*)(void))wrapper, 1, wArgs);

	threadTcb->currentPriority = priority;
	threadTcb->staticPriority = priority;
	threadTcb->state=Ready_State;
	threadTcb->tid = ++currentTid;
	if(addTcb(threadTcb) !=0)
		return -1;

	(*tid) = (uthread_t) threadTcb;
	return 0;
}

//Terminate the running thread, Change the state of all threads joined
//to this thread to Ready_State.Find the next highest priority thread
//and set context.
void uthread_exit(void *value_ptr)
{
	int i;
	Tcb *runningTcb = findRunningTcb();
	runningTcb->returnValue = value_ptr;
	for(i=0; i<runningTcb->numJoinedThreads; ++i)
	{
		Tcb *joinedTcb = (Tcb *)(runningTcb->joinedThreads[i]);
		joinedTcb->state = Ready_State;
		joinedTcb->joinRetval = value_ptr;
	}

	runningTcb->state = Terminated_State;

	Tcb* maxPriorityTcb = findMaxPriorityTcb();
	maxPriorityTcb->state=Running_State;
	setcontext(&maxPriorityTcb->context);
}

//Clean up, and return the value returned by the thread in value_ptr
//If joining thread still running, block the running thread
int uthread_join(uthread_t tid, void **value_ptr)
{
	sigset_t s;
	sigaddset(&s, SIGALRM);
	sigprocmask(SIG_BLOCK, &s, NULL);

	Tcb *joinTcb = (Tcb *) tid;
	if(joinTcb == NULL)
		return -1;
	Tcb *runningTcb = findRunningTcb();
	if(runningTcb == NULL)
		return -1;
	if(joinTcb->state == Terminated_State)
	{
		(*value_ptr) = joinTcb->returnValue;
		deleteTcb(joinTcb->tid);
		sigprocmask(SIG_UNBLOCK, &s, NULL);
		return 0;
	}
	joinTcb->joinedThreads[joinTcb->numJoinedThreads++] = (uthread_t) runningTcb;
	runningTcb->state = Blocked_State;
	Tcb *maxPriorityTcb = findMaxPriorityTcb();
	if(maxPriorityTcb == NULL)
		return -1;

	maxPriorityTcb->state = Running_State;

	swapcontext(&runningTcb->context, &maxPriorityTcb->context);

	sigprocmask(SIG_UNBLOCK, &s, NULL);

	(*value_ptr) = runningTcb->joinRetval;
	return 0;
}

int uthread_gettime(struct timespec *tp)
{
	return clock_gettime(CLOCK_MONOTONIC, tp);
}

//Change the state to Sleeping_State.. swap context
int uthread_abstime_nanosleep(const struct timespec *request)
{
	sigset_t s;
	sigaddset(&s, SIGALRM);
	sigprocmask(SIG_BLOCK, &s, NULL);

	Tcb *runningTcb = findRunningTcb();
	runningTcb->state = Sleeping_State;
	runningTcb->sleepingTime.tv_sec = request->tv_sec;
	runningTcb->sleepingTime.tv_nsec = request->tv_nsec;

	Tcb *maxPriorityTcb = findMaxPriorityTcb();

	maxPriorityTcb->state = Running_State;
	swapcontext(&runningTcb->context, &maxPriorityTcb->context);

	sigprocmask(SIG_UNBLOCK, &s, NULL);
	return 0;
}

//Initialize the mutex
int uthread_mutex_init(uthread_mutex_t *mutex, const int attr)
{
	return initMutex(mutex, attr);
}

//lock the mutex
int uthread_mutex_lock(uthread_mutex_t *mutex)
{
	return lockMutex(*mutex);
}

//unlock the mutex
int uthread_mutex_unlock(uthread_mutex_t *mutex)
{
	return unlockMutex(*mutex);
}

//enable the srp protocol
int uthread_srp_enable(resource_entry_t *resource_table, int n)
{
	return enableSrp(resource_table, n);
}
