#
# J1939Display Makefile
#

CC = gcc
LD = gcc

TCAN_API_DIR = /opt/tiny_can/dev/lib

OBJ_DIR = obj
OUTPUT_DIR = bin
PROGRAMM = TinyCanDiag

SRC_DIR = ./../src
C_PREPROC = -DENABLE_NLS

CFLAGS = `pkg-config --cflags gtk+-2.0` -Wall \
         -I./../src -I$(TCAN_API_DIR) -pthread -g0 -O2 #-ggdb3 -O0 

LIBS = `pkg-config --libs gtk+-2.0` -ldl -lm
LDFLAGS = -pthread


SRC_OBJS = \
  $(OBJ_DIR)/abaut_dlg.o \
  $(OBJ_DIR)/configfile.o \
  $(OBJ_DIR)/dialogs.o \
  $(OBJ_DIR)/file_sel.o \
  $(OBJ_DIR)/giwled.o \
  $(OBJ_DIR)/gtk-ex-frame.o \
  $(OBJ_DIR)/gtk_util.o \
  $(OBJ_DIR)/gui.o \
  $(OBJ_DIR)/gui_help.o \
  $(OBJ_DIR)/hw_info.o \
  $(OBJ_DIR)/io.o \
  $(OBJ_DIR)/main.o \
  $(OBJ_DIR)/main_samples.o \
  $(OBJ_DIR)/main_welcome.o \
  $(OBJ_DIR)/open_ext.o \
  $(OBJ_DIR)/paths.o \
  $(OBJ_DIR)/project.o \
  $(OBJ_DIR)/recent_files.o \
  $(OBJ_DIR)/setup.o \
  $(OBJ_DIR)/std_op.o \
  $(OBJ_DIR)/tcan_m2.o \
  $(OBJ_DIR)/tcan_m3.o \
  $(OBJ_DIR)/tcan_mx_gui.o \
  $(OBJ_DIR)/mhs_can_drv.o

all: create_dirs create_app

create_dirs:
	mkdir -p "$(OBJ_DIR)"
	mkdir -p "$(OUTPUT_DIR)"

create_app: $(SRC_OBJS)
	$(LD) -o $(OUTPUT_DIR)/$(PROGRAMM) $(CFLAGS) $(SRC_OBJS) $(LDFLAGS) $(LIBS)
  
$(OBJ_DIR)/abaut_dlg.o: $(SRC_DIR)/abaut_dlg.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
   
$(OBJ_DIR)/configfile.o: $(SRC_DIR)/configfile.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/dialogs.o: $(SRC_DIR)/dialogs.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/file_sel.o: $(SRC_DIR)/file_sel.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/giwled.o: $(SRC_DIR)/giwled.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@

$(OBJ_DIR)/gtk-ex-frame.o: $(SRC_DIR)/gtk-ex-frame.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/gtk_util.o: $(SRC_DIR)/gtk_util.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/gui.o: $(SRC_DIR)/gui.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/gui_help.o: $(SRC_DIR)/gui_help.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/hw_info.o: $(SRC_DIR)/hw_info.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/io.o: $(SRC_DIR)/io.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/main_samples.o: $(SRC_DIR)/main_samples.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/main_welcome.o: $(SRC_DIR)/main_welcome.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/open_ext.o: $(SRC_DIR)/open_ext.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/paths.o: $(SRC_DIR)/paths.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/project.o: $(SRC_DIR)/project.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/recent_files.o: $(SRC_DIR)/recent_files.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/setup.o: $(SRC_DIR)/setup.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/std_op.o: $(SRC_DIR)/std_op.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/tcan_m2.o: $(SRC_DIR)/tcan_m2.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/tcan_m3.o: $(SRC_DIR)/tcan_m3.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/tcan_mx_gui.o: $(SRC_DIR)/tcan_mx_gui.c
	$(CC) $(CFLAGS) $(C_PREPROC) -c $< -o $@
  
$(OBJ_DIR)/mhs_can_drv.o: $(TCAN_API_DIR)/mhs_can_drv.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:	
	rm -f *.o 
	rm -f *~ 
	rm -f ./$(OBJ_DIR)/*.o
	rm -f ./$(OBJ_DIR)/*~

