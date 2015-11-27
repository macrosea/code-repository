
#include <stdio.h>  
#include <stdlib.h>  
#include <stdint.h>  

#include "string.h"

#include <sys/timeb.h>
#include <time.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>

#include <arpa/inet.h> 
#include <netinet/ip.h> 


#include "client.h"

bool client::cnct_to(const char* ip, const int port, const int tmo)   
{
  struct sockaddr_in addr;
  struct timeval timeo = {5, 0};
  socklen_t len = sizeof(timeo);

  fd = socket(AF_INET, SOCK_STREAM, 0);
  timeo.tv_sec = tmo;
  setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_port = htons(port);
  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
          if (errno == EINPROGRESS) {
                  fprintf(stderr, "%s:timeout\n", __FUNCTION__);
                  disconnect();
                  return false; 
          }
          perror("connect");
          return true;
  }
  printf("connected\n");
  return true;
}

void client::disconnect()
{
    if (fd != -1 ) 
        ::close(fd); 
    fd = -1;
}

int client::report(char* buf, int size)
{
    return send(fd, buf, size, 0);
}   

int client::setnonblock() 
{
  int flags;

  flags = fcntl(fd, F_GETFL);
  if (flags < 0) return flags;
  flags |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) < 0) return -1; 
  return 0;
}
	
int client::handle()
{
    if (fd == -1)
    {
        cout << "socket closed!! \n";
        return -1;
    }

    fd_set rset;
    FD_ZERO(&rset);

    int result;
    int maxfd;
    struct timeval timeo = {5, 0};
    maxfd = fd;
    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};

    while (1)
    {
        FD_SET(fd, &rset);
        timeo.tv_sec = static_cast<int>(tmo/1000);
        timeo.tv_usec = static_cast<int>((tmo%1000)*1000);
        result = select(maxfd + 1, &rset, NULL, NULL, &timeo); 
        if (result == 0) {
            // printf("select() timed out!\n");
            onTimeout();
            continue;
        }
        else if (result < 0 && errno != EINTR) 
        {
            printf("Error in select(): %s\n", strerror(errno));
            return -2;
        }
        else if (result > 0) 
        {
            if (FD_ISSET(fd, &rset))
            {
                int ret = recv(fd, recvbuf, 1024, 0);
                if (ret == -1)
                {
                    perror("** read error\n");
                    return -3;
                }
                else if (ret  == 0)   
                {
                    printf("server close\n");
                    onClose();
                    break;
                }
                else if (ret > 0)
                {
                    onRecv(recvbuf, ret);
                }
                memset(recvbuf, 0, sizeof(recvbuf));
            }
        }
    }
    return 0;
}


//EOF

