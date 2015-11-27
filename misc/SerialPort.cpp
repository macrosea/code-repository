
#include "SerialPort.h"
#include <map>
#include <algorithm>
 
CSerialPort::CSerialPort(): 
                    threadAlive(false),
                    quit(false),
                    portfd(-1),
                    epfd(-1),
                    onRecv(NULL)
{

}

CSerialPort::~CSerialPort()
{
    if ( isOpen() )
        ClosePort();
}

int CSerialPort::InitPort(const string& dev, int baud, int dbits, char parity, int stopbits)
{
    if( isOpen() )
        return portfd;
    /*
    O_RDONLY, O_WRONLY, O_RDWR 
    O_NOCTTY  // terminal mode
    O_NONBLOCK :unblock mode; O_NDELAY :block mode
    */
    portfd=::open(dev.c_str(), O_RDWR|O_NOCTTY|O_NONBLOCK);
    if (portfd == -1)
    {
        ::perror("open failed");
        return portfd;
    }
    //flush();
    if (false == set(baud, dbits, parity, stopbits))
        ClosePort();

    return portfd;
}

bool CSerialPort::StartMonitor()
{
    epfd = epoll_create(6);
    if (-1 == epfd)
        return false;
    
    pthread_attr_t attr;                                                                                            
    pthread_attr_init(&attr);                                                                                       
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);                                                   
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);                                             
    return (0 == pthread_create(&pid, &attr, MonitorThread, (void *) this));  
}

void * CSerialPort::MonitorThread(void * pArg)
{
    CSerialPort *port = (CSerialPort*) pArg;

    TRACE();

    if (port->isOpen() == false)
    {
        printf("%s  com not open", __FUNCTION__);
        return NULL;
    }

    epoll_event ev;                                                                                          
    ev.data.fd = port->portfd;
    ev.events = EPOLLET | EPOLLIN;
    if (epoll_ctl(port->epfd, EPOLL_CTL_ADD, port->portfd, &ev) != 0) {
        return NULL;
    }

    /*
    

     * */
    port->threadAlive = true;
    //下面开始epoll等待
    int i =0,witeNum= 0;
    while (!port->quit) 
    {
        TRACE();
        witeNum = epoll_wait(port->epfd, port->events, 6, 2000);

        for (i = 0; i < witeNum; i++) {
            if ((port->events[i].events & EPOLLERR)
                    || (port->events[i].events & EPOLLHUP)
                    || (!port->events[i].events & EPOLLIN)) {
                break;
            } else if (port->events[i].events & EPOLLIN) {//有数据进入
                int size = ::read( port->portfd, port->recvBuf, MAXLEN); 
                printf("recv data, size= %d\n", size);
                if (size > 0 && port->onRecv != NULL)
                    port->onRecv(port->recvBuf, size);
            }
        }
    }
    printf("recv thread exit!!!");
    port->threadAlive = false;
    return NULL;
}


void CSerialPort::ClosePort()
{
    TRACE();
    if (portfd != -1) 
    {
        ::close( portfd );
        portfd = -1;
    }

    StopMonitor();

    if (epfd != -1)
    {
        ::close(epfd);
        epfd = -1;
    }
}

bool CSerialPort::flush()
{
    if ( ::tcflush( portfd , TCIFLUSH ) == -1 )
    {
        ::perror( "tcflush failed" );
        return false;
    }
    return true;
}

bool CSerialPort::set_speed( int baudrate )
{
    if (isOpen() == false) return false;

    const map<int, int>::value_type init_value[] =
                                        {
                                            map<int, int>::value_type(115200, B115200),
                                            map<int, int>::value_type(57600, B57600),
                                            map<int, int>::value_type(38400, B38400),
                                            map<int, int>::value_type(19200, B19200),
                                            map<int, int>::value_type(4800,  B4800),
                                            map<int, int>::value_type(2400,  B2400),
                                            map<int, int>::value_type(1200,  B1200),
                                            map<int, int>::value_type(300,   B300)
                                        };
    map<int, int> map_speed(init_value, init_value + 8);

    std::map<int, int>::iterator it;
    it = map_speed.find(baudrate);
    int speed = 0;
    if (it != map_speed.end())
        return false;
    else
        speed = it->second;

    struct termios   opt;
    tcgetattr(portfd, &opt);
    tcflush(portfd, TCIOFLUSH);
    cfsetispeed(&opt, speed);
    cfsetospeed(&opt, speed);
    int status = tcsetattr(portfd, TCSANOW, &opt);
    return (status == 0);
}

bool CSerialPort::set_ctrl(int databits, int stopbits, int parity)
{
    if (isOpen() == false) return false;

    struct termios options;
    int result = tcgetattr(portfd,&options);
    if(result != 0)
    {
        return false;
    }

    /*8N1*/
    options.c_cflag &= ~CSIZE; /* Mask the character size bits */
    switch (databits)
    {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            throw 11;
            break;
    }

    switch (parity)
    {
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;   /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Enable parity checking */
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB);  /* Set odd checking*/
            options.c_iflag |= INPCK;             /* Disnable parity checking */
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;     /* Enable parity */
            options.c_cflag &= ~PARODD;   /* Set event checking*/ 
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S':
        case 's':  /*as no parity*/
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:
            throw 12;
            break;
    }

    //set stop bits
    switch (stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            throw 13;
            break;
    }

    /* Set input parity option */
    if (parity != 'n') {
        options.c_iflag |= INPCK;
    }

    //options.c_cc[VTIME] = 150; // 15 seconds
    //options.c_cc[VMIN] = 0;    // effective on block mode; block to read until 0 character arrives 

    options.c_cflag &= ~CRTSCTS;//disable hardware flow control;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);/*raw input*/
    options.c_oflag  &= ~OPOST;   /*raw output*/

    tcflush(portfd,TCIFLUSH); /* Update the options and do it NOW */
    result = tcsetattr(portfd,TCSANOW,&options);
    return (result == 0);
}

bool CSerialPort::set_mode(const char mode, int tmout, int vmin)
{
    if (isOpen() == false) return false;

    bool res = false;
    if (mode == 'n') // nonblock
        res = (fcntl(portfd, F_SETFL, FNDELAY) >= 0);
    else if (mode == 'b')// block
        res = (fcntl(portfd, F_SETFL, 0) >= 0);
    else
        throw 21;

    if (false == res) return false;

    struct termios options;
    res = tcgetattr(portfd,&options);
    if(res != 0)   return false;

    options.c_cflag &= ~CSIZE; /* Mask the character size bits */
    options.c_cc[VTIME] = tmout; // TIME *0.1 s
    options.c_cc[VMIN] = vmin;    // effective on block mode; block to read until 0 character arrives 
    tcflush(portfd,TCIFLUSH); /* Update the options and do it NOW */
    res = tcsetattr(portfd,TCSANOW,&options);
    return (res == 0);
}

int CSerialPort::read( char * &data, int maxsize)
{
    int size;
    if ( (size = ::read( portfd, data, maxsize)) ==-1)
        ::perror( "read failed" );
    return size;
}

int CSerialPort::write( char * &data , int maxsize )
{
    int size;
    if ((size = ::write( portfd , data , maxsize )) == -1)
        ::perror( "write failed" );
    return size;
}

bool CSerialPort::set(int baud, int dbits, char parity, int stopbits)     
{     
    if (isOpen() == false) return false;

    struct termios newtio;     
    struct termios oldtio;     

    if(tcgetattr(portfd,&oldtio) != 0)     
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

    tcflush(portfd,TCIFLUSH);     
    if((tcsetattr(portfd,TCSANOW,&newtio)) != 0)     
    {     
        perror("com set error");     
        return false;     
    }     
    return true;     
}     

#if 1
void on_recv(char* buf, int len)
{
    char out[20] = {0};
    buf[len] = 0;
    printf("===>[%2x]  %s !!\n",(char)(buf[0]), buf);
    for (int i= 0; i < len; i++)
        printf(" %.2x", (char)(buf[i]) );

    printf("\n");
    printf("%2x %2x %2x", (char)buf[0], (char)buf[1], (char)buf[2]);

    printf("\n");
}

int main()
{
   int cmd[] = {0x01, 0x80, 0x33, 0xFE};
   CSerialPort port;
   port.reg_cb(on_recv);
   port.InitPort("/dev/ttyUSB0", 9600);
   port.set_mode('n', 0, 6);
   port.set_ctrl(8, 1, 'n');
   port.StartMonitor();
   port.write(cmd, 4);
   sleep(12);
   
   return 0;
}
#endif

// EOF
