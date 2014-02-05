/*
 * Z80SIM  -  a	Z80-CPU	simulator
 *
 * Copyright (C) 1987-2008 by Udo Munk
 * 2014 fork by Jack Carrozzo <jack@crepinc.com>
 *
 * This modul of the simulator contains a simple terminal I/O
 * simulation as an example.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

#include "config.h"
#include "global.h"

static BYTE io_trap(BYTE);

static BYTE p_8255_in(BYTE);
static void p_8255_out(BYTE,BYTE);

static BYTE p_ctc_in(BYTE);
static void p_ctc_out(BYTE,BYTE);

static BYTE p_dart_in(BYTE);
static void p_dart_out(BYTE,BYTE);

static BYTE timer;   
static void int_timer(int);


// TODO: cleanup all the time stuff to fit paradigm
static BYTE time_out(BYTE data)
{
  static struct itimerval tim;
  static struct sigaction newact;

	printf("--- timer set (%d)\n",data);

  if (data>0) {
    timer = 1;
    newact.sa_handler = int_timer;
    sigaction(SIGALRM, &newact, NULL);
    tim.it_value.tv_sec = 0;
    tim.it_value.tv_usec = 10000;
    
		if (1==data) { 
			tim.it_interval.tv_sec = 0;
    	tim.it_interval.tv_usec = 10000;	// 10ms
		} else { // TODO: other values we might want here
			tim.it_interval.tv_sec = 1;				// 1s
      tim.it_interval.tv_usec = 0;
		}    

		tim.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL, &tim, NULL);
  } else { // 0: disable timer
    timer = 0;
    newact.sa_handler = SIG_IGN;
    sigaction(SIGALRM, &newact, NULL);
    tim.it_value.tv_sec = 0;
    tim.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &tim, NULL);
  }
  return((BYTE) 0);
}

/*
 *  I/O handler for read timer
 *  return current status of 10ms interrupt timer,
 *  1 = enabled, 0 = disabled
 */
static BYTE time_in(void)
{
	printf("!!! time_in()\n");
  return(timer);
}

/*
 *  timer interrupt causes maskerable CPU interrupt
 */
// this func is called each time the timer fires
static void int_timer(int sig)
{
	int_lsb = 0x50; // LSB for vector table (mode 2 only)
	int_type = INT_INT;
}

void init_io(void) { // called at start to init all ports
}

void exit_io(void) { // called at exit
}

// handles all IN opcodes
BYTE io_in(BYTE adr) {
	if (adr<4) 			 return p_8255_in(adr);
	else if (adr<8)  return p_ctc_in(adr);
	else if (adr<12) return p_dart_in(adr);
	else if (adr<16) return time_in();
	else 						 return io_trap(adr);
}

// handles all OUT opcodes
void io_out(BYTE adr, BYTE data) {
	if (adr<4) 			 p_8255_out(adr,data);
	else if (adr<8)  p_ctc_out(adr,data);
	else if (adr<12) p_dart_out(adr,data);
	else if (adr<16) time_out(data);
	else 						 io_trap(adr);
}

// trap unused ports
static BYTE io_trap(BYTE adr) {
	printf("--- No device at port %d! Trapping...\n",adr);
	
	if (i_flag) {
		cpu_error = IOTRAP;
		cpu_state = STOPPED;
	}
	return((BYTE) 0);
}

static BYTE p_8255_in(BYTE port) {
	switch(port&0x03) {
		case 0:
			printf("--- 8255 port A read.\n");
			break;
		case 1:
			printf("--- 8255 port B read.\n");
			break;
		case 2:
			printf("--- 8255 port C read.\n");
			break;
		case 3:
			printf("--- 8255 control port read.\n");
			break;
		default:
			printf("--- 8255 port %d makes no sense!\n",port);
	}

	return 0x00;
}

static void p_8255_out(BYTE port, BYTE data) {
	switch(port&0x03) {
    case 0:
      printf("--- 8255 port A written: %02x\n",data);
    	break;
		case 1:
      printf("--- 8255 port B written: %02x\n",data);
    	break;
		case 2:
      printf("--- 8255 port C written: %02x\n",data);
    	break;
		case 3:
      printf("--- 8255 control port written: %02x\n",data);
    	break;
		default:
      printf("--- 8255 port %d makes no sense!\n",port);
  }
}

// TODO: fix these
static BYTE p_ctc_in(BYTE port) {
  switch(port&0x03) {
    case 0:
      printf("--- CTC channel 0 read.\n");
    	break;
		case 1:
      printf("--- CTC channel 1 read.\n");
    	break;
		case 2:
      printf("--- CTC channel 2 read.\n");
    	break;
		case 3:
      printf("--- CTC control port read.\n");
    	break;
		default:
      printf("--- CTC port %d makes no sense!\n",port);
  }

	return 0x00;
}

static void p_ctc_out(BYTE port, BYTE data) {
  switch(port&0x03) {
    case 0:
      printf("--- CTC channel 0 written: %02x\n",data);
    	break;
		case 1:
      printf("--- CTC channel 1 written: %02x\n",data);
    	break;
		case 2:
      printf("--- CTC channel 2 written: %02x\n",data);
    	break;
		case 3:
      printf("--- CTC control port written: %02x\n",data);
    	break;
		default:
      printf("--- CTC port %d makes no sense!\n",port);
  }
}

static BYTE p_dart_in(BYTE port) {
	char chan='A';
	chan+=(port&0x01); // 'B' if true

	if (port&=0x02) printf("--- DART channel %c control read.\n",chan);
	else						printf("--- DART channel %c data read.\n",chan);

	return 0x00;
}

static void p_dart_out(BYTE port, BYTE data) {
	if (!(port&0x03)) { // serial console: ch A data
		putchar((int)data);
		fflush(stdout);
		return;
	}
	
	char chan='A';
  chan+=(port&0x01);

	if (port&=0x02) printf("--- DART channel %c control written: %02x\n",chan,data);
  else            printf("--- DART channel %c data written: %02x\n",chan,data);
}

/*
 *	I/O function port 1 read:
 *	Read next byte from stdin.
 */
static BYTE p001_in(void)
{
	return((BYTE) getchar());
}

/*
 *	I/O function port 1 write:
 *	Write byte to stdout and flush the output.
 */
static void p001_out(BYTE data)
{
	putchar((int) data);
	fflush(stdout);
}

// ---
static BYTE p002_in(void) {
	printf("--- Port 2 read.\n");
	return 0x80;
}

static void p002_out(BYTE data) {
	printf("--- Port 2 written: 0x%02x\n",data);
}
