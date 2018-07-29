//-----------------------------------------------------------------------------
// PTASK_H: MANAGE EASILY PERIODIC FIFO TASKS
//----------------------------------------------------------------------------- 

#ifndef PTASK_H
#define PTASK_H

#include <pthread.h>

#define _GNU_SOURCE
#define LOW_PRIO 	1		// lowest fifo priority
#define HIGH_PRIO	99		// highest fifo priority

struct task_par {
	int 	period;			// period of task in millisecond
	int 	deadline;		// relative deadline in millisecond
	int 	priority;		// priority of task [1, 99] 
	int 	dmiss;			// num of deadline misses
	struct 	timespec at;	// next activation time 
	struct 	timespec dl; 	// absolute deadline
};

//------------------------------------------
// PUBLIC: CREATE WAIT AND TERMINATION OF THREAD FUNCTIONS
//------------------------------------------

// Create task w/ routine fun and prio specified in tp, leave pthread_id in id
void p_task_create(pthread_t* id, void *(*fun) (void *), struct task_par *tp);

// Kill the thread with pthread_id id
void p_task_kill(pthread_t id);

// Wait the termination of task with pthread_id id
void wait_for_task_end(pthread_t id);

//---------------------------------
// PUBLIC: PERIOD AND DEADLINE MANAGMENT
//---------------------------------

// Reads the curr time and computes the next activ time and the deadline
void set_period(struct task_par* tp);

// Suspends the thread until the next activ and updates activ time and deadline
void wait_for_period(struct task_par* tp);

// Check if thread is in execution after deadline and return 1, otherwise 0.
int deadline_miss(struct task_par* tp);

// Simply print formatted the number of dmiss of each thread
void deadline_handle(struct task_par* tp, int n_of_thread);

//---------------------------------
// PUBLIC: MUTEX UTILITY FUNCTIONS
//---------------------------------

// Initialize a mutex sem with priority in priority inheritance protocol
void mutex_init(pthread_mutex_t* mutex_id);

// Mutex lock
void mutex_lock(pthread_mutex_t* mutex_id);

// Mutex unlock
void mutex_unlock(pthread_mutex_t* mutex_id);

//--------------------------------
// PUBLIC: GETTER AND SETTER
//--------------------------------

// Set priority and period of a task
void set_tp_param(struct task_par* tp, int period, int priority);

//--------------------------------
// PUBLIC: UTILITY
//---------------------------------

// Obtain mutual exclusion and copy len memory location from src to dest
void safe_copy(pthread_mutex_t* mutex, void* dest, void* src, size_t len);

// Obtain mutual exclusion and set to 0 from dest to len mem location
void safe_reset(pthread_mutex_t* mutex, void* dest, size_t len);

// Set to 0 from dest to len mem location
void n_safe_reset(void* dest, size_t len);

#endif
