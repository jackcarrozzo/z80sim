/*
 * Z80SIM  -  a	Z80-CPU	simulator
 *
 * Copyright (C) 1987-2008 by Udo Munk
 * 2014 fork by Jack Carrozzo <jack@crepinc.com>
 *
 */

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

#include "config.h"
#include "global.h"

// base addresses for io devices (assumes that A1-0 are 
// used in the devices themselves, and A4-2 are demuxed
// to select devices)
#define ADDR_8255 0
#define ADDR_CTC	4
#define ADDR_DART	8

static BYTE io_trap(BYTE);

static BYTE p_8255_in(BYTE);
static void p_8255_out(BYTE,BYTE);

static BYTE p_ctc_in(BYTE);
static void p_ctc_out(BYTE,BYTE);

static BYTE p_dart_in(BYTE);
static void p_dart_out(BYTE,BYTE);

static BYTE timer; // timer enable flag 
static void int_timer(int);

typedef struct {
	BYTE ints_enabled;
	BYTE tc_next;
	BYTE tc;
	BYTE ivector;
} ctc_state;

static ctc_state ctc[4];
//static struct ctc_state ctc1;
//static struct ctc_state ctc2;
//static struct ctc_state ctc3;

void init_io(void) { // called at start to init all ports
	int i;

	for (i=0;i<4;i++) {
		ctc[i].ints_enabled=0;
		ctc[i].tc_next=0;
		ctc[i].tc=255;
		ctc[i].ivector=0;
	}
}

void exit_io(void) { // called at exit
}

// handles all IN opcodes
BYTE io_in(BYTE adr) {
	switch (adr&0xfc) { // zero the last two bits since they arent relevant
		case ADDR_8255: return p_8255_in(adr);
		case ADDR_CTC:	return p_ctc_in(adr);
		case ADDR_DART:	return p_dart_in(adr);
		default:				io_trap(adr);
	}

	return 0;
}

// handles all OUT opcodes
void io_out(BYTE adr, BYTE data) {
	switch (adr&0xfc) {
		case ADDR_8255: return p_8255_out(adr,data);
    case ADDR_CTC:  return p_ctc_out(adr,data);
    case ADDR_DART: return p_dart_out(adr,data);
    default:        io_trap(adr);
  }
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

/*
 *  I/O handler for read timer
 *  return current status of 10ms interrupt timer,
 *  1 = enabled, 0 = disabled
 */
static BYTE p_ctc_in(BYTE port)
{
  printf("!!! time_in()\n");
  return(timer);
}

static void p_ctc_out(BYTE port, BYTE data) {
  static struct itimerval tim;
  static struct sigaction newact;

	BYTE chan=(port&0x03);
	ctc_state *thisctc;
	thisctc=&ctc[chan];

	if (thisctc->tc_next) { // if we indicated the next write would be the tc
		thisctc->tc=data;
		thisctc->tc_next=0;

		printf("--- CTC chan %d TC set to 0x%02x.\n",chan,data);
		return;
	}

	if (data&0x01) { // control word
		thisctc->ints_enabled=(data&0x80)?1:0;
		thisctc->tc_next=(data&0x04)?1:0;

		printf("--- CTC chan %d config word set: 0x%02x. ",chan,data);

		if (thisctc->ints_enabled) {
	    newact.sa_handler = int_timer;
	    sigaction(SIGALRM, &newact, NULL);
	    tim.it_value.tv_sec = 0;
	    tim.it_value.tv_usec = 10000; // 10ms TODO: link this to TC properly
			setitimer(ITIMER_REAL, &tim, NULL);

			printf("(ints enabled)\n");
		} else {
    	newact.sa_handler = SIG_IGN;
	    sigaction(SIGALRM, &newact, NULL);
	    tim.it_value.tv_sec = 0;
	    tim.it_value.tv_usec = 0;
	  	setitimer(ITIMER_REAL, &tim, NULL);

			printf("(ints disabled)\n");
		}
	} else { // vector word
		// TODO: look up what happens when an interrupt vector word is written
		// to a different channel than that selected by A1-0
		thisctc->ivector=(data&0xf8)|(chan<<1);  
			
		printf("--- CTC chan %d ivector set: 0x%02x.\n",chan,data);
	}
}

// fired from the timer, sets interrupts
static void int_timer(int sig) {
	printf("I-- int_timer()\n");	

  int_lsb = ctc[0].ivector; // LSB for vector table (mode 2 only)
  int_type = INT_INT;
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
static BYTE p001_in(void)
{
	return((BYTE) getchar());
}

static void p001_out(BYTE data)
{
	putchar((int) data);
	fflush(stdout);
}
*/
