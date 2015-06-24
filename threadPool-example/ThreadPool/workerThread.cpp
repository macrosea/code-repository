#include "workerThread.h"
#include "threadPool.h"
#include "job.h"


CWorkerThread::CWorkerThread()
{
    m_Job = NULL;
    m_JobData = NULL;
    m_ThreadPool = NULL;
    m_IsEnd = false;
}
CWorkerThread::~CWorkerThread()
{
    if(NULL != m_Job)
        delete m_Job;
    if(m_ThreadPool != NULL)
        delete m_ThreadPool;
}
 
void CWorkerThread::Run()
{
    SetThreadState(CThread::issCreate);
    for(;;)
    {   
//           char szInfo[50];
//           sprintf(szInfo, "Thread ID: %d  issSuspended\n", GetThreadID());
//           cout << szInfo;
        SetThreadState(CThread::issSuspended);
        while((m_Job == NULL) && (m_IsEnd == false))
            m_JobCond.Wait();

        if (NULL != m_Job)
        {
            m_Job->setJobState(CJob::issRunning);
            SetThreadState(CThread::issRunning);
            m_Job->Run(m_JobData);
            m_Job->SetWorkThread(NULL);
            delete m_Job;
            m_Job = NULL;
        }
        
        if (m_IsEnd)
        {
            SetThreadState(CThread::issTerminated);
            m_WorkMutex.Unlock();  
            //WARNING:  do we need unlock?  how do this thread release the resource, 
            //                 and the thread should delete from list
            return;
        }
        
        SetThreadState(CThread::issIdle);
        m_ThreadPool->MoveToIdleList(this);
        if(m_ThreadPool->m_IdleList.size() > m_ThreadPool->GetAvailHighNum())
        {
            m_ThreadPool->DeleteIdleThread(m_ThreadPool->m_IdleList.size()-m_ThreadPool->GetInitNum());
        }
        m_WorkMutex.Unlock();
    }
}
void CWorkerThread::SetJob(CJob* job,void* jobdata)
{
    m_VarMutex.Lock();
    m_Job = job;
    m_JobData = jobdata;
    job->SetWorkThread(this);
    m_VarMutex.Unlock();
    m_JobCond.Signal();
}
void CWorkerThread::SetThreadPool(CThreadPool* thrpool)
{
    m_VarMutex.Lock();
    m_ThreadPool = thrpool;
    m_VarMutex.Unlock();
}
