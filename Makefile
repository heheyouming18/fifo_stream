CC = cc

ROOTPATH=..
INCLUDE = -I./fifo/inc -I$(ROOTPATH)/fifo/inc 
LIB=-lpthread

OBJ += $(patsubst %.c, %.o, $(wildcard *.c))
OBJ += $(patsubst %.c, %.o, $(wildcard $(ROOTPATH)/fifo/src/*.c))
# OBJ += $(patsubst %.c, %.o, $(wildcard fifo/port/*.c))

CFLAGS = -O0 -g3 -Wall
target = FifoLinuxDemo

all:$(OBJ)
	$(CC) out/*.o -o $(target) $(LIB)
	mv $(target) out
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE)
	mv $@ out
clean:
	rm -rf out/*