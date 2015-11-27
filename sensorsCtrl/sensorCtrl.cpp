#include "string.h"
#include <sys/timeb.h>
#include <time.h>
#include "sensorCtrl.h"

bool sensorCtrl::init(const char* ip, const int port)
{
    if (false == cnct_to(ip, port))
        return false;
    setnonblock();
    return true;
}

bool sensorCtrl::RegisterDev(SHumTemp* sensor)
{
    s_ht = sensor;
    if(false == s_ht->Init())
    {
        s_ht = NULL;
        return false;
    }
    return true;
} 

string sensorCtrl::get_time()
{
    struct timeb now;
    struct tm   *time_now;
    char szTime[24] = {0};
    ftime(&now);
    time_now = localtime(&now.time);
    sprintf(szTime,"%.4d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d",
                    1900+ time_now->tm_year,
                    1 + time_now->tm_mon,
                    time_now->tm_mday,
                    time_now->tm_hour,
                    time_now->tm_min,
                    time_now->tm_sec,
                    now.millitm);
    return string(szTime);
}


void sensorCtrl::onTimeout()
{
    count++;
    cout << "click: " << count << endl;
    if (s_ht != NULL)
    {
        bool res = false;
        HT v;
        res = s_ht->Query(v);
        // if (static_cast<int>(v.hum) == 0 && static_cast<int>(v.temp) == 0)
        //    res = false;
        if (res) 
        {
            char szData[250] = {0};
            string t = get_time();
            sprintf(szData, "%s|humidity|%.1f|temperature|%.1f;",
                    t.c_str(),
                    v.hum, 
                    v.temp);
            int ret = report(szData, strlen(szData));
            printf("connected: %s\n", szData);
            count = 0;
        }
    }
    if (count >= 5)
    {
        char szData[250] = {0};
        string t = get_time();
        sprintf(szData, "%s|s|0;",
                t.c_str());
        printf("disconnect: %s\n", szData);
        int ret = report(szData, strlen(szData));
        count = 0;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("usage: client 127.0.0.1\n");
        return 0;
    }
    sensorCtrl sc(3000);
    bool ret = sc.init(argv[1], 8899);
    SHumTemp* sensor = new SDHT11(7);
    if (false == sc.RegisterDev(sensor))
    {
        printf("failed to init dev\n");
        delete sensor;
    }
    if (!ret)
    {
        cout << "failed to connect\n";
       return 0; 
    }
    sc.handle();
    cout << "exit!!!!!\n";
    return 0;
}
