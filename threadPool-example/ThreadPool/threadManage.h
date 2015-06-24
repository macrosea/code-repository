#ifndef _THREAD_POOL_THREAD_MANAGE_H_
#define _THREAD_POOL_THREAD_MANAGE_H_
#include <iostream>
#include <map>
#include <algorithm>
#include <stdio.h>
#include "sync.h"

class CThreadPool;
class CJob;

using namespace std;

class CThreadManage 
{ 
private: 
    CThreadPool*    m_Pool; 
    int          m_NumOfThread;
	CThreadMutex m_jobInfoMutex;
	map<int, pthread_t> m_mapJobInfo;
    map<int, pthread_t>::iterator m_itMapJobInfo;

public: 
    CThreadManage(); 
    CThreadManage(int num); 

	void AddJobInfo(CJob& job);
	void RemoveJobInfo(CJob& job);
	pthread_t GetJobThreadID(int jobID);
    virtual ~CThreadManage(); 

    void     SetParallelNum(int num);     
    void    Run(CJob* job,void* jobdata); 
    void    TerminateAll(void); 
}; 

#endif
