#export ARCH := mips
#export CROSS_COMPILE :=mips64-octeon-linux-gnu-

CC=gcc
LDFLAGS+=-L./
CFLAGS+=-I../../include
TFTPDIR ?= /lib
CFLAGS += $(DEBUG_DEF)
libdaq.so:libdaq.o libdaq.h 
	$(CROSS_COMPILE)$(CC) -shared libdaq.o -lc -lrt -g -o libdaq.so $(CFLAGS)
	cp libdaq.so $(TFTPDIR)
	cp libdaq.so ../../lib 
	cp libdaq.h ../../include
libdaq.o:libdaq.c 
	$(CROSS_COMPILE)$(CC)  -fPIC -w -g -c libdaq.c -lrt -o libdaq.o -std=c99 $(CFLAGS)
all:libdaq.so
clean:
	rm -f *.o *.so *.c~ *.h~ *~
