#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

#include "sync.h"
#include "thread.h"
#include "string.h"


class CThreadPool;
class CJob;

class CWorkerThread:public CThread
{
public:
    CWorkerThread();
    virtual ~CWorkerThread();
    void    Run();
    void    SetJob(CJob* job,void* jobdata);
    CJob*   GetJob(void){return m_Job;}
    void    SetThreadPool(CThreadPool* thrpool);
    CThreadPool* GetThreadPool(void){return m_ThreadPool;}
    void    SetJobEndFlag(bool isEnd) { m_IsEnd = isEnd; }
    bool    GetJobEndFlag() { return m_IsEnd; }
    
public:
    CCondition   m_JobCond;
    CThreadMutex m_WorkMutex;


private:
    CThreadPool*  m_ThreadPool;
    CJob*    m_Job;
    void*    m_JobData;
    bool    m_IsEnd;
    CThreadMutex m_VarMutex;    
};


#endif
