#include "timer.h"
#include <iostream>
#include <sys/select.h>
#include <time.h>
#include <pthread.h>
using namespace std;

CTimer::CTimer(long second, long microsecond) :
    m_second(second), m_microsecond(microsecond)
{
}
CTimer::~CTimer()
{
}
void CTimer::SetTimer(long second, long microsecond)
{
    m_second = second;
    m_microsecond = microsecond;
}
void CTimer::StartTimer()
{
    pthread_create(&thread_timer, NULL, OnTimer_stub, this);
}
void CTimer::StopTimer()
{
    pthread_cancel(thread_timer);
    pthread_join(thread_timer, NULL); //wait the thread stopped
}

void CTimer::thread_proc()
{
    while (true)
    {
        OnTimer();
        pthread_testcancel();
        struct timeval tempval;
        tempval.tv_sec = m_second;
        tempval.tv_usec = m_microsecond;
        select(0, NULL, NULL, NULL, &tempval);
    }
}
void CTimer::OnTimer()
{
    cout<<"Timer once..."<<endl;
}


#include <iostream>
using namespace std;
int main()
{
    CTimer t1(1,0),t2(1,0);    //构造函数，设两个定时器，以1秒为触发时间。参数1是秒，参数2是微秒。
    t1.StartTimer();
    t2.StartTimer();
    sleep(10);
    return 0;
}
