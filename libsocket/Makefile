#export ARCH := mips
#export CROSS_COMPILE :=mips64-octeon-linux-gnu-
CC=gcc
LDFLAGS+=-L./
CFLAGS+=-I./ -I../include
libsocket.so:socket.o socketfns.h 
	$(CROSS_COMPILE)$(CC) -shared socket.o -lc -lrt -g -o libsocket.so $(CFLAGS)
	cp libsocket.so ../lib	
	cp socketfns.h ../include
	cp libsocket.so /lib
socket.o:socketfns.c
	$(CROSS_COMPILE)$(CC)  -fPIC -w -g -c socketfns.c -lrt -o socket.o -std=c99 $(CFLAGS)
all:libsocket.so
clean:
	rm -f *.o *.so *.c~ *.h~ *~
