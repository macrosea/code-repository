/*
 *  Created on Oct. 5, 2015
 *  version:    0.1
 *  author:     Xi Ouyang <xouyang@cisco.com>
*/
#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <wiringPi.h>  

struct ht
{		
  float hum;
  float temp;
};

class s_dht11
{
  public:
    explicit s_dht11(int pin){this->pin = pin;};
    ~s_dht11(){};
    bool init() { return(wiringPiSetup()!=-1); };
    bool query(ht& out);
  private:
    int pin;
};

#endif

