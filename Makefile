CC = gcc

# Platform independend compiler optimization
CFLAGS = -O3 -c -Wall

# Intel Pentium M (Centrino) and gcc 4.1.1
#CFLAGS = -O3 -march=i686 -minline-all-stringops -funroll-loops -c -Wall

# AMD Opteron and gcc 3.3.5
#CFLAGS = -O3 -mcpu=i686 -minline-all-stringops -c -Wall

# TODO: clang stuff vs real gcc (osx)

# Linux, BSD
#LFLAGS = -s # generates depreciated warning on OSX

# Solaris 9
#LFLAGS = -s -lrt

OBJ =	main.o \
	instr_single.o \
	instr_cb.o \
	instr_dd.o \
	instr_ed.o \
	instr_fd.o \
	instr_ddcb.o \
	instr_fdcb.o \
	cli.o \
	disas.o	\
	interrupt.o \
	io.o	\
	util.o \
	global.o

z80sim : $(OBJ)
	$(CC) $(OBJ) $(LFLAGS) -o z80sim

main.o : main.c	config.h global.h
	$(CC) $(CFLAGS) main.c

instr_single.o : instr_single.c	config.h global.h
	$(CC) $(CFLAGS) instr_single.c

instr_cb.o : instr_cb.c	config.h global.h
	$(CC) $(CFLAGS) instr_cb.c

instr_dd.o : instr_dd.c	config.h global.h
	$(CC) $(CFLAGS) instr_dd.c

instr_ed.o : instr_ed.c	config.h global.h
	$(CC) $(CFLAGS) instr_ed.c

instr_fd.o : instr_fd.c	config.h global.h
	$(CC) $(CFLAGS) instr_fd.c

instr_ddcb.o : instr_ddcb.c	config.h global.h
	$(CC) $(CFLAGS) instr_ddcb.c

instr_fdcb.o : instr_fdcb.c	config.h global.h
	$(CC) $(CFLAGS) instr_fdcb.c

cli.o : cli.c config.h global.h
	$(CC) $(CFLAGS) cli.c

disas.o	: disas.c
	$(CC) $(CFLAGS) disas.c

interrupt.o : interrupt.c config.h global.h
	$(CC) $(CFLAGS) interrupt.c

io.o	: io.c config.h	global.h
	$(CC) $(CFLAGS) io.c

util.o : util.c config.h
	$(CC) $(CFLAGS) util.c

global.o : global.c config.h
	$(CC) $(CFLAGS) global.c

clean:
	rm -f *.o core z80sim
