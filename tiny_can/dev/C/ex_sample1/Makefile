#
# libusbcan Makefile
#

DEBUG    = 1

CC = gcc
LD = gcc 
INCLUDE = -I. -I.. -I../../lib
LIB = -ldl -lc
CFLAGS = -Wall -pthread 
LDFLAGS =

ifdef DEBUG
    CFLAGS   += -ggdb3 -O0 -DMHS_DRV_DEBUG_OUTPUT
else
    CFLAGS   += -g0 -O0 # -flto
endif


SRCS = main.c ../linux_util.c ../../lib/can_drv_linux.c
OBJS = $(SRCS:.c=.o)

all: $(OBJS)
	$(LD) -o ex_sample1 $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

clean:	
	rm -f *.o *~ ./../../lib/*.o ./../../lib/*~ \
	./../*.o ./../*~


