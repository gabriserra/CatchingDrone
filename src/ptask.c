#include "ptask.h"
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <string.h>
#include <errno.h>

//---------------------------------
// PRIVATE: TIME UTILITY FUNCTIONS
//---------------------------------

// ---
// Copies a source time variable ts in a destination variable pointed by td
// timespec* td: pointer to destination timespec data structure
// timespec ts: source timespec data structure
// return: void
// ---
static void time_copy(struct timespec* td, struct timespec ts) {
    td->tv_sec  = ts.tv_sec;
    td->tv_nsec = ts.tv_nsec;
}

// ---
// Adds a value ms expressed in milliseconds to the time variable pointed by t
// timespec* t: pointer to timespec data structure
// int ms: value in milliseconds to add to t
// return: void
// ---
static void time_add_ms(struct timespec *t, int ms) {
	t->tv_sec += ms/1000;			 // convert ms to sec and add to sec
    t->tv_nsec += (ms%1000)*1000000; // convert and add the remainder to nsec
	
	// if nsec is greater than 10^9 means has reached 1 sec
	if (t->tv_nsec > 1000000000) { 
		t->tv_nsec -= 1000000000; 
		t->tv_sec += 1;
	}
}

// ---
// Compares time var t1, t2 and returns 0 if are equal, 1 if t1>t2, ‐1 if t1<t2
// timespec t1: first timespec data structure
// timespec t2: second timespec data structure
// return: int - 1 if t1 > t2, -1 if t1 < t2, 0 if they are equal
// ---
static int time_cmp(struct timespec t1, struct timespec t2) {
	// at first sec value is compared
	if (t1.tv_sec > t2.tv_sec) 
		return 1; 
	if (t1.tv_sec < t2.tv_sec) 
		return -1;
	//  at second nano sec value is compared
	if (t1.tv_nsec > t2.tv_nsec) 
		return 1; 
	if (t1.tv_nsec < t2.tv_nsec) 
		return -1; 
	return 0;
}

//-------------------------------------------------------
// PUBLIC: CREATE WAIT AND TERMINATION OF THREAD FUNCTIONS
//-------------------------------------------------------

// ---
// Create task w/ routine fun and prio specified in tp, leave pthread_id in id
// pthread_t* id: pointer to pthread_t in which will be leaved the id of task
// void *(*fun) (void *): pointer to starting routine of thread
// task_par* tp: pointer to tp data structure
// return: void
// ---
void p_task_create(pthread_t* id, void *(*fun) (void *), struct task_par* tp) {		
	pthread_attr_t t_att; 
	struct sched_param t_sched_param;

	pthread_attr_init(&t_att);
	pthread_attr_setinheritsched(&t_att, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&t_att, SCHED_FIFO); // FIFO scheduling
	t_sched_param.sched_priority = tp->priority;
	pthread_attr_setschedparam(&t_att, &t_sched_param);
	
	pthread_create(id, &t_att, fun, NULL);
}

// ---
// Kill the thread with pthread_id id
// pthread_t id: pthread_t id of thread that has to be killed
// return: void
// ---
void p_task_kill(pthread_t id) {
	pthread_cancel(id);
}

// ---
// Wait the termination of task with pthread_id id
// pthread_t id: pthread_t id of thread that has to be joined
// return: void
// ---
void wait_for_task_end(pthread_t id) {
	pthread_join(id, NULL);
}

//---------------------------------
// PUBLIC: PERIOD AND DEADLINE MANAGMENT
//---------------------------------

// ---
// Reads the curr time and computes the next activ time and the deadline
// task_par* tp: pointer to tp data structure of the thread
// return: void
// ---
void set_period(struct task_par* tp) {
	struct timespec t;
	
	// get current clock value
	clock_gettime(CLOCK_MONOTONIC, &t); 
	time_copy(&(tp->at), t); 
	time_copy(&(tp->dl), t);

	// adds period and deadline 
	time_add_ms(&(tp->at), tp->period); 
	time_add_ms(&(tp->dl), tp->deadline);
}

// ---
// Suspends the thread until the next activ and updates activ time and deadline
// task_par* tp: pointer to tp data structure of the thread
// return: void
// ---
void wait_for_period(struct task_par* tp) {
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(tp->at), NULL);
	time_add_ms(&(tp->at), tp->period);
	time_add_ms(&(tp->dl), tp->period);
}

// ---
// Check if thread is in execution after deadline and return 1, otherwise 0.
// task_par* tp: pointer to tp data structure of the thread
// return: int - 1 if thread has executed after deadline, 0 otherwise
// ---
int deadline_miss(struct task_par* tp) {
	struct timespec now;
	
	// get the clock time and compare to abs deadline
	clock_gettime(CLOCK_MONOTONIC, &now);
	if (time_cmp(now, tp->dl) > 0) { 
		tp->dmiss++;
		return 1; 
	}
	
	return 0;
}

// ---
// Simply print formatted the number of dmiss of each thread
// task_par* tp: pointer to Vector[n_of_thread] of tp data structure
// return: int - 1 if thread has executed after deadline, 0 otherwise
// ---
void deadline_handle(struct task_par* tp, int n_of_thread) {
	int 	i;	// array indexes [0-n_of_thread]

	printf("-----------------------------------------------\n");
	printf("MISSED DEADLINE NUMBER:\n");
	for(i = 0; i < n_of_thread; i++)
		printf("\tThread num: %d - dmiss: %d\n", i, tp[i].dmiss);
	printf("-----------------------------------------------\n");
}

//---------------------------------
// PUBLIC: MUTEX UTILITY FUNCTIONS
//---------------------------------

// ---
// Initialize a mutex sem with priority in priority inheritance protocol
// pthread_mutex_t* mutex_id: will be leaved the id of mutex
// return: void
// ---
void mutex_init(pthread_mutex_t* mutex_id) {
	pthread_mutexattr_t mattr;
	pthread_mutexattr_init(&mattr);

	// sem is set to robust to protect from inconsistency 
	// and use inheritance protocol
	pthread_mutexattr_setrobust(&mattr, PTHREAD_MUTEX_ROBUST);
	pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_INHERIT);

	pthread_mutex_init(mutex_id, &mattr);
	pthread_mutexattr_destroy(&mattr);
}

// ---
// Mutex lock
// pthread_mutex_t* mutex_id: pointer to identifier of the mutex
// return: void
// ---
void mutex_lock(pthread_mutex_t* mutex_id) {
	int ret_value = pthread_mutex_lock(mutex_id);

	// if previous owner of semaphore is dead
	// we have to make sem consisten again
	if(ret_value == EOWNERDEAD)
		pthread_mutex_consistent(mutex_id);
}

// ---
// Mutex unlock
// pthread_mutex_t* mutex_id: pointer to identifier of the mutex
// return: void
// ---
void mutex_unlock(pthread_mutex_t* mutex_id) {
	pthread_mutex_unlock(mutex_id);
}

//--------------------------------
// PUBLIC: GETTER AND SETTER
//--------------------------------

// ---
// Set priority and period of a task
// task_par* tp: pointer to tp data structure of the thread
// int period: desidered period of the thread (ms)
// int priority: desidered priority of the thread [1 low - 99 high]
// return: void
// ---
void set_tp_param(struct task_par* tp, int period, int priority) {
	// if prio is not a possible one, set default to lowest
	if(priority < LOW_PRIO || priority > HIGH_PRIO)
		priority = LOW_PRIO;

	tp->priority = priority;
	tp->period = period;
	tp->deadline = period;
}

//--------------------------------
// PUBLIC: UTILITY
//---------------------------------

// ---
// Obtain mutual exclusion and copy len memory location from src to dest
// pthread_mutex_t* mutex_id: pointer to id of the mutex to be used
// void* dest: pointer to data structure that received the data
// void* src: pointer to data structure that has to be copied
// size_t len: number of byte to be copied from src to dest
// return: void
// ---
void safe_copy(pthread_mutex_t* mutex, void* dest, void* src, size_t len) {
	mutex_lock(mutex);
	memcpy(dest, src, len);
	mutex_unlock(mutex);
}

// ---
// Obtain mutual exclusion and set to 0 from dest to len mem location
// pthread_mutex_t* mutex_id: pointer to id of the mutex to be used
// void* dest: pointer to data structure that will be 0-filled
// size_t len: number of byte to be 0-filled
// return: void
// ---
void safe_reset(pthread_mutex_t* mutex, void* dest, size_t len) {
	mutex_lock(mutex);
	memset(dest, 0, len);
	mutex_unlock(mutex);
}

// ---
// Set to 0 from dest to len mem location
// void* dest: pointer to data structure that will be 0-filled
// size_t len: number of byte to be 0-filled
// return: void
// ---
void n_safe_reset(void* dest, size_t len) {
	memset(dest, 0, len);
}