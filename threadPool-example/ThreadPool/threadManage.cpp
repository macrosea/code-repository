#include "threadManage.h"
#include "threadPool.h"
#include "job.h"


CThreadManage::CThreadManage()
{ 
    m_NumOfThread = 10; 
    m_Pool = new CThreadPool(m_NumOfThread); 
}
CThreadManage::CThreadManage(int num)
{ 
    m_NumOfThread = num; 
    m_Pool = new CThreadPool(m_NumOfThread); 
}
CThreadManage::~CThreadManage()
{ 
    if(NULL != m_Pool) 
        delete m_Pool; 
}
void CThreadManage::SetParallelNum(int num)
{ 
    m_NumOfThread = num; 
}
void CThreadManage::Run(CJob* job,void* jobdata)
{ 
    m_Pool->Run(job,jobdata); 
}
void CThreadManage::TerminateAll(void)
{   
    m_Pool->TerminateAll(); 
}

void CThreadManage::AddJobInfo(CJob& job)
{
	m_jobInfoMutex.Lock();
	CThread* pWorkThread = job.GetWorkThread();
	pthread_t thrID = pWorkThread->GetThreadID();
	int jobID = job.GetJobNo();
	m_mapJobInfo.insert(pair<int, pthread_t>(jobID, thrID));
	m_jobInfoMutex.Unlock();
}

void CThreadManage::RemoveJobInfo(CJob& job)
{
	m_jobInfoMutex.Lock();
	int jobID = job.GetJobNo();
	map<int,pthread_t>::iterator it = m_mapJobInfo.find(jobID);
	if (m_mapJobInfo.end() != it)
		m_mapJobInfo.erase (it);
	m_jobInfoMutex.Unlock();
}

pthread_t CThreadManage::GetJobThreadID(int jobID)
{
	m_jobInfoMutex.Lock();
	pthread_t thrID = 0;
	map<int,pthread_t>::iterator it = m_mapJobInfo.find(jobID);
	if (m_mapJobInfo.end() != it)
		thrID = m_mapJobInfo[jobID];
	m_jobInfoMutex.Unlock();
	return thrID;
}

