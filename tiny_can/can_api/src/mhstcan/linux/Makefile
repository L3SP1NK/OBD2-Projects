#
# mhstcan Makefile
#

#DEBUG	  = 1

CC = gcc
LD = gcc

INCLUDE = -I.. -I../../lib -I../../tcan-core -I../../LinuxLib

LIB = -lpthread -lrt -lc
CFLAGS = -m32 -fPIC -Wall -Wextra -pthread
LDFLAGS = -shared -Wl,--no-undefined -Wl,--soname,libmhstcan.so  

ifdef DEBUG
    CFLAGS   += -ggdb3 -O0
else
    CFLAGS   += -g0 -O3 
    LDFLAGS  += -flto -s
    
endif

SRCS = ../../LinuxLib/linux_mhstcan.c \
       ../../LinuxLib/usb_scan.c \
       ../../LinuxLib/usb_pnp.c \
       ../../LinuxLib/mhs_cmd.c \
       ../../LinuxLib/mhs_event.c \
       ../../LinuxLib/mhs_thread.c \
       ../../LinuxLib/mhs_file_event.c \
       ../../LinuxLib/ser_io.c \
       ../../LinuxLib/com_io.c \
       ../../LinuxLib/sysfs.c \
       ../drv_info.c \
       ../../lib/api_calls.c \
       ../../lib/api_cp.c \
       ../../lib/api_ex.c \
       ../../lib/api_fd.c \
       ../../lib/can_core.c \
       ../../lib/can_fifo.c \
       ../../lib/can_obj.c \
       ../../lib/can_puffer.c \
       ../../lib/errors.c \
       ../../lib/index.c \
       ../../lib/info.c \
       ../../lib/log.c \
       ../../lib/mem_debug.c \
       ../../lib/mhs_obj.c \
       ../../lib/paths.c \
       ../../lib/usb_hlp.c \
       ../../lib/user_mem.c \
       ../../lib/util.c \
       ../../lib/var_obj.c \
       ../../lib/mhs_user_event.c \
       ../../lib/ul_fifo.c \
       ../../tcan-core/sm_drv.c \
       ../../tcan-core/tar_drv.c \
       ../../tcan-core/tcan_com.c \
       ../../tcan-core/tcan_drv.c \
       ../../tcan-core/tar_drv2.c \
       ../../tcan-core/tcan_com2.c \
       ../../tcan-core/tcan_setup.c \
       ../../tcan-core/fd_speeds.c \
       ../../tcan-core/read_info.c \
       ../../tcan-core/can_test.c \
       ../../tcan-core/ecu_flash.c \
       ../../tcan-core/speeds_db.c \
       ../../lib/mhs_event_list.c

OBJS = $(SRCS:.c=.o)


all: $(OBJS) 
	$(LD) $(LDFLAGS) $(CFLAGS) -o libmhstcan.so $(OBJS) $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

clean:	
	rm -f *.o *~ ./../../lib/*.o ./../../lib/*~ ./../../tcan-core/*.o ./../../tcan-core/*~ \
	./../*.o ./../*~ ./../../LinuxLib/*.o ./../LinuxLib/../*~ ./*.o ./*~
