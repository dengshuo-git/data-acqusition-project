#export ARCH := mips
#export CROSS_COMPILE :=mips64-octeon-linux-gnu-

TARGET = tcp_client
CC	= g++

all:$(TARGET)
$(TARGET):tcp_client.c
	$(CROSS_COMPILE)$(CC) tcp_client.c  -lsocket -g  -o $(TARGET) -I../include -L../lib 
	
clean:
	rm -rf *.o  $(TARGET)









