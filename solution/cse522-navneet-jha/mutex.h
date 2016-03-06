/*
 * mutex.h
 *
 *  Created on: Apr 2, 2015
 *      Author: root
 */

#ifndef MUTEX_H_
#define MUTEX_H_

//Header file containing function declarations for mutex operations

#include "uthread.h"
#include "datatypes.h"
extern uthread_mutex_t numMutexes;
extern MutexStructure *ceilingMutex;
int initMutex(uthread_mutex_t *mutex, int attr);
int lockMutex(uthread_mutex_t mutex);
int unlockMutex(uthread_mutex_t mutex);
int enableSrp(resource_entry_t *resource_table, int n);
int lockCeilingTcb(Tcb *tcb);

#endif /* MUTEX_H_ */
