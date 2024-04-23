#
# Tiny-CAN-Monitor "libcanmonitor" Makefile
#
CC = gcc
LD = gcc

OBJ_DIR = obj
SRC_DIR = ./../src
#C_PREPROC = -DENABLE_NLS

LIB = `pkg-config --libs gtk+-2.0` \
      -lpthread -lrt -lc -ldl 
CFLAGS = `pkg-config --cflags gtk+-2.0` -I./../../../lib \
          -Wall -pthread -g0 -O2 # -ggdb3 -O0
LDFLAGS = -L/usr/local/lib -Wall -shared -Wl,--soname,libcanmonitor.so -Wl,--export-dynamic \
          -pthread -nostartfiles # -nostdlib 
PICFLAGS = -fPIC
 

SRC_OBJS = \
  $(OBJ_DIR)/about_dlg.o \
  $(OBJ_DIR)/can_widget.o \
  $(OBJ_DIR)/cbuf.o \
  $(OBJ_DIR)/configfile.o \
  $(OBJ_DIR)/cview.o \
  $(OBJ_DIR)/datei_info.o \
  $(OBJ_DIR)/dialogs.o \
  $(OBJ_DIR)/drv_info.o \
  $(OBJ_DIR)/file_sel.o \
  $(OBJ_DIR)/gtk_util.o \
  $(OBJ_DIR)/hex_entry.o \
  $(OBJ_DIR)/mhs_event.o \
  $(OBJ_DIR)/mhs_obj.o \
  $(OBJ_DIR)/mhs_signal.o \
  $(OBJ_DIR)/moofontsel.o \
  $(OBJ_DIR)/splash.o \
  $(OBJ_DIR)/util.o

all: create_dirs create_lib

create_dirs:
	mkdir -p "$(OBJ_DIR)"

create_lib: $(SRC_OBJS)
	$(LD) $(LDFLAGS) $(LIB) -o libcanmonitor.so $(SRC_OBJS)
	cp libcanmonitor.so ./../../linux


$(OBJ_DIR)/about_dlg.o: $(SRC_DIR)/about_dlg.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/can_widget.o: $(SRC_DIR)/can_widget.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/cbuf.o: $(SRC_DIR)/cbuf.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/configfile.o: $(SRC_DIR)/configfile.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/cview.o: $(SRC_DIR)/cview.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/datei_info.o: $(SRC_DIR)/datei_info.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/dialogs.o: $(SRC_DIR)/dialogs.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/drv_info.o: $(SRC_DIR)/drv_info.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/file_sel.o: $(SRC_DIR)/file_sel.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/gtk_util.o: $(SRC_DIR)/gtk_util.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/hex_entry.o: $(SRC_DIR)/hex_entry.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/mhs_event.o: $(SRC_DIR)/mhs_event.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/mhs_obj.o: $(SRC_DIR)/mhs_obj.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/mhs_signal.o: $(SRC_DIR)/mhs_signal.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/moofontsel.o: $(SRC_DIR)/moofontsel.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/splash.o: $(SRC_DIR)/splash.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

$(OBJ_DIR)/util.o: $(SRC_DIR)/util.c 
	$(CC) $(CFLAGS) $(C_PREPROC) $(PICFLAGS) -c $< -o $@

clean:	
	rm -f *.o *~
	rm -f ./$(OBJ_DIR)/*.o
	rm -f ./$(OBJ_DIR)/*~


