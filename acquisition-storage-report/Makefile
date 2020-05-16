#export ARCH := mips
#export CROSS_COMPILE :=mips64-octeon-linux-gnu-

TARGET = daq_function 
CC	= gcc

CCFLAGS += -I../include
LDFLAGS += -L../lib 

all:$(TARGET)
$(TARGET):acqusition_storage_report.c minIni.c
	$(CROSS_COMPILE)$(CC) acqusition_storage_report.c minIni.c -g -lsocket -ldaq -lpthread -o  $(TARGET) $(CCFLAGS) $(LDFLAGS) 
	cp $(TARGET) ../bin
	
clean:
	rm -rf *.o  $(TARGET)









