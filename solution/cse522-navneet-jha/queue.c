/*
 * queue.c
 *
 *  Created on: Apr 1, 2015
 *      Author: root
 */



#include "queue.h"
#include "mutex.h"
#include <stdlib.h>


Tcb *tcbRunQueue[MAX_RUN_QUEUE]; //Queue containing Tcb's of threads
int numJobs = 0; //Number of threads (Including Main and Idle threads)

//Add A Tcb to Queue
int addTcb(Tcb *tcb)
{
	if(numJobs == MAX_RUN_QUEUE)
		return -1;
	tcbRunQueue[numJobs++] = tcb;
	return 0;
}

//Find the maximum priority Tcb in the queue which is in Ready_State or Running_State
//and has a priority more than the ceilingMutex, is in Running_State
//or the idleState Tcb.. (with a static priority of LOWEST_PRIORITY)
Tcb *findMaxPriorityTcb()
{
	int i;
	Tcb *retTcb;
	while(1)
	{
		retTcb = NULL;
		for(i=0; i<numJobs; ++i)
		{
			if((tcbRunQueue[i]->state == Ready_State) || (tcbRunQueue[i]->state == Running_State))
			{
				if(retTcb==NULL)
				{
					retTcb = tcbRunQueue[i];
				}
				else
				{
					if((retTcb->currentPriority) > ((tcbRunQueue[i]->currentPriority)))
					{
						retTcb = tcbRunQueue[i];
					}
				}
			}
		}
		if((retTcb->staticPriority == LOWEST_PRIORITY) || ceilingMutex == NULL || (retTcb->state == Running_State))
		{
			break;
		}

		if(retTcb->currentPriority >= ceilingMutex->mutexPriority)
		{
			retTcb->state = WaitingForMutex_State;
			retTcb->blockingMutex = (uthread_mutex_t) ceilingMutex;
			Tcb *lockingTcb = (Tcb *) (ceilingMutex->lockingThread);
			if(lockingTcb->currentPriority > retTcb->currentPriority)
			{
				lockingTcb->currentPriority = retTcb->currentPriority;
			}
		}
		else
			break;
	}
	return retTcb;
}

//Return the Tcb in the Running_State
Tcb *findRunningTcb()
{
	int i;
	Tcb *retTcb = NULL;
	for(i=0; i<numJobs; ++i)
	{
		if(tcbRunQueue[i]->state == Running_State)
		{
			retTcb = tcbRunQueue[i];
			break;
		}
	}
	return retTcb;
}

//Delete the Tcb structure and its stack
int deleteTcb(uthread_t tid)
{
	int i,j;
	for(i=0; i<numJobs; ++i)
	{
		if(tcbRunQueue[i]->tid == tid)
			break;
	}
	if(i==numJobs)
		return -1;

	Tcb *temp = tcbRunQueue[i];
	for(j=i; j<numJobs-1; ++j)
	{
		tcbRunQueue[j] = tcbRunQueue[j+1];
	}
	numJobs--;
	//free(temp->context.uc_stack.ss_sp);
	free(temp);
	return 0;
}

//Wake up a sleeping Tcb if its sleeping time is over
void handleSleepingTcb()
{
	int i;
	for(i=0; i<numJobs; ++i)
	{
		if(tcbRunQueue[i]->state == Sleeping_State)
		{
			struct timespec currentTime;
			uthread_gettime(&currentTime);
			if(currentTime.tv_sec>tcbRunQueue[i]->sleepingTime.tv_sec)
			{
				tcbRunQueue[i]->state = Ready_State;
			}
			else if((currentTime.tv_sec == tcbRunQueue[i]->sleepingTime.tv_sec) &&
					(currentTime.tv_nsec > tcbRunQueue[i]->sleepingTime.tv_nsec))
			{
				tcbRunQueue[i]->state = Ready_State;
			}
		}
	}
}

//Wake up a Tcb waiting on the mutex
void handleWaitingForMutexTcb(uthread_mutex_t mutex)
{
	int i;
	for(i=0; i<numJobs; ++i)
	{
		if((tcbRunQueue[i]->state == WaitingForMutex_State) &&
				(tcbRunQueue[i]->blockingMutex == mutex))
		{
			tcbRunQueue[i]->state = Ready_State;
			tcbRunQueue[i]->blockingMutex = 0;
		}
	}
}
