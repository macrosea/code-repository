
CFLAGS=-O9 -Wall -g 
DEFINE = -D _TEST
XX = g++ 

exampleTP:  thread.o sync.o workerThread.o threadPool.o job.o threadManage.o example.o 
	$(XX) thread.o sync.o workerThread.o threadPool.o job.o threadManage.o  example.o -o exampleTP -lpthread

example.o : ./example/example.cpp
	$(XX) -c ./example/example.cpp -o example.o

thread.o:  ./ThreadPool/thread.cpp ./ThreadPool/thread.h
	g++ -c  -lpthread  ./ThreadPool/thread.cpp -o thread.o -lstdc++	
	#$(XX) -c ./ThreadPool/thread.cpp -o thread.o
	#$(XX) $(CFLAGS) $(DEFINE) -lpthread  ./ThreadPool/thread.cpp -o thread.o -lstdc++

sync.o:./ThreadPool/sync.cpp ./ThreadPool/sync.h
	$(XX) -c ./ThreadPool/sync.cpp -o sync.o

workerThread.o:./ThreadPool/workerThread.cpp ./ThreadPool/workerThread.h
	$(XX) -c ./ThreadPool/workerThread.cpp -o workerThread.o	
	
threadManage.o:./ThreadPool/threadManage.cpp ./ThreadPool/threadManage.h
	$(XX) -c ./ThreadPool/threadManage.cpp -o threadManage.o 

threadPool.o:./ThreadPool/threadPool.cpp ./ThreadPool/threadPool.h
	$(XX) -c ./ThreadPool/threadPool.cpp -o threadPool.o

job.o:./ThreadPool/job.cpp ./ThreadPool/job.h
	$(XX) -c ./ThreadPool/job.cpp -o job.o



clean:
	rm -rf *.o exampleTP

#    gcc -o thread.o -g -lpthread sync.cpp -lstdc++

##  v2

##		CC = gcc
##		XX = g++
##		CFLAGS = -Wall -O ¨Cg
##		TARGET = ./out/testThr
##					%.o: %.c
##		$(CC) $(CFLAGS) -c $< -o $@
##			%.o:%.cpp
##		$(XX) $(CFLAGS) -c $< -o $@
##	
##		SOURCES = $(wildcard *.c *.cpp)
##		OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
##	
##		$(TARGET) : $(OBJS)
##		$(XX) $(OBJS) -o $(TARGET)
##		chmod a+x $(TARGET)
##		clean:
##			rm -rf *.o helloworld
