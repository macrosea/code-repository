
#include "reactor.h"

/* 
*EventLoop成员实现 
*/  
__thread EventLoop* t_loopInThisThread=0;//线程私有数据表示线程是否拥有EventLoop  
const int kPollTimeMs=10000;//poll等待时间  
EventLoop::EventLoop():looping_(false),quit_(false),threadID_(syscall(SYS_gettid)),poller_(new Poller(this)){  
    if(!t_loopInThisThread){  
        t_loopInThisThread=this;//EventLoop构造时线程私有数据记录  
    }  
}  
EventLoop::~EventLoop(){  
    assert(!looping_);  
    t_loopInThisThread=NULL;//EventLoop析构将其置空  
}  
void EventLoop::loop(){//EventLoop主循环，主要功能是监听事件集合，执行就绪事件的处理函数  
    assert(!looping_);  
    assertInLoopThread();  
    looping_=true;  
    quit_=false;  
    while(!quit_){  
        activeChannels_.clear();  
        poller_->Poll(kPollTimeMs,&activeChannels_);//activeChannels是就绪事件  
        for(ChannelList::iterator it=activeChannels_.begin();it!=activeChannels_.end();it++){  
            (*it)->handleEvent();//处理就绪事件的回调函数  
        }  
    }  
    looping_=false;  
}  
void EventLoop::quit(){  
    quit_=true;//停止主循环标志，主循环不会马上停止有延迟  
}  
void EventLoop::updateChannel(Channel* channel){//主要用于文件描述符添加到poll的监听事件集合中  
    assert(channel->ownerLoop()==this);  
    assertInLoopThread();  
    poller_->updateChannel(channel);  
}  
void EventLoop::abortNotInLoopThread(){  
    printf("abort not in Loop Thread\n");  
    abort();//非本线程调用强行终止  
}  
/* 
*Poller成员实现 
*/  
Poller::Poller(EventLoop* loop):ownerLoop_(loop){}//Poller明确所属的EventLoop  
Poller::~Poller(){}  
void Poller::Poll(int timeoutMs,ChannelList* activeChannels){  
    int numEvents=poll(&*pollfds_.begin(),pollfds_.size(),timeoutMs);//poll监听事件集合pollfds_  
    if(numEvents>0){  
        fillActiveChannels(numEvents,activeChannels);//将就绪的事件添加到activeChannels  
    }  
    else if(numEvents==0){  
    }  
    else{  
        printf("Poller::Poll error\n");  
    }  
}  
void Poller::fillActiveChannels(int numEvents,ChannelList* activeChannels) const{//将就绪事件通过activeChannels返回  
    for(PollFdList::const_iterator pfd=pollfds_.begin();pfd!=pollfds_.end()&&numEvents>0;++pfd){  
        if(pfd->revents>0){  
            --numEvents;//若numEvents个事件全部找到就不需要再遍历容器剩下的部分  
            ChannelMap::const_iterator ch=channels_.find(pfd->fd);  
            assert(ch!=channels_.end());  
            Channel* channel=ch->second;  
            assert(channel->fd()==pfd->fd);  
            channel->set_revents(pfd->revents);  
            activeChannels->push_back(channel);  
        }  
    }  
}  
void Poller::updateChannel(Channel* channel){  
    assertInLoopThread();  
    if(channel->index()<0){//若channel的文件描述符fd没有添加到poll的监听事件集合中  
        assert(channels_.find(channel->fd())==channels_.end());  
        struct pollfd pfd;  
        pfd.fd=channel->fd();  
        pfd.events=static_cast<short>(channel->events());  
        pfd.revents=0;  
        pollfds_.push_back(pfd);  
        int idx=static_cast<int>(pollfds_.size())-1;  
        channel->set_index(idx);  
        channels_[pfd.fd]=channel;  
    }  
    else{//若已经添加到监听事件集合中，但是需要修改  
        assert(channels_.find(channel->fd())!=channels_.end());  
        assert(channels_[channel->fd()]==channel);  
        int idx=channel->index();  
        assert(0<=idx&&idx<static_cast<int>(pollfds_.size()));  
        struct pollfd& pfd=pollfds_[idx];  
        assert(pfd.fd==channel->fd()||pfd.fd==-1);  
        pfd.events=static_cast<short>(channel->events());//修改注册事件类型  
        pfd.revents=0;  
        if(channel->isNoneEvent()){  
            pfd.fd=-1;//若无事件则poll忽略  
        }  
    }  
}  
/* 
*Channel成员实现 
*/  
const int Channel::kNoneEvent=0;//无事件  
const int Channel::kReadEvent=POLLIN|POLLPRI;//可读事件  
const int Channel::kWriteEvent=POLLOUT;//可写事件  
Channel::Channel(EventLoop* loop,int fdArg):loop_(loop),fd_(fdArg),events_(0),revents_(0),index_(-1){}  
void Channel::update(){//添加或修改文件描述符的事件类型  
    loop_->updateChannel(this);  
}  
void Channel::handleEvent(){//处理就绪事件的处理函数  
    if(revents_&POLLNVAL){  
        printf("Channel::handleEvent() POLLNVAL\n");  
    }  
    if(revents_&(POLLERR|POLLNVAL)){//出错回调  
        if(errorCallback)  
            errorCallback();  
    }  
    if(revents_&(POLLIN|POLLPRI|POLLRDHUP)){//可读回调  
        if(readCallback)  
            readCallback();  
    }  
    if(revents_&POLLOUT){//可写回调  
        if(writeCallback)  
            writeCallback();  
    }  
}  
/* 
*测试代码，主线程往管道写数据，子线程通过EventLoop监听管道读端然后执行相应的可读回调(读取数据并输出) 
*/  
int pipefd[2];  
EventLoop* loop;  
void ReadPipe(){  
    char buf[1024];  
    read(pipefd[0],buf,1024);  
    printf("%s\n",buf);  
    loop->quit();//执行完可读回调后终止EventLoop的事件循环loop  
}  
void* threadFun(void* arg){  
    loop=new EventLoop();  
    Channel channel(loop,pipefd[0]);  
    channel.setReadCallBack(ReadPipe);  
    channel.enableReading();  
    loop->loop();  
}  
int main(){  
    pthread_t pid;  
    pipe(pipefd);  
    pthread_create(&pid,NULL,&threadFun,NULL);  
    const char* ptr="Can you get this data?";  
    write(pipefd[1],ptr,strlen(ptr));  
    pthread_join(pid,NULL);  
    return 0;  
}  


///////
#if 0 
/*  try timerfd */
/
void* threadFun(void* arg){  
    loop=new EventLoop();  
    //Channel channel(loop,pipefd[0]);  
    int timerfd=timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);  
    struct itimerspec howlong;  
    bzero(&howlong,sizeof(howlong));  
    howlong.it_value.tv_sec=5;  
    timerfd_settime(timerfd,0,&howlong,NULL);  
    Channel channel(loop,timerfd);  
    channel.setReadCallBack(ReadPipe);  
    channel.enableReading();  
    loop->loop();  
}  
#endif
