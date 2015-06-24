#ifndef _THREAD_POOL_JOB_H_
#define _THREAD_POOL_JOB_H_

#include <iostream>
#include "string.h"
#include "thread.h"
#include "workerThread.h"

typedef int BOOL;  

using namespace std;

class CJob
{
public:
    
    typedef enum
    {
        issCreate =0,
        issIdle,  // no used
        issRunning,
        issSuspended,
        issTerminated,  // no used
        issFinished     // no used
    }JobState;

public:
    explicit CJob(char* szJobName, int jobID);
    CJob(CJob& job);
    
    virtual ~CJob();
       
    int      GetJobNo(void) const { return m_JobNo; }
    void     SetJobNo(int jobno){ m_JobNo = jobno;}
    char*    GetJobName(void) const { return m_JobName; }
    void     SetJobName(char* jobname);
    CThread *GetWorkThread(void){ return m_pWorkThread; }
    void     SetWorkThread ( CThread* pWorkThread ) 
            {                 
                m_pWorkThread = pWorkThread; 
                if ((pWorkThread != NULL) && (m_JobName == NULL))
                {
                    char szName[100];
                    sprintf(szName, "TID_%d", m_pWorkThread->GetThreadID());
                    SetJobName(szName);                     
                }
            }
    void    setJobState(const JobState state) {m_jobState = state;}
    JobState GetJobState() const {return m_jobState;}
    virtual void Run ( void *ptr ) = 0;

	  BOOL SendMsg(int nJobID, int Msg);

private:
    int         m_JobNo;        //The num was assigned to the job
    char*       m_JobName;      //The job name
    CThread*    m_pWorkThread;     //The thread associated with the job
    JobState    m_jobState; 
	
};

#endif
