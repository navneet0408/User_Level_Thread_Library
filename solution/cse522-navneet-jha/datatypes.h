/*
 * datatypes.h
 *
 *  Created on: Apr 1, 2015
 *      Author: root
 */


// Header file for defining structures and enumerations
#ifndef DATATYPES_H_
#define DATATYPES_H_

#define STACK_SIZE 1024
#define HIGHEST_PRIORITY 0
#define LOWEST_PRIORITY 100
#define MAX_RUN_QUEUE 100
#define MAX_JOIN_QUEUE 20
#define MAX_NUM_MUTEXES 100
#define MAX_MUTEX_QUEUE 20

#include "uthread.h"
#include <ucontext.h>

//States a thread can be in
typedef enum Thread_State_t {Blocked_State, Ready_State, Running_State, Sleeping_State,
	WaitingForMutex_State, Terminated_State} Thread_State;

//States a Mutex can be in
typedef enum Mutex_State_t {Mutex_Locked, Mutex_Unlocked} Mutex_State;

//Thread Control block structure
typedef struct Tcb_t
{
	Thread_State state;
	ucontext_t context;
	int staticPriority;
	int currentPriority;
	uthread_t tid;
	uthread_t joinedThreads[MAX_JOIN_QUEUE];
	int numJoinedThreads;
	struct timespec sleepingTime;
	void *joinRetval;
	void *returnValue;
	uthread_mutex_t blockingMutex;
	int numHoldingMutexes;
}Tcb;

//Arguments passed to the wrapper function for threads
typedef struct WrapperArgs_t
{
	void *(*start)(void *);
	void *args;
}WrapperArgs;

//Mutex Structure
typedef struct MutexStructure_t
{
	//uthread_mutex_t mutex;
	int attr;
	Mutex_State state;
	uthread_t lockingThread;
	int mutexPriority;
}MutexStructure;

#endif /* DATATYPES_H_ */
