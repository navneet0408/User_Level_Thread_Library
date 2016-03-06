/*
 * queue.h
 *
 *  Created on: Apr 1, 2015
 *      Author: root
 */

#ifndef QUEUE_H_
#define QUEUE_H_

//Header file containing function declarations for all the
//Tcb Queue operations

#include "datatypes.h"

int addTcb(Tcb *tcb);
Tcb *findMaxPriorityTcb();
Tcb *findRunningTcb();
Tcb *findTcb(uthread_t tid);
int deleteTcb(uthread_t tid);
void handleSleepingTcb();
void handleWaitingForMutexTcb(uthread_mutex_t mutex);

#endif /* QUEUE_H_ */
