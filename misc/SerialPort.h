#ifndef _SERIAL_PORT_H_
#define _SERIAL_PORT_H_

/*
*  refer to: http://www.tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
*/

extern "C" {
#include     <stdio.h> 
#include     <stdlib.h> 
#include     <unistd.h>   
#include     <sys/types.h> 
#include     <sys/stat.h> 
#include     <fcntl.h>   
#include     <termios.h> 
#include     <errno.h>  
#include     <pthread.h>
}
#include<unistd.h>
#include "sys/epoll.h"

#include <cstring>
#include <string>

using namespace std;

#define TRACE() printf("TRACE===> %s : %d\n", __FUNCTION__, __LINE__)

typedef void(*cb_onRecv)(char *, int);

class CSerialPort
{
public:
    CSerialPort();
	~CSerialPort();

    enum {MAXLEN=1024};

    int InitPort(const string& dev, int baud=9600, int dbits=8, char parity='N', int stopbits=1);  

    bool StartMonitor();
    void StopMonitor() { quit = true; };


    void reg_cb(cb_onRecv func) { onRecv = func;};
    bool ThreadAlive() const { return threadAlive; };
	bool isOpen() const {return (portfd != -1);};

	void ClosePort();
	bool flush();
    bool set_speed(int baudrate);
    bool set_ctrl(int databits, int stopbits, int parity);
    bool set_mode(const char mode = 'b', int tmout = 15, int vmin=0);/*'b': block; 'n': nonblock; default: block*/
    bool set(int baud, int dbits, char parity, int stopbits);
	
	int read( char *& , int );
	
	int write( char *& , int );

private:
    static void * MonitorThread(void * arg);

private:
    bool threadAlive;
    bool quit;
	int portfd;
	int epfd;
    cb_onRecv onRecv;
    epoll_event events[6];//事件集合                                                                            
    char recvBuf[MAXLEN];//接受到的数据                                                                        
    pthread_t pid;//接受数据线程的Id                                                                            

};


#endif
