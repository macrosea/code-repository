/**
 * Generic thread pool for Visual C++
 *
 * AUTHOR: Craig Mason-Jones (craig@lateral.co.za)
 * VERSION: 1.0
 * LICENCE: LPGL
 * 
 * This code uses three OS specific concepts:
 * 1. Events

 * 2. Mutex
  
			#include <pthread.h>

			pthread_mutexattr_t mattr; 
			int ret; 

			ret = pthread_mutexattr_init(&mattr);
			ret = pthread_mutex_lock(&mp);
			ret = pthread_mutex_unlock(&mp);

			int pthread_mutex_destroy(pthread_mutex_t *mp);

 * 3. beginthreadex or pthread_create

			#include <pthread.h>
			pthread_attr_t tattr;
			pthread_t tid;
			extern void *start_routine(void *arg);
			void *arg;
			int ret;
			// default behavior
			ret = pthread_create(&tid, NULL, start_routine, arg);
			// ALTERNATIVE FORMS:
			// initialized with default attributes 
			ret = pthread_attr_init(&tattr);
			// default behavior specified
			ret = pthread_create(&tid, &tattr, start_routine, arg);

 */

#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <vector>
#include <deque>

#include "OSSpecific.h"

template<class parameterT> class ThreadWrapper;

/**
 * A pool that manages threads, launching new threads as required.
 */
template <class parameterT> class ThreadPool {
public:
	typedef void (*ThreadFunctionT) (parameterT param);	// The Thread function prototype
protected:
	ThreadFunctionT threadFunction;									///< Function that will process requests
	std::vector<ThreadWrapper<parameterT>*> threads;		///< Vector of available threads
	std::deque<parameterT> params;					///< Vector of parameters still to be processed

	OSSpecific::MutexT poolAccess;		///< Control access to the thread pool

	/** The maximum number of threads to allow */
	int maxThreads;
	/** The minimum number of threads to run */
	int minThreads;
	/** The number of cycles before a thread is destroyed. */
	int maxExecutionCount;
	/** The current number of threads that exist */
	int numThreads;
public:
	static const int UnlimitedThreads = -1;
	static const int UnlimitedLifetime = -1;

	enum NextThreadState { Execute, Sleep, Die } ;
	typedef enum NextThreadState NextThreadStateT;

public:
	/**
	 * @param int nThreads Number of threads to launch initially.
   */
	ThreadPool (ThreadFunctionT pFunction, int minThreads, int maxThreads, int maxExecutionCount);

	~ThreadPool();

	ThreadFunctionT GetFunction() { return threadFunction; }

	/**
	 * Launces a thread with the given parameter.
	 */
	void Launch(parameterT param);

	/**
	 * Called by a thread rejoining the pool
	 */
	ThreadPool<parameterT>::NextThreadState Rejoin(ThreadWrapper<parameterT> *pWrapper);
};	// end of class ThreadPool<parameterT>

/*********************************************************************************************/
/****								DECLARATION OF THE ThreadWrapper CLASS									****/
/*********************************************************************************************/

/**
 * Wraps a thread that calls a function that takes a single argument of
 * type parameterT. The ThreadWrapper manages the interaction with the
 * ThreadPool.
 */
template<class parameterT> class ThreadWrapper {
public:
	static long ThreadRoutingFunction(void* pParam);
protected:
	/** Parameter for the thread */
	parameterT param;
	/** Pointer to the Pool this thread belongs to. */
	ThreadPool<parameterT> *pPool;

	/** The thread is sleeping */
	OSSpecific::ConditionT sleepingCondition;
	/** The thread should awake */
	OSSpecific::ConditionT awakeCondition;

	/** Thread is sleeping */
	//HANDLE sleepingEvent;
   /** Thread should awake */
   //HANDLE awakeEvent;

   /** Number of times the thread has executed */
	long executionCount;
	/** ID of this thread */
  //unsigned int threadId;
	OSSpecific::ThreadIdentifierT threadIdentifier;
public:
	ThreadWrapper(ThreadPool<parameterT> *pPool);

	~ThreadWrapper();

	/** Modifier for the Parameter to the thread */
	void SetParameter(parameterT param);

	/** Accessor for the Parameter to the thread */
	parameterT GetParameter();

	/** Returns the number of times the thread has executed */
	long GetExecutionCount() { return executionCount; }

	/**
	 * Sleep puts the thread to sleep until awake() is called
	 */
	void Sleep();

	/**
	 * Awakes the thread to process the given parameter.
	 * This function is not called by the thread, but by the ThreadPool.
	 *
	 */
	void Awake(parameterT param);

	/**
	 * Main execution loop of a thread in the ThreadPool.
	 * The main execution loop joins the pool, 
	 * sleeps if the pool hasn't immediately passed it data, then
	 * executes the ServerThread with the data that it has received
	 */
	long Execute();
};


/*********************************************************************************************/
/****								IMPLEMENTATION OF THE ThreadPool CLASS									****/
/*********************************************************************************************/

/**
 * @param nThreads The number of threads to launch.
 */
template <class parameterT>
ThreadPool<parameterT>::ThreadPool(ThreadFunctionT pFunction, int minThreads, int maxThreads, int maxExecutionCount)
{
	ThreadPool<parameterT>::minThreads = minThreads;
	ThreadPool<parameterT>::maxThreads = maxThreads;
	ThreadPool<parameterT>::maxExecutionCount = maxExecutionCount;
	threadFunction = pFunction;
	OSSpecific::CreateMutex(&poolAccess);
	for (numThreads=0; numThreads<minThreads; numThreads++) {
		new ThreadWrapper<parameterT>(this);
	}
}

template <class parameterT>
ThreadPool<parameterT>::~ThreadPool()
{
	OSSpecific::DestroyMutex(poolAccess);
}

/**
 * Launches a thread to process the given parameters.
 */
template <class parameterT>
void ThreadPool<parameterT>::Launch(parameterT param) {
	OSSpecific::LockMutex(poolAccess);
	if (threads.size()!=0) {
		ThreadWrapper<parameterT> *pWrapper = threads.back();
		threads.pop_back();
		OSSpecific::UnlockMutex(poolAccess);
		pWrapper->Awake(param);
		return;
	}

	// If we can still create new threads, we do so. This should catch the params as soon
	// as it has started.
	if ((maxThreads == ThreadPool<parameterT>::UnlimitedThreads) || (numThreads < maxThreads)) {
		new ThreadWrapper<parameterT>(this);
		numThreads++;
	}

	/* If no threads are available, we queue the parameters and an incoming thread
	 * will manage them when rejoin() is called.
	 * At this point, we could also consider launching a new thread to manage the request...
	 */
	params.push_back(param);
	OSSpecific::UnlockMutex(poolAccess);
}

/**
 * Called by a thread that has finished processing and is rejoining the pool.
 * @param pWrapper The wrapped thread that is rejoining the pool
 * @return bool True if the thread should continue existing. False if it should shut itself down.
 */
template <class parameterT>
typename ThreadPool<parameterT>::NextThreadStateT
ThreadPool<parameterT>::Rejoin
(ThreadWrapper<parameterT>* pWrapper) {
	OSSpecific::LockMutex(poolAccess);
	/* If there is a queued parameter, we return that to the thread */
	if (params.size()!=0) {
		parameterT param = params.front();
		params.pop_front();
		OSSpecific::UnlockMutex(poolAccess);
		pWrapper->SetParameter(param);
		return ThreadPool<parameterT>::Execute;
	}

	/* If we have more than the minimum number of threads, terminate back to the minimum */
	if (numThreads > minThreads) {
		numThreads--;
		OSSpecific::UnlockMutex(poolAccess);
		return ThreadPool<parameterT>::Die;
	}

	/* Can check how much the thread has executed, and potentially terminate it in
	 * favour of a new thread */
	if 
		((maxExecutionCount!=UnlimitedLifetime) && (pWrapper->GetExecutionCount()>maxExecutionCount))	
	{
		OSSpecific::UnlockMutex(poolAccess);
		// Launch a new thread - this is enough: the thread will join us.
		new ThreadWrapper<parameterT>(this);
		return ThreadPool<parameterT>::Die;	// Indicate to the calling thread that it should exit
	}

	/* If no parameters are queued, we queue the thread */
	threads.push_back(pWrapper);
	OSSpecific::UnlockMutex(poolAccess);
	return ThreadPool<parameterT>::Sleep;
}


/*********************************************************************************************/
/****								IMPLEMENTATION OF THE ThreadWrapper CLASS								****/
/*********************************************************************************************/

/**
 * Wrapper C function passes control through to the ThreadWrapper class
 */
/*template <class parameterT>
long ThreadWrapperThread(void* pParam);
*/
template <class parameterT>
long ThreadWrapper<parameterT>::ThreadRoutingFunction(void* pParam)
{
	ThreadWrapper<parameterT> *pWrapper = reinterpret_cast<ThreadWrapper<parameterT>*>( pParam );
	return pWrapper->Execute();	// Infinite loop in here until instructed to die.
}

/**
 * Construction of a new ThreadWrapper automatically binds it to the given
 * ThreadPool.
 */
template<class parameterT>
ThreadWrapper<parameterT>::ThreadWrapper(ThreadPool<parameterT> *pPool) {
	ThreadWrapper<parameterT>::pPool=pPool;
	executionCount=0;
	
	/* Create the Event we will use to launch and signal sleeping */
	OSSpecific::CreateCondition(&awakeCondition);
	OSSpecific::CreateCondition(&sleepingCondition);

	/* Launch the thread that will be the thread that we wrap */
	OSSpecific::BeginThread( 
		reinterpret_cast<OSSpecific::ThreadFunctionT>(ThreadWrapper<parameterT>::ThreadRoutingFunction),
		this,	// Parameter to the thread
		&threadIdentifier 
		);
}

template<class parameterT>
ThreadWrapper<parameterT>::~ThreadWrapper()
{
	OSSpecific::DestroyCondition(awakeCondition);
	OSSpecific::DestroyCondition(sleepingCondition);
}

template<class parameterT>
void ThreadWrapper<parameterT>::Awake(parameterT param)
{
	OSSpecific::WaitForCondition(sleepingCondition);
	SetParameter(param);
	OSSpecific::SignalCondition(awakeCondition);
}

template<class parameterT>
long ThreadWrapper<parameterT>::Execute() {
	typename ThreadPool<parameterT>::NextThreadStateT nextState;
	while ((nextState = pPool->Rejoin(this)) != ThreadPool<parameterT>::Die) {
		// Rejoin will set pParam if possible
		if (nextState == ThreadPool<parameterT>::Sleep) {
			Sleep();
		} 
		// We've been awoken, so we execute the function we're bound to.
		(*pPool->GetFunction()) (GetParameter());
		executionCount++;		// Track the number of executions
		SetParameter(NULL);	// So we don't loop endlessly with the same parameter
	}
	// The thread exits if the pool won't accept it's rejoin - deleting itself on exit
	delete this;
	return 0;
}

/** Accessor for the Parameter to the thread */
template<class parameterT>
parameterT ThreadWrapper<parameterT>::GetParameter() { return param; }

template<class parameterT>
void ThreadWrapper<parameterT>::SetParameter(parameterT param) {
	ThreadWrapper::param = param; 
}

template<class parameterT>
void ThreadWrapper<parameterT>::Sleep()
{
	OSSpecific::SignalCondition(sleepingCondition);
	OSSpecific::WaitForCondition(awakeCondition);
}

#endif
