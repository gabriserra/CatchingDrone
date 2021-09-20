//-----------------------------------------------------------------------------
// TASK_H: MANAGE EASILY PERIODIC FIFO TASKS
//----------------------------------------------------------------------------- 

#ifndef TASK_H
#define TASK_H

#include <pthread.h>

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
// PUBLIC: UTILITY
//---------------------------------

// Obtain mutual exclusion and copy len memory location from src to dest
void safe_copy(pthread_mutex_t* mutex, void* dest, void* src, size_t len);

// Obtain mutual exclusion and set to 0 from dest to len mem location
void safe_reset(pthread_mutex_t* mutex, void* dest, size_t len);

// Set to 0 from dest to len mem location
void non_safe_reset(void* dest, size_t len);

#endif
