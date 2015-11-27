/*
 *  Created on Oct. 5, 2015
 *  version:    0.1
 *  author:     Xi Ouyang <xouyang@cisco.com>
*/

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <iostream>

using namespace std;



class client
{
    public:
        explicit client(const int tmo_):
                       tmo(tmo_),
                       fd(-1) 
                       {};
        ~client() { disconnect(); };
        bool cnct_to(const char* ip, const int port, const int tmo=5);   
        void disconnect(); 

        int setnonblock();
        int handle();
        int report(char* buf, int size);

    protected:
        virtual void onClose(){cout << "closed \n"; };
        virtual void onRecv(void* buf, const int size) {cout << "received\n";};    
        virtual void onTimeout() {cout << "timeout\n";};    

    private:
        int tmo;
        int fd;
};


#endif
