/*
 * mutex.c
 *
 *  Created on: Apr 2, 2015
 *      Author: root
 */

#include "mutex.h"
#include "datatypes.h"
#include "queue.h"
#include <stdlib.h>

uthread_mutex_t numMutexes=0; //Number of Mutexes initialized
MutexStructure *mutexStructureQueue[MAX_NUM_MUTEXES]; //Holding the MutexStructure of the initialized Mutexes
MutexStructure *ceilingMutex = NULL; //For srp


//Initialize the Mutex
int initMutex(uthread_mutex_t *mutex, int attr)
{
	if(numMutexes == MAX_NUM_MUTEXES)
		return -1;
	MutexStructure *mutexStructure = (MutexStructure *) malloc(sizeof(MutexStructure));
	if(mutexStructure == NULL)
		return -1;
	mutexStructure->attr=attr;
	mutexStructure->state=Mutex_Unlocked;
	//mutexStructure->mutex = ++currentMutex;
	mutexStructure->mutexPriority = LOWEST_PRIORITY+1;
	mutexStructureQueue[numMutexes++] = mutexStructure;
	(*mutex) = (uthread_mutex_t) mutexStructure;
	return 0;

}

//Lock a Mutex. If Mutex already locked, change the state of the calling thread to
//WaitingForMutex_State, and swap context...
//Also sets the ceilingMutex...
int lockMutex(uthread_mutex_t mutex)
{
	sigset_t s;
	sigaddset(&s, SIGALRM);
	sigprocmask(SIG_BLOCK, &s, NULL);

	Tcb *runningTcb=NULL;
	runningTcb = findRunningTcb();
	getcontext(&runningTcb->context);

	int retval=0;
	Tcb *maxPriorityTcb = NULL;
	MutexStructure *mutexStructure = (MutexStructure *) mutex;

	if(mutexStructure->state == Mutex_Unlocked)
	{
		mutexStructure->state = Mutex_Locked;
		mutexStructure->lockingThread = (uthread_t) runningTcb;
		runningTcb->numHoldingMutexes++;
		if(ceilingMutex == NULL)
		{
			ceilingMutex = mutexStructure;
		}
		else if(ceilingMutex->mutexPriority > mutexStructure->mutexPriority)
		{
			ceilingMutex = mutexStructure;
		}
	}
	else
	{
		runningTcb->state = WaitingForMutex_State;
		runningTcb->blockingMutex = mutex;
		if(mutexStructure->attr == UTHREAD_MUTEX_ATTR_PI)
		{
			Tcb *lockingTcb = (Tcb *) (mutexStructure->lockingThread);
			if(lockingTcb->currentPriority > runningTcb->currentPriority)
			{
				lockingTcb->currentPriority = runningTcb->currentPriority;
			}
		}
		retval = -2;
	}

	if(retval == -2)
	{
		maxPriorityTcb = findMaxPriorityTcb();
		maxPriorityTcb->state = Running_State;
		setcontext(&maxPriorityTcb->context);
		retval = 0;
	}
	sigprocmask(SIG_UNBLOCK, &s, NULL);
	return retval;
}


//Unlock a Mutex... Change the state of all the threads waiting for this mutex to
//Ready_State..
//Also sets the ceilingMutex
int unlockMutex(uthread_mutex_t mutex)
{
	sigset_t s;
	sigaddset(&s, SIGALRM);
	sigprocmask(SIG_BLOCK, &s, NULL);

	int i, retval=0;
	Tcb *runningTcb=NULL, *maxPriorityTcb=NULL;
	runningTcb = findRunningTcb();

	if((runningTcb == NULL))
	{
		return -1;
	}

	MutexStructure *mutexStructure = (MutexStructure *) mutex;

	mutexStructure->state = Mutex_Unlocked;
	runningTcb->numHoldingMutexes--;
	if(runningTcb->numHoldingMutexes == 0)
		runningTcb->currentPriority = runningTcb->staticPriority;

	ceilingMutex = NULL;
	for(i=0;i<numMutexes; ++i)
	{
		if(mutexStructureQueue[i]->state == Mutex_Locked)
		{
			if(ceilingMutex == NULL)
				ceilingMutex = mutexStructureQueue[i];
			else if(ceilingMutex->mutexPriority > mutexStructureQueue[i]->mutexPriority)
				ceilingMutex = mutexStructureQueue[i];
		}
	}
	handleWaitingForMutexTcb(mutex);

	maxPriorityTcb = findMaxPriorityTcb();

	if(maxPriorityTcb==NULL)
	{
		return -1;
	}
	if(runningTcb->currentPriority == maxPriorityTcb->currentPriority)
	{
		return 0;
	}

	runningTcb->state = Ready_State;
	maxPriorityTcb->state = Running_State;
	swapcontext(&runningTcb->context, &maxPriorityTcb->context);

	sigprocmask(SIG_UNBLOCK, &s, NULL);
	return retval;
}

//Enable the stack based ceiling resource protocol
int enableSrp(resource_entry_t *resource_table, int n)
{
	int i,j;
	for(i=0; i<n; ++i)
	{
		MutexStructure *mutexStructure = (MutexStructure *) resource_table[i].resource;
		for(j=0; j<resource_table[i].n; ++j)
		{
			Tcb *threadTcb = (Tcb *) (resource_table[i].thread_array[j]);
			if(mutexStructure->mutexPriority > threadTcb->staticPriority)
			{
				mutexStructure->mutexPriority = threadTcb->staticPriority;
			}
		}
	}
	return 0;
}
