
#include "job.h"

CJob::CJob(char* szJobName, int jobID)
:m_pWorkThread(NULL)
,m_JobNo(jobID)
{
    m_jobState = CJob::issCreate;
    if (szJobName != NULL)
    {
        int nSize = strlen(szJobName)+1;
        m_JobName = new char[nSize];
        strncpy(m_JobName, szJobName, nSize);      
    }
    else
      m_JobName = NULL;   
}


CJob::~CJob(){
    if(NULL != m_JobName)
        delete [] m_JobName;
    m_jobState = CJob::issTerminated;
}
void CJob::SetJobName(char* jobname)
{
    if(NULL !=m_JobName)    
    {
        delete [] m_JobName;
        m_JobName = NULL;
    }
    if(NULL !=jobname)    
    {
        int nSize = strlen(jobname)+1;
        m_JobName = new char[nSize];
        strncpy(m_JobName, jobname, nSize);
    }
} 

BOOL CJob::SendMsg(int nJobID, int Msg)
{
	
}


