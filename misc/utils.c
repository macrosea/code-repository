/*
 *
 *
 */

#include "utils.h"


void init_lock(LOCK* pLock) {
    pthread_mutex_init(&(pLock->mutex), NULL);
}

void rel_lock(LOCK* pLock) {
    pthread_mutex_destroy(&(pLock->mutex));
}

void lock(LOCK* pLock) {
    pthread_mutex_lock(&(pLock->mutex));
}

void unlock(LOCK* pLock) {
    pthread_mutex_unlock(&(pLock->mutex));
}

void init_cond(COND* pCond) {
    pthread_cond_init(&(pCond->condition), NULL);
    pthread_mutex_init(&(pCond->mutex), NULL);
    pCond->cond_set = _FALSE;
}

int wait(COND* pCond, int msec) {
    int rc = 0;
    struct timespec tout;
    tout.tv_sec = time(NULL) + msec/1000;
    tout.tv_nsec = (msec % 1000) * 1000000;
    pthread_mutex_lock(&(pCond->mutex));
    while (!(pCond->cond_set)) {
        rc = pthread_cond_timedwait(&(pCond->condition), &(pCond->mutex),
                &tout);
        if (rc == ETIMEDOUT)
            break;
    }
    pCond->cond_set = _FALSE;
    pthread_mutex_unlock(&(pCond->mutex));
    return rc;
}

void notify(COND* pCond) {
    pthread_mutex_lock(&(pCond->mutex));
    pCond->cond_set = _TRUE;
    pthread_cond_signal(&(pCond->condition));
    pthread_mutex_unlock(&(pCond->mutex));
}

void rel_cond(COND* pCond) {
    pthread_cond_destroy(&(pCond->condition));
    pthread_mutex_destroy(&(pCond->mutex));
}



//EOF
