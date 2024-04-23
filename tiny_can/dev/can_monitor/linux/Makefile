#
# Tiny-CAN-Monitor "can_monitor" Makefile
#

CC = gcc
LD = gcc

OBJ_DIR = obj
OUTPUT_DIR = .
PROGRAMM = can_monitor

SRC_DIR = ./../src
C_PREPROC = -DENABLE_NLS

CFLAGS = `pkg-config --cflags gtk+-2.0` -Wall \
         -I./../../lib -I./../libcanmonitor/src -I. -pthread -g0 -O2 #-ggdb3 -O0 

LIBS = `pkg-config --libs gtk+-2.0` \
        -ldl -lgmodule-2.0 -lcanmonitor
LDFLAGS = -L. -pthread


SRC_OBJS = \
  $(OBJ_DIR)/api_find.o \
  $(OBJ_DIR)/can_speed_db.o \
  $(OBJ_DIR)/can_speed_win.o \
  $(OBJ_DIR)/can_monitor_obj.o \
  $(OBJ_DIR)/cmdline.o \
  $(OBJ_DIR)/const.o \
  $(OBJ_DIR)/drv_scan.o \
  $(OBJ_DIR)/drv_win.o \
  $(OBJ_DIR)/filter.o \
  $(OBJ_DIR)/filter_win.o \
  $(OBJ_DIR)/main.o \
  $(OBJ_DIR)/main_can.o	\
  $(OBJ_DIR)/makro.o \
  $(OBJ_DIR)/makro_win.o \
  $(OBJ_DIR)/paths.o \
  $(OBJ_DIR)/plugin.o \
  $(OBJ_DIR)/plugin_call.o \
  $(OBJ_DIR)/plugin_win.o \
  $(OBJ_DIR)/setup.o \
  $(OBJ_DIR)/start_win.o \
  $(OBJ_DIR)/linux_port_setup.o \
  $(OBJ_DIR)/can_drv_linux.o \

all: create_dirs create_app

create_dirs:
	mkdir -p "$(OBJ_DIR)"
	#mkdir -p "$(OUTPUT_DIR)"

create_app: $(SRC_OBJS)
	$(LD) -o $(OUTPUT_DIR)/$(PROGRAMM) $(CFLAGS) $(SRC_OBJS) $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/api_find.o: $(SRC_DIR)/api_find.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/can_speed_db.o: $(SRC_DIR)/can_speed_db.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/can_speed_win.o: $(SRC_DIR)/can_speed_win.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/can_monitor_obj.o: $(SRC_DIR)/can_monitor_obj.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/cmdline.o: $(SRC_DIR)/cmdline.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/const.o: $(SRC_DIR)/const.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/drv_scan.o: $(SRC_DIR)/drv_scan.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/drv_win.o: $(SRC_DIR)/drv_win.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/filter.o: $(SRC_DIR)/filter.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/filter_win.o: $(SRC_DIR)/filter_win.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/main_can.o: $(SRC_DIR)/main_can.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/makro.o: $(SRC_DIR)/makro.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/makro_win.o: $(SRC_DIR)/makro_win.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/paths.o: $(SRC_DIR)/paths.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/plugin.o: $(SRC_DIR)/plugin.c 
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/plugin_call.o: $(SRC_DIR)/plugin_call.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/plugin_win.o: $(SRC_DIR)/plugin_win.c  
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@		

$(OBJ_DIR)/setup.o: $(SRC_DIR)/setup.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/start_win.o: $(SRC_DIR)/start_win.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/linux_port_setup.o: $(SRC_DIR)/linux_port_setup.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/can_drv_linux.o: ./../../lib/can_drv_linux.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:	
	rm -f *.o 
	rm -f *~ 
	rm -f ./$(OBJ_DIR)/*.o
	rm -f ./$(OBJ_DIR)/*~

