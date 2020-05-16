#export ARCH := mips
#export CROSS_COMPILE :=mips64-octeon-linux-gnu-

TARGET = configuration_management 
CC	= gcc
#OBJS += configuration_management.o minIni.o 

CCFLAGS += -I../include
LDFLAGS += -L../lib 

all:$(TARGET)
$(TARGET):configuration_management.c minIni.c
	$(CROSS_COMPILE)$(CC) configuration_management.c minIni.c -g -lsocket -ldaq -o  $(TARGET) $(CCFLAGS) $(LDFLAGS) 
	cp $(TARGET) ../bin
	
clean:
	rm -rf *.o  $(TARGET)









