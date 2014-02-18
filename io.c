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

static pio_state pio;  // called pio for var name compliance, but in my application
											 // it is in fact an 82c55
static ctc_state ctc[4];
static dart_state dart[2]; // 0=chan A, 1=chan B

void init_io(void) { // called at start to init all ports
	int i;

	pio.port_a=0;
	pio.port_b=0;
	pio.port_c=0;
	pio.control=0x80; // TODO: look up state after reset

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

// TODO: proper 82c55 emulation
static BYTE p_8255_in(BYTE port) {
	port&=0x03;

	if (3==port) 	printf("--- 8255 control port read.\n");
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
	port&=0x03;
	char chan='A'+(port&0x01);
  char pre[17]; // preamble of our status lines
  sprintf(pre,"--- DART chan %c:",chan);

	dart_state *thisdart=&dart[port&0x01];

	if (port&0x02) { // control
		BYTE resp=0x00;

		switch (thisdart->reg_ptr) {
			case 0: // RR0
				// D7: break/abort. never happens here. could emulate later if needed.
				// D6: Tx underrun
				resp|=thisdart->cts_<<5; // CTS D5
				// D4: SYNC/Hunt. unused for async.
				resp|=thisdart->dcd_<<3; // DCD D3
				resp|=0x04; // tx buffer is always empty in our case (D2)
				// D1: int pending (no ints emulated)
				resp|=(thisdart->rx_char_avail>0); // rx char available D0 
				break;
			case 1: // RR1
				// D7: SDLC end of frame (unused in async)
				// D6: framing error (wont happen here, but could emulate later if needed)
				resp|=(thisdart->rx_buf_overrun<<5); // D5: rx overrun
				// D4: parity error (we arent implementing parity)
				// D3-1: SDLC residue codes (unused in async)
				resp|=thisdart->all_sent; // D0: all sent
				break;
			default:
				printf("%s RR%d requested, but nonexistant.\n",pre,thisdart->reg_ptr);
		}

		thisdart->reg_ptr=0; // back to 0 after any read or write	
		return resp;	
	} else { // data
		if (thisdart->rx_char_avail) {
			thisdart->rx_char_avail--;
			return thisdart->rx_buf[0];
		} else {
			printf("%s read, but no chars available.\n",pre);
			return 0x00;
		}
	}

	//return((BYTE) getchar());
}

static void p_dart_out(BYTE port,BYTE data) {
	port&=0x03;
	char chan='A'+(port&0x01);
  char pre[17]; // preamble of our status lines
  sprintf(pre,"--- DART chan %c:",chan);

	dart_state *thisdart=&dart[port&0x01];

	// TODO: split this up and make it easier to read

	if (port&0x02) { // control word
		char pre[17]; // preamble of our status lines
		sprintf(pre,"--- DART chan %c:",chan);

		if (thisdart->reg_ptr) { // write is to a config reg (WR1-7)
			switch (thisdart->reg_ptr) { 
				case 1: // WR1: interrupt config
					if (data) // nonzero interrupt flags set (unimplemented)
						printf("%s WR1 (ints) written (0x%02x), but unimplemented.\n",pre,data);
					else
						printf("%s WR1 (ints) set zero.\n",pre);

					thisdart->interrupt_mode=data;
					break;

				case 2: // WR2: interrupt vector
					if ('A'==chan) 
						printf("%s WR2 (int vector) written, but only in B. (0x%02x)\n",pre,data);
					else
						printf("%s WR2 (int vector) written, but unimplemented. (0x%02x)\n",pre,data);
					break;

				case 3: // WR3: rx config
					if (0xc0!=(data&0xc0)) 
						printf("%s WR3: RX bits set, but not 8! (0x%02x)\n",pre,data);
					else {
						thisdart->rx_bits=8;
						printf("%s WR3: RX bits set to 8.\n",pre);
					}

					if (data&0x3e) printf("%s WR3: unimplemented conf requested! (0x%02x)\n",pre,data);
					break;

				case 4: // WR4: prescaler, parity, stop bits
					thisdart->parity=data&0x01;
					if (thisdart->parity)	printf("%s WR4: parity requested, but unimplemented.\n",pre);
					else printf("%s WR4: parity set to none.\n",pre);

					// D1 is odd/even parity, but not checking since we dont care

					thisdart->stopbits=(data&0x0c)>>2;
					if (1==thisdart->stopbits) printf("%s WR4: 1 stop bit selected.\n",pre);
					else printf("%s WR4: stop bits val %d set, but unimplemented.\n",
							pre,thisdart->stopbits);

					// these magic numbers just set clk_prescale to the values listed on pg 286 
					// of the Z80 peripherals doc (UM008101-0601)
					thisdart->clk_prescale=(data&0xc0)>>6;
					thisdart->clk_prescale=(thisdart->clk_prescale)?(0x10<<(thisdart->clk_prescale-1)):1;
					printf("%s WR4: clock prescale set to x%d.\n",pre,thisdart->clk_prescale);

					break;

				case 5: // WR5: tx config and status pins
					if (data&0x01) printf("%s WR5: Tx CRC enabled, but unimplemented.\n",pre);

					// normally in async mode, the RTS pin is only active (low) after all bits 
					// are sent, but since we aren't emulating at that level, we allow it to be 
					// set manually here.
					thisdart->rts_=(data&0x02)?0:1; // active low

					thisdart->tx_enabled=(data&0x08)>>3;

					// since we arent emulating at this low a level, support only 8 bits per char
					thisdart->tx_bits=8;					
					if ((data&0x60)==0x60) printf("%s WR5: Tx bits/char set to 8.\n",pre);
					else printf("%s WR5: Tx bits/char set to something other than 8! Using 8.\n",pre);

					thisdart->dtr_=(data&0x80)?0:1; // active low

					break;

				default: // WR6 and WR7 exist in the SIO series only
					printf("%s WR%d written (0x%02x), but unimplemented.\n",pre,thisdart->reg_ptr,data);
			}

			thisdart->reg_ptr=0; // following read or write to any reg, ptr set back to WR0
		} else { // this write is to WR0
			thisdart->reg_ptr=data&0x07; // last 3 bits set the WR pointer

			BYTE cmd=(data&0x38)>>3;
			// if cmd is 'channel reset', do it:
			if (3==cmd) dart_reset(thisdart); 
			// otherwise if it's not the 'Null' cmd, complain:
			else if (cmd) 
				printf("%s Unimplemented CMD bits written to WR0: 0x%02x\n",pre,data);

			if (data&0xc0) printf("%s CRC reset requested, but unimplemented.\n",pre);
		}
	} else { // data word
		// TODO: error if configuration is wack

		printf("%s write: 0x%02x.\n",pre,data);
	}

	/*if (!(port&0x03)) { // serial console: ch A data
		putchar((int)data);
		fflush(stdout);
		return;
	}*/
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

  dart->tx_buf_empty=1;
  dart->rx_char_avail=0;
  dart->all_sent=0;
  dart->rx_buf_overrun=0;
}
