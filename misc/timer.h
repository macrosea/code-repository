#ifndef __TIMER_H__
#define __TIMER_H__
#include <pthread.h>
#include <sys/time.h>
class CTimer
{
private:
    pthread_t thread_timer;
    long m_second, m_microsecond;
    static void * OnTimer_stub(void *p)
    {
        (static_cast<CTimer*>(p))->thread_proc();
        return NULL;
    };

    void thread_proc();
    void OnTimer();
public:
    //CTimer();
    explicit CTimer(long second, long microsecond=0);
    virtual ~CTimer();
    void SetTimer(long second,long microsecond);
    void StartTimer();
    void StopTimer();
};
#endif /* CTIMER_H_ */
