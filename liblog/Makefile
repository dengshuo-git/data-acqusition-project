#export ARCH := mips
#export CROSS_COMPILE :=mips64-octeon-linux-gnu-
CC=gcc
LDFLAGS+=-L./
CFLAGS += -I../include
liblog.so:logfns.o
	$(CROSS_COMPILE)$(CC) -shared logfns.o -lc -lrt -g -o liblog.so $(CFLAGS)
	cp liblog.so ../lib	
	cp liblog.so /lib	
	cp logfns.h ../include
logfns.o:logfns.c
	$(CROSS_COMPILE)$(CC)  -fPIC -w -g -c logfns.c -lrt -o logfns.o -std=c99 $(CFLAGS)
all:liblog.so
clean:
	rm -f *.o *.so *.c~ *.h~ *~
