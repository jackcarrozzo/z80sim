// vim:set shiftwidth=2 softtabstop=2 expandtab:

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

// TODOs: 
//
// - fix the interrupt handling stuff (again), to have a
// queue with acks.
// - see if i cant clean up some of the globals and pass
// struct ptrs around.

static ctc_state ctc[4];
static dart_state dart[2]; // 0=chan A, 1=chan B

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

	for (i=0;i<2;i++) {
		dart_reset(&dart[i]);
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

	ctc_state *thisctc=&ctc[port];

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

/* --- DART / SIO ---
 * 
 * Implemented: async mode, polling
 * Unimplemented: all other modes, interrupts (TODO)
 *
 * Notes:
 *	- there are a few differences between chan A and B, but they are
 *			treated identically here.
*/

static BYTE p_dart_in(BYTE port) {
	// TODO: this whole thing to match new emulation
	// (dont forget that thisdart->wr_ptr=0; after any read)

	char chan='A'+(port&0x01);

	if (port&=0x02) printf("--- DART channel %c control read.\n",chan);
	else						printf("--- DART channel %c data read.\n",chan);

	return 0x00;
	//return((BYTE) getchar());
}

static void p_dart_out(BYTE port,BYTE data) {
	port&=0x03;
	char chan='A'+(port&0x01);

	dart_state *thisdart=&dart[port&0x01];

	// TODO: split this up and make it easier to read

	if (port&0x02) { // control word
		if (thisdart->wr_ptr) { // write is to a config reg (WR1-7)
			switch (thisdart->wr_ptr) { 
				case 1: // WR1: interrupt config
					if (data) // nonzero interrupt flags set (unimplemented)
						printf("--- DART chan %c: WR1 (ints) written (0x%02x), but unimplemented.\n",
							chan,data);
					else
						printf("--- DART chan %c: WR1 (ints) set zero.\n",chan);

					thisdart->interrupt_mode=data;
					break;

				case 2: // WR2: interrupt vector
					if ('A'==chan) 
						printf("--- DART chan A: WR2 (int vector) written, but only in B. (0x%02x)\n",
							chan,data);
					else
						printf("--- DART chan B: WR2 (int vector) written, but unimplemented. (0x%02x)\n",
              chan,data);
					break;

				case 3:
					if (0xc0!=(data&0xc0)) 
						printf("--- DART chan %c: WR3: RX bits set, but not 8! (0x%02x)\n",chan,data);
					else {
						thisdart->rx_bits=8;
						printf("--- DART chan %c: WR3: RX bits set to 8.\n",chan);
					}

					if (data&0x3e) 
						printf("--- DART chan %c: WR3: unimplemented conf requested! (0x%02x)\n",
							chan,data);
					
					break;

				case 4:
					thisdart->parity=data&0x01;
					if (thisdart->parity) 
						printf("--- DART chan %c: WR4: parity requested, but inimplemented.\n",chan);
					else
						printf("--- DART chan %c: WR4: parity set to none.\n",chan);

					// D1 is odd/even parity, but not checking since we dont care

					thisdart->stopbits=(data&0x0c)>>2;
					if (1==thisdart->stopbits) 
						printf("--- DART chan %c: WR4: 1 stop bit selected.\n",chan);
					else
						printf("--- DART chan %c: WR4: stop bits val %d set, but unimplemented.\n",
							chan,thisdart->stopbits);

					// these magic numbers just set clk_prescale to the values listed on pg 286 
					// of the Z80 peripherals doc (UM008101-0601)
					thisdart->clk_prescale=(data&0xc0)>>6;
					thisdart->clk_prescale=(thisdart->clk_prescale)?(0x10<<(thisdart->clk_prescale-1)):1;
					printf("--- DART chan %c: WR4: clock prescale set to x%d.\n",chan,thisdart->clk_prescale

					break;

				case 5:
					// TODO: parse WR5
					break;

				default: // WR6 and WR7 exist in the SIO series
					printf("--- DART chan %c: WR%d written (0x%02x), but unimplemented.\n",chan,data);
			}

			thisdart->wr_ptr=0; // following read or write to any reg, ptr set back to WR0
		} else { // this write is to WR0
			thisdart->wr_ptr=data&0x07; // last 3 bits set the WR pointer

			BYTE cmd=(data&0x38)>>3;
			// if cmd is 'channel reset', do it:
			if (3==cmd) dart_reset(thisdart); 
			// otherwise if it's not the 'Null' cmd, complain:
			else if (cmd) 
				printf("--- DART chan %c: unimplemented CMD bits written to WR0: 0x%02x\n",
					chan,data);

			if (data&0xc0) printf("--- DART chan %c: CRC reset requested, but unimplemented.\n",chan);
		}
	} else { // data word
		// if configured properly, send it
		// TODO: error if configuration is wack
	}

	// ------
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

static void dart_reset(dart_state *dart) {
    dart->clk_prescale=0;
    dart->rx_bits=0;
    dart->tx_bits=0;
    dart->rx_enabled=0;
    dart->tx_enabled=0;
    dart->stopbits=0;
    dart->parity=0;
    dart->interrupt_mode=0;

    dart->reg_ptr=0;
    dart->wr0_prev=0;

    dart->tx_buf_empty=1;
    dart->rx_char_avail=0;
    dart->all_sent=0;
    dart->rx_buf_overrun=0;
}
