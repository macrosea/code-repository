
/*  
*   Created by XOUYANG  Sep. 20, 2016
*
*/

#ifndef __UTILS_H__
#define __UTILS_H__
#include <errno.h>
#include <pthread.h>


#define _BOOL int
#define _TRUE 1
#define _FALSE 0

typedef struct lock_t {
    pthread_mutex_t mutex;
} LOCK;

typedef struct cond_t {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    _BOOL cond_set;
} COND;


void init_lock(LOCK* pLock) ;
void rel_lock(LOCK* pLock) ;
void lock(LOCK* pLock) ;
void unlock(LOCK* pLock) ;
void init_cond(COND* pCond) ;
int wait(COND* pCond, int msec) ;
void notify(COND* pCond) ;
void rel_cond(COND* pCond) ;

#endif
// EOF
