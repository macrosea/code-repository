
#include <stdio.h>  
#include <stdlib.h>  
#include <stdint.h>  

#include "sensor2.h"

int SHT232::Open()
{
    if( IsOpen() )
        return fd;
    /*
    O_RDONLY, O_WRONLY, O_RDWR 
    O_NOCTTY  // terminal mode
    O_NONBLOCK :unblock mode; O_NDELAY :block mode
    */
    fd = ::open(dev.c_str(), O_RDWR|O_NOCTTY|O_NONBLOCK);
    return fd;
}

bool SHT232::Config(int baud, int dbits, char parity, int stopbits)
{     
    if (IsOpen() == false) return false;

    struct termios newtio;     
    struct termios oldtio;     

    if(tcgetattr(fd,&oldtio) != 0)     
    {     
        perror("Setuport failed");     
        return false;     
    }     

    bzero(&newtio,sizeof(newtio));     
    newtio.c_cflag |= CLOCAL |CREAD;     
    newtio.c_cflag &= ~CSIZE;     

    /**************************/      
    switch(dbits)     
    {     
        case 7:     
            newtio.c_cflag |= CS7;     
            break;     
        case 8:     
            newtio.c_cflag |= CS8;     
            break;         
    }     
    /***************************/    
    switch(parity)     
    {     
        case 'O':     
            newtio.c_cflag |= PARENB;     
            newtio.c_cflag |= PARODD;     
            newtio.c_iflag |= (INPCK | ISTRIP);     
            break;     
        case 'E':     
            newtio.c_iflag |= (INPCK |ISTRIP);     
            newtio.c_cflag |= PARENB;     
            newtio.c_cflag &= ~PARODD;     
            break;     
        case 'N':     
            newtio.c_cflag &= ~PARENB;     
            break;     
    }     
    /***************************/     
    switch(baud)     
    {     
        case 2400:     
            cfsetispeed(&newtio,B2400);     
            cfsetospeed(&newtio,B2400);     
            break;     
        case 4800:     
            cfsetispeed(&newtio,B4800);     
            cfsetospeed(&newtio,B4800);     
            break;     
        case 9600:     
            cfsetispeed(&newtio,B9600);     
            cfsetospeed(&newtio,B9600);     
            break;   
        case 57600:     
            cfsetispeed(&newtio,B57600);     
            cfsetospeed(&newtio,B57600);     
            break;     
        case 115200:     
            cfsetispeed(&newtio,B115200);     
            cfsetospeed(&newtio,B115200);     
            break;     
      case 460800:     
            cfsetispeed(&newtio,B460800);     
            cfsetospeed(&newtio,B460800);     
            break;               
        default:     
            cfsetispeed(&newtio,B9600);     
            cfsetospeed(&newtio,B9600);     
            break;     
    }     
    /*************************/    
    if(stopbits == 1){     
        newtio.c_cflag &= ~CSTOPB;     
    }     
    else if(stopbits == 2){     
        newtio.c_cflag |= CSTOPB;     
    }     
    newtio.c_cc[VTIME] = 0; //1;     
    newtio.c_cc[VMIN] = 0;  //FRAME_MAXSIZE;   //effective on block mode

    tcflush(fd,TCIFLUSH);     
    if((tcsetattr(fd,TCSANOW,&newtio)) != 0)     
    {     
        perror("com set error");     
        return false;     
    }     
    return true;     
}     



bool SHT232::Init()
{
    if (Open() == -1) 
        return false;

    if (false == Config(9600, 8, 'N', 1))
    {
        Close();
        return false;
    }
    return true;
}

HT SHT232::Calc(char* data)
{
    float t = static_cast<float>((data[2]*256+data[3])*0.01 - 40);
    const double C1 = -4;
    const double C2 = 0.0405;
    const double C3 = -0.0000028;
    const double T1 = 0.01;
    const double T2 = 0.00008;

    double rh = static_cast<int>(data[4]) * 256 + data[5];
    double rh_lin = C3 * rh * rh + C2*rh + C1;
    float h =  static_cast<float>((t - 25)*(T1+T2*rh)+rh_lin);
    HT v;
    v.hum = h;
    v.temp = t;
    return v;
}

bool SHT232::Query(HT& out)
{
    char cmd[] = {0x01, 0x80, 0x33, 0xFE};
    int ret = ::write(fd, cmd, 4);
    if (ret == -1)
    {
        perror("failed to send command\n");
        return false;
    }

    const int SIZE = 32;
    char buf[SIZE] = {0};
    ret = ::read(fd, buf, SIZE);
    if ((ret == 6) 
       && (buf[0] == 0x80)
       && (buf[1] == 0x4))
    {
        out = Calc(buf);
        return true;
    }
    for (int i = 0; i<ret; i++)
        printf(" 0x%2x", buf[i]);
    printf("\nCalc return false\n");
    return false;
}
////////////////////////////////////////
bool SDHT11::Query(HT& out)
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
      //printf("%s: %d.%d\t%d.%d\n", __FUNCTION__, dht11_val[0],dht11_val[1],dht11_val[2],dht11_val[3]); 
      //
      if (dht11_val[0] + dht11_val[1] + dht11_val[2] + dht11_val[3] + dht11_val[4] == 0)
          return false;

      out.hum = dht11_val[0] + dht11_val[1]/10;
      out.temp = dht11_val[2] + dht11_val[3]/10;
      return true;
    }
    else
    {
      return false;
    }
}


#if 0

int main(void)  
{ 
  int pin = 7;
  SHumTemp* sen =  new SDHT11(pin);
  //SHumTemp* sen =  new SHT232("/dev/ttyUSB0");
  if (false == sen->Init())
  {
    printf("failed to Init sensor\n");
    return -1;
  }

  HT v;
  bool res = false;
  int retry = 0;
  for(retry  =0;  retry < 3; retry++)
  {
    res = sen->Query(v);
    if(res)
      break;

    delay(3000);
  }

  if(res)
    printf("hum: %.1f, temp: %.1f\n", v.hum, v.temp);
  else
    printf("no output\n");   

  delete sen;
  return 0;
  
} 
#endif

