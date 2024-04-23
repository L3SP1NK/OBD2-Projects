#
# libusbcan Makefile
#

CC = gcc
LD = gcc 
INCLUDE = -I. -I.. -I../../lib
LIB = -ldl -lc
CFLAGS = -Wall -pthread -ggdb3 -O0 
#CFLAGS = -Wall -pthread -g0 -O2
LDFLAGS =

SRCS = main.c ../util.c ../linux_util.c ../../lib/can_drv_linux.c
OBJS = $(SRCS:.c=.o)

all: $(OBJS)
	$(LD) -o ex_sample4 $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

clean:	
	rm -f *.o *~ ./../../lib/*.o ./../../lib/*~ \
	./../*.o ./../*~


