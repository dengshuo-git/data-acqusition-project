export ARCH := mips
export CROSS_COMPILE :=
CC=gcc
LDFLAGS+= -L../../lib
CCFLAGS+= -I../../include
TARGET_OBJ = driver_test.o 
symm_function_test:$(TARGET_OBJ)	
	$(CROSS_COMPILE)$(CC) $(TARGET_OBJ) -g  -lpthread -ldaq -o driver_function_test $(CCFLAGS) $(LDFLAGS)
%.o:%.c 
	$(CROSS_COMPILE)$(CC) -c -g $<  -o $@ $(CCFLAGS) $(LDFLAGS) 
all:driver_function_test
clean:
	rm -f *.o driver_function_test *~

