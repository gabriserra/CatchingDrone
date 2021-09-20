#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "task.h"

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
void non_safe_reset(void* dest, size_t len) {
	memset(dest, 0, len);
}

