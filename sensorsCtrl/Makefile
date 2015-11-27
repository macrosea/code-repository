
BINPATH = .
BINNAME = sensorCtrl
TARGET =  $(BINPATH)/$(BINNAME)

SRCDIR = .

CC = g++

INCDIR = .

LFLAGS = -std=c++11 -lwiringPi

OBJS = client.cpp sensor2.cpp sensorCtrl.cpp 


$(TARGET) : $(OBJS)
		$(CC) -g -rdynamic $(OBJS) $(LFLAGS)  -o $(TARGET)

$(SRCDIR)/%.cpp : $(INCDIR)/%.h

%.o : $(SRCDIR)/%.cpp
		$(CC) -g $@ $<
				
				#rm -f *.o
clean:
		rm -f *.o $(TARGET)
