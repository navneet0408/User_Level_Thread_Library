/*
 * scheduler.h
 *
 *  Created on: Apr 2, 2015
 *      Author: root
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_
//Header file containing function declarations for scheduler operations

#include "uthread.h"
extern uthread_t currentTid;
extern int isSchedulerInitialized;

void init(long int sec, long int usec);
void scheduler(int);
void wrapper(void *args);
void idleTask();
#endif /* SCHEDULER_H_ */
