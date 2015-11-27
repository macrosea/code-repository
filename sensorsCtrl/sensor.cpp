
#include <stdio.h>  
#include <stdlib.h>  
#include <stdint.h>  

#include "sensor.h"

bool s_dht11::query(ht& out)
{
    const uint8_t MAX_TIME = 85;
    uint8_t lststate=HIGH;  
    int dht11_val[5]={0,0,0,0,0}; 
    
    pinMode(pin,OUTPUT);  
    digitalWrite(pin, LOW);  
    delay(18);  
    digitalWrite(pin, HIGH);  
    delayMicroseconds(40);  
    pinMode(pin,INPUT);     

    uint8_t i = 0, j = 0;

    for(i=0, j=0; i<MAX_TIME; i++)  
    {  
      uint8_t counter=0;  
      while(digitalRead(pin)==lststate)
      {  
        if((counter++) == 255)  
        {
            break;
        }

        delayMicroseconds(1);
      }  
      if(counter>255)
          break;
      //printf(">>>> %d\n", counter);

      lststate=digitalRead(pin); 
      // top 3 transistions are ignored  
      if((i>=4)&&(i%2==0))
      {  
        dht11_val[j/8]<<=1;  
        if(counter>16)  
          dht11_val[j/8]|=1;  
        j++;  
      }  
    }

    // verify cheksum and print the verified data  
    if((j>=40)
       &&(dht11_val[4]==((dht11_val[0]+dht11_val[1]+dht11_val[2]+dht11_val[3])& 0xFF)))  
    {
      printf("%s: %d.%d\t%d.%d\n", __FUNCTION__, dht11_val[0],dht11_val[1],dht11_val[2],dht11_val[3]); 
      out.hum = dht11_val[0] + dht11_val[1]/10;
      out.temp = dht11_val[2] + dht11_val[3]/10;
      return true;
    }
    else
    {
      return false;
    }

}

// EOF
