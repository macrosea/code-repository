
#include <stdio.h>
#include <stdlib.h>
#include "../ThreadPool/job.h"
#include "../ThreadPool/threadManage.h"

#include <time.h>
#include <sys/time.h>


class CXJob:public CJob
{
public:
    explicit CXJob(char* szJobName, int jobID):CJob(szJobName, jobID){}
    virtual ~CXJob(){ printf("Job Name %send; !!!!!!!!!!!!!\n", GetJobName());}
    
    void Run(void* jobdata)    
    {
        CThreadManage* pThreadManage = (CThreadManage*)jobdata;
        pThreadManage->AddJobInfo(*this);
        srand((unsigned)time(0));
        int nmSec = (rand() % 20) * 500;
        printf("The Job comes from CXJOB,%s delay %d ms\n",  GetJobName(), nmSec);
        usleep(nmSec * 1000 );
		    pThreadManage->RemoveJobInfo(*this);
      //  printf("The Job comes from CXJOB,Name: %s Exit\n", GetJobName());
    }
};
 
class CYJob:public CJob
{
public:
    CYJob(char* szJobName, int jobID):CJob(szJobName, jobID){}
    ~CYJob(){}
    void Run(void* jobdata)    
    {
        CThreadManage* pThreadManage = (CThreadManage*)jobdata;
        pThreadManage->AddJobInfo(*this);
        srand((unsigned)time(0));
        int nmSec = (rand() % 20) * 1000;
        printf("The Job comes from CYJOB,%s delay %d ms\n",  GetJobName(), nmSec);
        usleep(nmSec * 1000 );
		    pThreadManage->RemoveJobInfo(*this);      
    }
};
 
main()
{

    unsigned int nTimeTestSec = 0;        /* sec */
    unsigned int nTimeTest = 0;        /* usec */
    unsigned int nDelay = 0;        /* usec */
    struct timeval tvBegin;
    struct timeval tvNow;    
    char szTemp[100];
    gettimeofday (&tvBegin, NULL);  
    
    char szJobName[20];

    CThreadManage* manage = new CThreadManage(100);
    for(int i=0; i<30; i++)
    {              
        sprintf(szJobName,"A_JOB_%d",(i+1));
        CXJob*   job = new CXJob(szJobName, (i+1));
        manage->Run(job, manage);
        usleep(1 * 1000 * 1000);
    }    
	
   // usleep(2 * 1000 * 1000);
    
    strcpy(szJobName, "B_JOB_100");    
    CYJob* job = new CYJob(szJobName, 100);
    manage->Run(job,  manage);
    //  usleep(1 * 1000 * 1000);
    int i = 1;
    struct timeval timeout;
    for (i= 0; i < 6; i++)
    {
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
		    printf("The Job  ID %d  thrID is %d\n", ((i+1)*2), manage->GetJobThreadID(30 - (i+1)*2));
        select(1, NULL, NULL, NULL, & timeout );
        printf("waitfor %d minutes=====================\n", (i+1) );
    }
  
    manage->TerminateAll();


    gettimeofday (&tvNow, NULL);
    nTimeTest = (tvNow.tv_sec - tvBegin.tv_sec) * 1000000 + \
                 tvNow.tv_usec - tvBegin.tv_usec;
    sprintf(szTemp, "nTimeTest: %d\n", nTimeTest);
    cout << szTemp;
}

