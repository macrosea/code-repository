#include "thread.h"  
#include <unistd.h> 

#define LOG4CXX_ERROR(para1, strPara2) 

//   gcc -o thread.o -g -lpthread sync.cpp -lstdc++

CThread::CThread(bool detach)  
{   
    SetThreadState(CThread::issCreate);  
  
    m_ThreadName = NULL;  
    m_Detach = detach; 
}  

CThread::CThread()
{
    m_Detach = false;
}

  
CThread::~CThread()  
{  
    if(NULL != m_ThreadName)  
        delete [] m_ThreadName;  
      
}  


void CThread::Start(void)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    if(m_Detach==true)
    {  
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    }
    else
    {
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    }
    
    try
    {
        int rc = pthread_create(&m_ThreadID, &attr, ThreadFunction,(void*)this);
        if (rc) 
        {
            return;
        }
    }
    catch(exception& e)
    {        
        LOG4CXX_ERROR(logger, "create CThread fail!");        
    }

}

void* CThread::ThreadFunction(void* pThread)
{
    ((CThread*)pThread)->Run();
}



void CThread::Exit(void) 
{  
    pthread_exit(NULL);    
}

bool CThread::Detach(void)
{
    return (0 == pthread_detach(m_ThreadID));
}

bool CThread::WaitforOtherEnd(pthread_t id)
{
    int ret=-1;
    try{
        ret = pthread_join(id, NULL);
    }catch(exception& e){
        LOG4CXX_ERROR(logger, e.what());
    }
    return (ret==0);
}

//bool CThread::Join(void)
//{
//   return (0 == pthread_join(m_ThreadID, NULL));
//}

bool CThread::Yield(void)
{
  return (0 == sched_yield());
}

int CThread::Self(void){
    return (int)pthread_self();
}

bool CThread::SetPriority(int priority)
{
 //   assert(priority >= 0 && priority <= 99 );
    if (priority < 0 || priority > 99 )
    {
        printf("Invalid thread's priority(%ld)\n", (long) priority );
        return false;
    }
    // Set thread¡¯s prority
    struct sched_param sched;
    sched.sched_priority = priority;
    // We use RR to act as the default scheduler
    return (0 == pthread_setschedparam(m_ThreadID, SCHED_RR, &sched ));
}

int CThread::GetPriority(void) const
{
    sched_param  param;     
    int    priority;     
    int    policy;     
    int    ret;     
    /* scheduling parameters of target thread */     
    if (0 == pthread_getschedparam (m_ThreadID, &policy, &param))   
        return param.sched_priority;
    else
        return -1;
}

void CThread::SetThreadName(const char* thrName)   
{   
    int nSize = strlen(thrName) + 1;
    delete [] m_ThreadName;
    thrName = new char[nSize];
    strncpy(m_ThreadName, thrName, nSize);                    
} 

//set the thread to sleep  
void CThread::Sleep(int nSec)  
{  
    usleep( nSec * 1000000);
} 

void CThread::select_sleep(int nSec, int nuSec) //more accurate
{
    struct timeval timeout;

    timeout.tv_sec = nSec;
    timeout.tv_usec = nuSec;
    select(1, NULL, NULL, NULL, & timeout );
}  


#if _DEBUG
/* 
  About Suspend and Resume. POSIX does not support suspending/resuming a 
thread. 
  Suspending a thread is considerd dangerous since it is not guaranteed where 
the 
  thread would be suspend. It might be holding a lock, mutex or it might be 
inside 
  a critical section.  In order to simulate it in Linux we've used signals. To 
  suspend, a thread SIGSTOP is sent and to resume, SIGCONT is sent. Note that 
this 
  is Linux only i.e. according to POSIX if a thread receives SIGSTOP then the 
  entire process is stopped. However Linux doesn't entirely exhibit the POSIX-
mandated 
  behaviour. If and when it fully complies with the POSIX standard then 
suspend 
  and resume won't work. 
*/  
void CThread::Suspend(void)  
{ /* 
    SetThreadState(issSuspended);  
    if(pthread_kill(m_ThreadID,SIGSTOP) !=0)  
        SetErrcode(Error_ThreadSuspend);  
    else  
        SetErrcode(Error_ThreadSuccess);  

    SetThreadState(issSuspended);  
    */
}  

//send the signal SIGCONT  
void CThread::Resume(void)  
{  /*
    SetThreadState(issRunnding);  
    */
}  
  
//terminate the thread  
void CThread::Terminate(void)  
{  
       
}  
  
size_t CThread::get_stack_size() const
{
    size_t stack_size = 0;
    pthread_attr_t _attr;
    int retval = pthread_attr_getstacksize(&_attr, &stack_size);
    return (retval != 0)?-1:stack_size;
}


//this is cpp file

#endif


///////////////////////////////
//   Test code
//
///////////////////////////////

#ifdef _TEST

class CTestThread: public CThread
{
public:
    CTestThread()
    {
        pthread_mutex_init( &m_Mutex, NULL );
        
    }
    ~CTestThread()
    {
        pthread_mutex_destroy(&m_Mutex);
    }
    void Run()
    {
        unsigned int nTimeTestSec = 0;        /* sec */
        unsigned int nTimeTest = 0;        /* usec */
        unsigned int nDelay = 0;        /* usec */
        struct timeval tvBegin;
        struct timeval tvNow;    
        char szTemp[100];
        gettimeofday (&tvBegin, NULL);  
       
        pthread_mutex_lock(&m_Mutex); 
        select_sleep(1, 1000);
        gettimeofday (&tvNow, NULL);
        nTimeTest = (tvNow.tv_sec - tvBegin.tv_sec) * 1000000 + \
                     tvNow.tv_usec - tvBegin.tv_usec;
        sprintf(szTemp, "ThreadID: %d; nTimeTest: %d; stack_size: %d\n", \
                        GetThreadID(), nTimeTest, get_stack_size());
        cout << szTemp;
        pthread_mutex_unlock(&m_Mutex);
    }

private:
    pthread_mutex_t      m_Mutex;
};


int main()
{
    cout << "This test program for CThread!!" << endl;
    CTestThread* test = new CTestThread[10];
    int n = 0;
    for (n = 0; n < 10; n++ )
    {    
        test[n].Start();
        //test[n].Sleep(1);
    }

    for (n = 0; n < 10; n++ )
        test[n].Join();

    cout << "end test for CThread!!" << endl;
    return 0;
}


#endif 

