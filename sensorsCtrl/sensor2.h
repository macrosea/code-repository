

#ifndef _SENSOR_H_
#define _SENSOR_H_

extern "C" {
#include     <stdio.h>
#include     <stdlib.h>
#include     <unistd.h>
#include     <sys/types.h>
#include     <sys/stat.h>
#include     <fcntl.h>
#include     <termios.h>
#include     <errno.h>
}
//#include<unistd.h>

#include <cstring>
#include <string>

#include <wiringPi.h>  

using namespace std;

struct HT 
{		
  float hum;
  float temp;
};

class SHumTemp
{
    public:
        virtual bool Init() = 0; 
        virtual bool Query(HT& out) = 0; 
        virtual ~SHumTemp() {};
};

class SHT232 :public SHumTemp
{
    public: 
        explicit SHT232(const char* dev_):dev(string(dev_)),fd(-1) {};
        ~SHT232(){
            Close();        
        };
        bool Init();
        bool Query(HT& out);

    private:
        int Open();
        void Close() { 
            if (fd != -1)
                close(fd);
            fd = -1;
        };
	    bool IsOpen() const {return (fd != -1);};
        bool Config(int baud, int dbits, char parity, int stopbits);
        HT Calc(char* data);

    private:
        string dev;
        int fd;
};

class SDHT11 : public SHumTemp
{
  public:
    explicit SDHT11(int pin_):pin(pin_){};
    ~SDHT11(){};
    bool Init() { return(wiringPiSetup()!=-1); };
    bool Query(HT& out);
  private:
    int pin;
};

#endif

