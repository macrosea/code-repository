#ifndef WIN32

#pragma message("COMPILING POSIX");
#include "OSSpecific.h"

#include <pthread.h>

/*
PosixSpecific::PosixSpecific() {
	// Might need to call pthread_once () .. ?
}
*/

void PosixSpecific::CreateMutex(PosixSpecific::MutexT* pMutex) {
	pthread_mutex_init(pMutex, NULL);
}

void PosixSpecific::LockMutex(PosixSpecific::MutexT& mutex) {
	pthread_mutex_lock(&mutex);
}

void PosixSpecific::UnlockMutex(PosixSpecific::MutexT& mutex) {
	pthread_mutex_unlock(&mutex);
}
void PosixSpecific::DestroyMutex(PosixSpecific::MutexT& mutex) {
	pthread_mutex_destroy(&mutex);
}

void PosixSpecific::BeginThread(PosixSpecific::ThreadFunctionT pFunction, void* arg, PosixSpecific::ThreadIdentifierT* pThreadId ) {
  pthread_create(pThreadId, NULL, pFunction, arg);
}

void PosixSpecific::CreateCondition(PosixSpecific::ConditionT* pCondition) {
	pthread_cond_init( &(pCondition->condition) , NULL);
	pthread_mutex_init( &(pCondition->mutex), NULL);
	pCondition->condition_set = false;
}

void PosixSpecific::WaitForCondition(PosixSpecific::ConditionT& condition) {
	pthread_mutex_lock( &(condition.mutex) );
	while (!(condition.condition_set)) {
	  pthread_cond_wait(&(condition.condition), &(condition.mutex));
	}
	condition.condition_set = false;
	pthread_mutex_unlock( (&condition.mutex) );
}

void PosixSpecific::SignalCondition(PosixSpecific::ConditionT& condition) {
	pthread_mutex_lock( &(condition.mutex) );
	condition.condition_set = true;
	pthread_cond_signal(&(condition.condition));
	pthread_mutex_unlock( &(condition.mutex) );
}

void PosixSpecific::DestroyCondition(PosixSpecific::ConditionT& condition) {
	pthread_cond_destroy( &(condition.condition) );
	pthread_mutex_destroy( &(condition.mutex) );
}

#endif
