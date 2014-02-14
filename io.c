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
#include "io.h"

// base addresses for io devices (assumes that A1-0 are 
// used in the devices themselves, and A4-2 are demuxed
// to select devices)
#define ADDR_8255 0
#define ADDR_CTC	4
#define ADDR_DART	8

static ctc_state ctc[4];

void init_io(void) { // called at start to init all ports
	int i;

	for (i=0;i<4;i++) {
		ctc[i].ints_enabled=0;
		ctc[i].tc_next=0;
		ctc[i].tc=255;
		ctc[i].ivector=0;
		ctc[i].c_val=255;
		ctc[i].prescaler=255;
		ctc[i].p_val=255;
	}
}

void exit_io(void) { // called at exit
}

// this is written to emulate CTC funtionality if run once per clock - 
//		however, it is currently called from the cpu wrapper and thus only 
//		runs once per instruction, and is as such 4-8x slower than realtime.
// (TODO)
// also, this currently doesnt queue interrupts: only the highest priority is 
//		sent to the cpu and then cleared. TODO: make this match the hardware (ints
//		can be acknowledged separately)
void run_counters(void) {
	int i;

	for (i=3;i>=0;i--) { // in interrupt priority order (most important last)
		if (!ctc[i].ints_enabled) continue; // dont bother with counters that arent
																				// sending interupts

		if (!--ctc[i].p_val) { // prescaler empty
			ctc[i].p_val=ctc[i].prescaler; // refill it

			if (!--ctc[i].c_val) { // counter empty
				ctc[i].c_val=ctc[i].tc; // refill it
				
				int_lsb=ctc[i].ivector; // set the interupt
				int_type=INT_INT;
			}
		}
	}		
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
	port&=0x03;

	if (3==port) 	printf("--- 8255 control port read.\n")
	else					printf("--- 8255 port %c read.\n",'A'+port);

	return 0x00;
}

static void p_8255_out(BYTE port,BYTE data) {
	port&=0x03;

	if (3==port)	printf("--- 8255 control port written: %02x\n",data);
	else					printf("--- 8255 port %c written: %02x\n",'A'+port,data);
}

// reads the current value of the down counter on the specified channel
static BYTE p_ctc_in(BYTE port) {
	port&=0x03;

	printf("--- CTC chan %d read: 0x%02x.\n",port,ctc[port].c_val);
	return ctc[port].c_val;
}

static void p_ctc_out(BYTE port,BYTE data) {
	port&=0x03;

	ctc_state *thisctc;
	thisctc=&ctc[port];

	if (thisctc->tc_next) { // if we indicated the next write would be the tc
		thisctc->tc=data;
		thisctc->tc_next=0;

		printf("--- CTC chan %d TC set to 0x%02x.\n",port,data);
		return;
	}

	if (data&0x01) { // control word
		thisctc->ints_enabled=(data&0x80)?1:0;
		thisctc->tc_next=(data&0x04)?1:0;
		thisctc->prescaler=(data&0x20)?255:15;

		printf("--- CTC chan %d config word set: 0x%02x. ",port,data);

		if (thisctc->ints_enabled) {
			printf("(ints enabled)\n");
		} else {
			printf("(ints disabled)\n");
		}
	} else { // vector word
		// there is only one interrupt vector register on the chip, since
		// bits 2-1 contain the channel. TODO: emulate this.

		thisctc->ivector=(data&0xf8)|(port<<1);  
			
		printf("--- CTC chan %d ivector set: 0x%02x.\n",port,data);
	}
}

static BYTE p_dart_in(BYTE port) {
	char chan='A';
	chan+=(port&0x01);

	if (port&=0x02) printf("--- DART channel %c control read.\n",chan);
	else						printf("--- DART channel %c data read.\n",chan);

	return 0x00;
	//return((BYTE) getchar());
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

