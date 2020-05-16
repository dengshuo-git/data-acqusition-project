#export ARCH := mips
#export CROSS_COMPILE :=mips64-octeon-linux-gnu-

TARGET = log 
CC	= gcc
OBJS += log.o 

CCFLAGS += -I../include
LDFLAGS += -L../lib 

all:$(TARGET)
$(TARGET):log.c
	$(CROSS_COMPILE)$(CC) log.c -g -lsocket -o log $(CCFLAGS) $(LDFLAGS) 
	cp $(TARGET) ../bin
	cp ./log.h ../include
	
clean:
	rm -rf *.o  $(TARGET)









