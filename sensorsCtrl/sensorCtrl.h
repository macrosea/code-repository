/*
 *  Created on Oct. 5, 2015
 *  version:    0.1
 *  author:     Xi Ouyang <xouyang@cisco.com>
*/
#ifndef __SENSOR_CTRL__
#define __SENSOR_CTRL__

#include "client.h"
#include "sensor2.h"
//class client;
using namespace std;

class sensorCtrl : public client
{
    public:
        explicit sensorCtrl(int tmo):
                    client(tmo),
                    s_ht(NULL),
                    count(0)
                    {};
        ~sensorCtrl() {
            if (s_ht != NULL)
                delete s_ht;
        };

        bool init(const char* ip, const int port);

        bool RegisterDev(SHumTemp* sensor);
        void onTimeout();
        void onClose() {cout << "onClose\n"; };
        void onRecv(void* buf, const int size) { cout << "onRecv\n"; };

        string get_time();

    private:
        SHumTemp* s_ht;
        int  count;
};


#endif
