#ifndef _THREAD_POOL_THREAD_H_
#define _THREAD_POOL_THREAD_H_

//#define _TEST 1
#define _DEBUG 1


#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>


using namespace std;

#define THREAD_ID pthread_t

class CThread
{
public:

//    typedef void ThreadHandler;
    typedef sem_t Semaphore;
    
    typedef enum
    {
        issCreate =0,
        issIdle,
        issRunning,
        issSuspended,
        issTerminated,
        issFinished 
    }ThreadState;
    
    
    typedef enum
    {
        Error_ThreadSuccess = 0,    
        Error_ThreadInit,       
        Error_ThreadCreate,         
        Error_ThreadSuspend,    
        Error_ThreadResume,         
        Error_ThreadCancel,     
        Error_ThreadTerminalte,  
        Error_ThreadExit,       
        Error_ThreadSetPriority, 
    } ThreadErrorCode;
    


public: 
    CThread(); 
    CThread(bool detach); 
    virtual ~CThread(); 
    
    void     Start(void);        //Start to execute the thread 
    virtual void Run(void) = 0; 
    void     Exit(void); 
    void            SetThreadState(const ThreadState state){m_ThreadState = state;} 
    ThreadState     GetThreadState(void) const {return m_ThreadState;} 
    void 	    SetErrcode(const ThreadErrorCode errcode) {m_ErrCode = errcode;}    
    int         GetLastError(void) const {return m_ErrCode;} 
    void        SetThreadName(const char* thrName);
    const char* GetThreadName(void) const {return m_ThreadName;} 
    THREAD_ID   GetThreadID(void) const {return m_ThreadID;} 
    inline bool SetPriority(const int priority); 
    inline int  GetPriority(void) const; 
    static inline int  GetConcurrency(void){ return pthread_getconcurrency();} 
    static inline bool SetConcurrency(const int num) { return (0 == pthread_setconcurrency(num)); }
    inline bool Detach(void); 
    inline bool WaitforOtherEnd(THREAD_ID id);
    bool Join(void)  {return (0 == pthread_join(m_ThreadID, NULL));}
    inline bool Yield(void); 
    inline int  Self(void); 	

    //set the thread to sleep  
    void Sleep(int nSec);

void select_sleep(int nSec, int nuSec = 0);

#if _DEBUG
    void    Suspend(void);

    void    Resume(void);  
      
    //terminate the thread  
    void    Terminate(void); 

    size_t get_stack_size() const;

#endif

protected:
    static void* ThreadFunction(void* pThread);  

private:
//    ThreadHandler   m_Handler;    
    THREAD_ID       m_ThreadID;	
    ThreadErrorCode m_ErrCode;
//    Semaphore	    m_ThreadSemaphore;	//the inner  semaphore,which is used to realize
    bool    m_Detach;
 //   bool	m_CreateSuspended;	//if suspend after creating
    char*	m_ThreadName; 
    ThreadState m_ThreadState;		//the state of the thread
};


#endif


/*

class CThread 
{ 
private: 
    int          m_ErrCode; 
    Semaphore    m_ThreadSemaphore;  //the inner semaphore, which is used to realize 
    unsigned     long m_ThreadID;    
    bool         m_Detach;       //The thread is detached 
    bool         m_CreateSuspended;  //if suspend after creating 
    char*        m_ThreadName; 
    ThreadState m_ThreadState;      //the state of the thread

protected: 
    void     SetErrcode(int errcode){m_ErrCode = errcode;} 
    static void* ThreadFunction(void*);

public: 
    CThread(); 
    CThread(bool createsuspended,bool detach); 
    virtual ~CThread(); 

    virtual void Run(void) = 0; 
    void     SetThreadState(ThreadState state){m_ThreadState = state;} 
    bool     Terminate(void);    //Terminate the threa 
    bool     Start(void);        //Start to execute the thread 
    void     Exit(void); 
    bool     Wakeup(void);
    ThreadState  GetThreadState(void){return m_ThreadState;} 
    int      GetLastError(void){return m_ErrCode;} 
    void     SetThreadName(char* thrname){strcpy(m_ThreadName,thrname);} 
    char*    GetThreadName(void){return m_ThreadName;} 
    int      GetThreadID(void){return m_ThreadID;} 
    bool     SetPriority(int priority); 
    int      GetPriority(void); 
    int      GetConcurrency(void); 
    void     SetConcurrency(int num); 
    bool     Detach(void); 
    bool     Join(void); 
    bool     Yield(void); 
    int      Self(void); 
};


*/

/*

bool CThread::Run(TRunMode flags)
{
    // Do not allow the new thread to run until m_Handle is set
    CFastMutexGuard state_guard(s_ThreadMutex);

    // Check
    xncbi_Validate(!m_IsRun,
                   "CThread::Run() -- called for already started thread");

    m_IsDetached = (flags & fRunDetached) != 0;

#if defined NCBI_THREAD_PID_WORKAROUND
    CProcess::sx_GetPid(CProcess::ePID_GetCurrent);
#endif

    // Thread will run - increment counter under mutex
    ++sm_ThreadsCount;
    try {

#if defined(NCBI_WIN32_THREADS)
        // We need this parameter in WinNT - can not use NULL instead!
        DWORD thread_id;
        // Suspend thread to adjust its priority
        DWORD creation_flags = (flags & fRunNice) == 0 ? 0 : CREATE_SUSPENDED;
        m_Handle = CreateThread(NULL, 0, ThreadWrapperCallerImpl,
                                this, creation_flags, &thread_id);
        xncbi_Validate(m_Handle != NULL,
                       "CThread::Run() -- error creating thread");
        if (flags & fRunNice) {
            // Adjust priority and resume the thread
            SetThreadPriority(m_Handle, THREAD_PRIORITY_BELOW_NORMAL);
            ResumeThread(m_Handle);
        }
        if ( m_IsDetached ) {
            CloseHandle(m_Handle);
            m_Handle = NULL;
        }
        else {
            // duplicate handle to adjust security attributes
            HANDLE oldHandle = m_Handle;
            xncbi_Validate(DuplicateHandle(GetCurrentProcess(), oldHandle,
                                           GetCurrentProcess(), &m_Handle,
                                           0, FALSE, DUPLICATE_SAME_ACCESS),
                           "CThread::Run() -- error getting thread handle");
            xncbi_Validate(CloseHandle(oldHandle),
                           "CThread::Run() -- error closing thread handle");
        }
#elif defined(NCBI_POSIX_THREADS)
        pthread_attr_t attr;
        xncbi_Validate(pthread_attr_init (&attr) == 0,
                       "CThread::Run() - error initializing thread attributes");
        if ( ! (flags & fRunUnbound) ) {
#if defined(NCBI_OS_BSD)  ||  defined(NCBI_OS_CYGWIN)  ||  defined(NCBI_OS_IRIX)
            xncbi_Validate(pthread_attr_setscope(&attr,
                                                 PTHREAD_SCOPE_PROCESS) == 0,
                           "CThread::Run() - error setting thread scope");
#else
            xncbi_Validate(pthread_attr_setscope(&attr,
                                                 PTHREAD_SCOPE_SYSTEM) == 0,
                           "CThread::Run() - error setting thread scope");
#endif
        }
        if ( m_IsDetached ) {
            xncbi_Validate(pthread_attr_setdetachstate(&attr,
                                                       PTHREAD_CREATE_DETACHED) == 0,
                           "CThread::Run() - error setting thread detach state");
        }
        xncbi_Validate(pthread_create(&m_Handle, &attr,
                                      ThreadWrapperCallerImpl, this) == 0,
                       "CThread::Run() -- error creating thread");

        xncbi_Validate(pthread_attr_destroy(&attr) == 0,
                       "CThread::Run() - error destroying thread attributes");

#else
        if (flags & fRunAllowST) {
            Wrapper(this);
        }
        else {
            xncbi_Validate(0,
                           "CThread::Run() -- system does not support threads");
        }
#endif

        // prevent deletion of CThread until thread is finished
        m_SelfRef.Reset(this);

    }
    catch (...) {
        // In case of any error we need to decrement threads count
        --sm_ThreadsCount;
        throw;
    }

    // Indicate that the thread is run
    m_IsRun = true;
    return true;
}


void CThread::Detach(void)
{
    CFastMutexGuard state_guard(s_ThreadMutex);

    // Check the thread state: it must be run, but not detached yet
    xncbi_Validate(m_IsRun,
                   "CThread::Detach() -- called for not yet started thread");
    xncbi_Validate(!m_IsDetached,
                   "CThread::Detach() -- called for already detached thread");

    // Detach the thread
#if defined(NCBI_WIN32_THREADS)
    xncbi_Validate(CloseHandle(m_Handle),
                   "CThread::Detach() -- error closing thread handle");
    m_Handle = NULL;
#elif defined(NCBI_POSIX_THREADS)
    xncbi_Validate(pthread_detach(m_Handle) == 0,
                   "CThread::Detach() -- error detaching thread");
#endif

    // Indicate the thread is detached
    m_IsDetached = true;

    // Schedule the thread object for destruction, if already terminated
    if ( m_IsTerminated ) {
        m_SelfRef.Reset();
    }
}


void CThread::Join(void** exit_data)
{
    // Check the thread state: it must be run, but not detached yet
    {{
        CFastMutexGuard state_guard(s_ThreadMutex);
        xncbi_Validate(m_IsRun,
                       "CThread::Join() -- called for not yet started thread");
        xncbi_Validate(!m_IsDetached,
                       "CThread::Join() -- called for detached thread");
        xncbi_Validate(!m_IsJoined,
                       "CThread::Join() -- called for already joined thread");
        m_IsJoined = true;
    }}

    // Join (wait for) and destroy
#if defined(NCBI_WIN32_THREADS)
    xncbi_Validate(WaitForSingleObject(m_Handle, INFINITE) == WAIT_OBJECT_0,
                   "CThread::Join() -- can not join thread");
    DWORD status;
    xncbi_Validate(GetExitCodeThread(m_Handle, &status) &&
                   status != DWORD(STILL_ACTIVE),
                   "CThread::Join() -- thread is still running after join");
    xncbi_Validate(CloseHandle(m_Handle),
                   "CThread::Join() -- can not close thread handle");
    m_Handle = NULL;
#elif defined(NCBI_POSIX_THREADS)
    xncbi_Validate(pthread_join(m_Handle, 0) == 0,
                   "CThread::Join() -- can not join thread");
#endif

    // Set exit_data value
    if ( exit_data ) {
        *exit_data = m_ExitData;
    }

    // Schedule the thread object for destruction
    {{
        CFastMutexGuard state_guard(s_ThreadMutex);
        m_SelfRef.Reset();
    }}
}


void CThread::Exit(void* exit_data)
{
    // Don't exit from the main thread
    CThread* x_this = GetCurrentThread();
    xncbi_Validate(x_this != 0,
                   "CThread::Exit() -- attempt to call for the main thread");

    {{
        CFastMutexGuard state_guard(s_ThreadMutex);
        x_this->m_ExitData = exit_data;
    }}

    // Throw the exception to be caught by Wrapper()
    throw CExitThreadException();
}


bool CThread::Discard(void)
{
    CFastMutexGuard state_guard(s_ThreadMutex);

    // Do not discard after Run()
    if ( m_IsRun ) {
        return false;
    }

    // Schedule for destruction (or, destroy it right now if there is no
    // other CRef<>-based references to this object left).
    m_SelfRef.Reset(this);
    m_SelfRef.Reset();
    return true;
}


void CThread::OnExit(void)
{
    return;
}


void *cThread::threadfunc(void *p)
{
	cThread *pthread=(cThread *)p;
	int ret=(*(pthread->m_pFunc))(pthread->m_pArgs);
	pthread->m_thrid=0;
	pthread_M_EXIT(NULL);
	return NULL;//no use
}

cThread::cThread()
{
	m_thrid=0;
	m_pArgs=NULL;
	m_pFunc=NULL;
}
cThread::cThread(FUNC_THREAD *pfunc)
{
	m_thrid=0;
	m_pArgs=NULL;
	m_pFunc=pfunc;
}

cThread::~cThread()
{
	detach();
}

bool cThread::operator==(const cThread& other) const
{
	return other.m_thrid==m_thrid;
}

bool cThread::operator!=(const cThread& other) const
{
    return !operator==(other);
}
//分离线程，不等待其结束
int cThread::detach()
{
	if(m_thrid!=0)
		pthread_M_DETACH(m_thrid);
	m_thrid=0;
	return 0;
}

//启动线程 0-OK 
int cThread::start(void * pArgs)
{
	int retv=0;
	if(m_thrid!=0) return 0;
	if(m_pFunc!=NULL)
	{
		m_pArgs=pArgs;
		pthread_M_CREATE(&retv,&m_thrid,threadfunc,(void *)this);
	}
	return retv;
}
//等待线程结束 0-OK
int cThread::join()
{
	int retv=0;
	pthread_t pid=m_thrid;
	if(pid!=0)
	{
		pthread_M_KILL(pid,SIGALRM);
		pthread_M_JOIN(pid,NULL);
	}
	return retv;
}

============================================
void* thread_proc(void* thread_param)
{
    CThread* thread = (CThread *)thread_param;
    //thread->inc_refcount(); // start中已经调用，可以确保这里可以安全的使用thread指针

    thread->run();
    thread->dec_refcount();
    return NULL;
}

CThread::CThread()
    :_stop(false)
    ,_stack_size(0)
{
    int retval = pthread_attr_init(&_attr);
    if (retval != 0)
    {
        throw CSyscallException(retval, __FILE__, __LINE__);
    }
}

CThread::~CThread()
{
	pthread_attr_destroy(&_attr);
}

uint32_t CThread::get_current_thread_id()
{
    return pthread_self();
}

void CThread::stop(bool wait_stop)
{
    _stop = true;
    if (wait_stop && can_join())
        join();
}

bool CThread::start(bool detach)
{
    if (!before_start()) return false;

    // 如果本过程成功，则线程体run结束后再减引用计数，
    // 否则在失败的分支减引用计数
    this->inc_refcount();

    int retval = 0;

    // 设置线程栈大小
    if (_stack_size > 0)
        retval = pthread_attr_setstacksize(&_attr, _stack_size);
    if (0 == retval)
        retval = pthread_attr_setdetachstate(&_attr, detach?PTHREAD_CREATE_DETACHED:PTHREAD_CREATE_JOINABLE);
       
    if (0 == retval)
        retval = pthread_create(&_thread , &_attr, thread_proc, this);

    if (retval != 0)
    {
        this->dec_refcount();
        throw CSyscallException(retval, __FILE__, __LINE__);
    }

    return true;
}

size_t CThread::get_stack_size() const
{
    size_t stack_size = 0;
    int retval = pthread_attr_getstacksize(&_attr, &stack_size);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);

    return stack_size;
}

void CThread::join()
{
    int retval = pthread_join(_thread, NULL);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);
}

void CThread::detach()
{
    int retval = pthread_detach(_thread);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);
}

bool CThread::can_join() const
{
    int detachstate;
    int retval = pthread_attr_getdetachstate(&_attr, &detachstate);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);

    return (PTHREAD_CREATE_JOINABLE == detachstate);
}



//======================

#include "CThread.h"
namespace gateway {
LoggerPtr CThread::logger(Logger::getLogger("gateway.CThread"));
CThread::CThread() {
  runflag=true;
  this->m_Detach=false;
}
CThread::CThread(bool detach){
  this->m_Detach=detach;
  runflag=true;
}
CThread::~CThread() {
}
void CThread::Start(void){
pthread_attr_init(&attr);
if(m_Detach==true){
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
}else{
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
}
try{
int rc = pthread_create(&m_ThreadID, &attr, ThreadFunction,(void*)this);
if (rc) {
return ;
}
}
catch(exception& e){

LOG4CXX_ERROR(logger, "create CThread fail!");

}

}
void CThread::Exit(void){
pthread_exit(NULL);
}
bool CThread::Detach(void){
int rc=pthread_detach(m_ThreadID);
if(rc==0){
return true;
}else{
return false;
}
}

bool CThread::Join(void){
int ret=-1;
try{
ret=pthread_join(m_ThreadID,NULL);
}catch(exception& e){
LOG4CXX_ERROR(logger, e.what());
}

if(ret==0) return true;
else return false;
}
void CThread::Yield(void){
//pthread_yield();
}
int CThread::Self(void){
return (int)pthread_self();
}


*/
