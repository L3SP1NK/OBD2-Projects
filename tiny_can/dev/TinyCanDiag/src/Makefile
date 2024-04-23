PROGRAMM = ./linux/octopus-gui
CFLAGS = `pkg-config --cflags libgnomeui-2.0` \
         -Wall -I.  -g0 -O2 # -ggdb3 -O0 

         
LIBS = `pkg-config --libs libgnomeui-2.0` -lusb
LDFLAGS =


SRCS = main.c gui.c dialogs.c paths.c configfile.c file_sel.c gtk_util.c \
 ain_op.c std_op.c setup.c abaut_dlg.c giwled.c octopus.c

OBJS = $(SRCS:.c=.o)


all: $(OBJS)
	gcc -o $(PROGRAMM) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LIBS)

%.o: %.c
	gcc $(CFLAGS) -c $<

clean:	
	rm -f *.o *~
