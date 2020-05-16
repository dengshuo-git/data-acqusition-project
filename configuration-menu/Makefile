#export ARCH := mips
#export CROSS_COMPILE :=mips64-octeon-linux-gnu-

TARGET = config_menu
CC	= g++

CCFLAGS += -I../include

LDFLAGS += -L../lib 

all:$(TARGET)
$(TARGET):menu.c
	$(CROSS_COMPILE)$(CC) menu.c -lsocket -g -o config_menu $(CCFLAGS) $(LDFLAGS)
	cp $(TARGET) ../bin
	
clean:
	rm -rf *.o  $(TARGET)









