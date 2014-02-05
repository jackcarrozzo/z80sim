/*
 * Z80SIM  -  a	Z80-CPU	simulator
 *
 * Copyright (C) 1987-2008 by Udo Munk
 *
 * History:
 * 28-SEP-87 Development on TARGON/35 with AT&T Unix System V.3
 * 11-JAN-89 Release 1.1
 * 08-FEB-89 Release 1.2
 * 13-MAR-89 Release 1.3
 * 09-FEB-90 Release 1.4  Ported to TARGON/31 M10/30
 * 20-DEC-90 Release 1.5  Ported to COHERENT 3.0
 * 10-JUN-92 Release 1.6  long casting problem solved with COHERENT 3.2
 *			  and some optimization
 * 25-JUN-92 Release 1.7  comments in english and ported to COHERENT 4.0
 * 02-OCT-06 Release 1.8  modified to compile on modern POSIX OS's
 * 18-NOV-06 Release 1.9  modified to work with CP/M sources
 * 08-DEC-06 Release 1.10 modified MMU for working with CP/NET
 * 17-DEC-06 Release 1.11 TCP/IP sockets for CP/NET
 * 25-DEC-06 Release 1.12 CPU speed option
 * 19-FEB-07 Release 1.13 various improvements
 * 06-OCT-07 Release 1.14 bug fixes and improvements
 * 06-AUG-08 Release 1.15 many improvements and Windows support via Cygwin
 * 25-AUG-08 Release 1.16 console status I/O loop detection and line discipline
 * 20-OCT-08 Release 1.17 frontpanel integrated and Altair/IMSAI emulations
 */

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "config.h"
#include "global.h"

#ifdef FRONTPANEL
#include "../../frontpanel/frontpanel.h"
#endif

#ifdef WANT_GUI
void check_gui_break(void);
#endif

//static int op_notimpl(void);
static int op_nop(void), op_halt(void), op_scf(void);
static int op_ccf(void), op_cpl(void), op_daa(void);
static int op_ei(void), op_di(void);
static int op_in(void), op_out(void);
static int op_ldan(void), op_ldbn(void), op_ldcn(void);
static int op_lddn(void), op_lden(void);
static int op_ldhn(void), op_ldln(void);
static int op_ldabc(void), op_ldade(void), op_ldann(void);
static int op_ldbca(void), op_lddea(void), op_ldnna(void);
static int op_ldhla(void), op_ldhlb(void), op_ldhlc(void), op_ldhld(void);
static int op_ldhle(void), op_ldhlh(void), op_ldhll(void), op_ldhl1(void);
static int op_ldaa(void), op_ldab(void), op_ldac(void);
static int op_ldad(void), op_ldae(void);
static int op_ldah(void), op_ldal(void), op_ldahl(void);
static int op_ldba(void), op_ldbb(void), op_ldbc(void);
static int op_ldbd(void), op_ldbe(void);
static int op_ldbh(void), op_ldbl(void), op_ldbhl(void);
static int op_ldca(void), op_ldcb(void), op_ldcc(void);
static int op_ldcd(void), op_ldce(void);
static int op_ldch(void), op_ldcl(void), op_ldchl(void);
static int op_ldda(void), op_lddb(void), op_lddc(void);
static int op_lddd(void), op_ldde(void);
static int op_lddh(void), op_lddl(void), op_lddhl(void);
static int op_ldea(void), op_ldeb(void), op_ldec(void);
static int op_lded(void), op_ldee(void);
static int op_ldeh(void), op_ldel(void), op_ldehl(void);
static int op_ldha(void), op_ldhb(void), op_ldhc(void);
static int op_ldhd(void), op_ldhe(void);
static int op_ldhh(void), op_ldhl(void), op_ldhhl(void);
static int op_ldla(void), op_ldlb(void), op_ldlc(void);
static int op_ldld(void), op_ldle(void);
static int op_ldlh(void), op_ldll(void), op_ldlhl(void);
static int op_ldbcnn(void), op_lddenn(void), op_ldhlnn(void);
static int op_ldspnn(void), op_ldsphl(void);
static int op_ldhlin(void), op_ldinhl(void);
static int op_incbc(void), op_incde(void), op_inchl(void), op_incsp(void);
static int op_decbc(void), op_decde(void), op_dechl(void), op_decsp(void);
static int op_adhlbc(void), op_adhlde(void), op_adhlhl(void), op_adhlsp(void);
static int op_anda(void), op_andb(void), op_andc(void), op_andd(void), op_ande(void);
static int op_andh(void), op_andl(void), op_andhl(void), op_andn(void);
static int op_ora(void), op_orb(void), op_orc(void), op_ord(void), op_ore(void);
static int op_orh(void), op_orl(void), op_orhl(void), op_orn(void);
static int op_xora(void), op_xorb(void), op_xorc(void), op_xord(void), op_xore(void);
static int op_xorh(void), op_xorl(void), op_xorhl(void), op_xorn(void);
static int op_adda(void), op_addb(void), op_addc(void), op_addd(void), op_adde(void);
static int op_addh(void), op_addl(void), op_addhl(void), op_addn(void);
static int op_adca(void), op_adcb(void), op_adcc(void), op_adcd(void), op_adce(void);
static int op_adch(void), op_adcl(void), op_adchl(void), op_adcn(void);
static int op_suba(void), op_subb(void), op_subc(void), op_subd(void), op_sube(void);
static int op_subh(void), op_subl(void), op_subhl(void), op_subn(void);
static int op_sbca(void), op_sbcb(void), op_sbcc(void), op_sbcd(void), op_sbce(void);
static int op_sbch(void), op_sbcl(void), op_sbchl(void), op_sbcn(void);
static int op_cpa(void), op_cpb(void), op_cpc(void), op_cpd(void), op_cpe(void);
static int op_cph(void), op_cplr(void), op_cphl(void), op_cpn(void);
static int op_inca(void), op_incb(void), op_incc(void), op_incd(void), op_ince(void);
static int op_inch(void), op_incl(void), op_incihl(void);
static int op_deca(void), op_decb(void), op_decc(void), op_decd(void), op_dece(void);
static int op_dech(void), op_decl(void), op_decihl(void);
static int op_rlca(void), op_rrca(void),op_rla(void),op_rra(void);
static int op_exdehl(void), op_exafaf(void), op_exx(void), op_exsphl(void);
static int op_pushaf(void), op_pushbc(void), op_pushde(void), op_pushhl(void);
static int op_popaf(void), op_popbc(void), op_popde(void), op_pophl(void);
static int op_jp(void), op_jphl(void), op_jr(void), op_djnz(void), op_call(void), op_ret(void);
static int op_jpz(void), op_jpnz(void), op_jpc(void), op_jpnc(void);
static int op_jppe(void), op_jppo(void), op_jpm(void), op_jpp(void);
static int op_calz(void), op_calnz(void), op_calc(void), op_calnc(void);
static int op_calpe(void), op_calpo(void), op_calm(void), op_calp(void);
static int op_retz(void), op_retnz(void), op_retc(void), op_retnc(void);
static int op_retpe(void), op_retpo(void), op_retm(void), op_retp(void);
static int op_jrz(void), op_jrnz(void), op_jrc(void), op_jrnc(void);
static int op_rst00(void), op_rst08(void), op_rst10(void), op_rst18(void);
static int op_rst20(void), op_rst28(void), op_rst30(void), op_rst38(void);
extern int op_cb_handel(void), op_dd_handel(void);
extern int op_ed_handel(void), op_fd_handel(void);

/*
 *	This function builds the Z80 central processing unit.
 *	The opcode where PC points to is fetched from the memory
 *	and PC incremented by one. The opcode is used as an
 *	index to an array with function pointers, to execute a
 *	function which emulates this Z80 opcode.
 */
void cpu(void)
{
	static int (*op_sim[256]) (void) =	{
		op_nop,				/* 0x00	*/
		op_ldbcnn,			/* 0x01	*/
		op_ldbca,			/* 0x02	*/
		op_incbc,			/* 0x03	*/
		op_incb,			/* 0x04	*/
		op_decb,			/* 0x05	*/
		op_ldbn,			/* 0x06	*/
		op_rlca,			/* 0x07	*/
		op_exafaf,			/* 0x08	*/
		op_adhlbc,			/* 0x09	*/
		op_ldabc,			/* 0x0a	*/
		op_decbc,			/* 0x0b	*/
		op_incc,			/* 0x0c	*/
		op_decc,			/* 0x0d	*/
		op_ldcn,			/* 0x0e	*/
		op_rrca,			/* 0x0f	*/
		op_djnz,			/* 0x10	*/
		op_lddenn,			/* 0x11	*/
		op_lddea,			/* 0x12	*/
		op_incde,			/* 0x13	*/
		op_incd,			/* 0x14	*/
		op_decd,			/* 0x15	*/
		op_lddn,			/* 0x16	*/
		op_rla,				/* 0x17	*/
		op_jr,				/* 0x18	*/
		op_adhlde,			/* 0x19	*/
		op_ldade,			/* 0x1a	*/
		op_decde,			/* 0x1b	*/
		op_ince,			/* 0x1c	*/
		op_dece,			/* 0x1d	*/
		op_lden,			/* 0x1e	*/
		op_rra,				/* 0x1f	*/
		op_jrnz,			/* 0x20	*/
		op_ldhlnn,			/* 0x21	*/
		op_ldinhl,			/* 0x22	*/
		op_inchl,			/* 0x23	*/
		op_inch,			/* 0x24	*/
		op_dech,			/* 0x25	*/
		op_ldhn,			/* 0x26	*/
		op_daa,				/* 0x27	*/
		op_jrz,				/* 0x28	*/
		op_adhlhl,			/* 0x29	*/
		op_ldhlin,			/* 0x2a	*/
		op_dechl,			/* 0x2b	*/
		op_incl,			/* 0x2c	*/
		op_decl,			/* 0x2d	*/
		op_ldln,			/* 0x2e	*/
		op_cpl,				/* 0x2f	*/
		op_jrnc,			/* 0x30	*/
		op_ldspnn,			/* 0x31	*/
		op_ldnna,			/* 0x32	*/
		op_incsp,			/* 0x33	*/
		op_incihl,			/* 0x34	*/
		op_decihl,			/* 0x35	*/
		op_ldhl1,			/* 0x36	*/
		op_scf,				/* 0x37	*/
		op_jrc,				/* 0x38	*/
		op_adhlsp,			/* 0x39	*/
		op_ldann,			/* 0x3a	*/
		op_decsp,			/* 0x3b	*/
		op_inca,			/* 0x3c	*/
		op_deca,			/* 0x3d	*/
		op_ldan,			/* 0x3e	*/
		op_ccf,				/* 0x3f	*/
		op_ldbb,			/* 0x40	*/
		op_ldbc,			/* 0x41	*/
		op_ldbd,			/* 0x42	*/
		op_ldbe,			/* 0x43	*/
		op_ldbh,			/* 0x44	*/
		op_ldbl,			/* 0x45	*/
		op_ldbhl,			/* 0x46	*/
		op_ldba,			/* 0x47	*/
		op_ldcb,			/* 0x48	*/
		op_ldcc,			/* 0x49	*/
		op_ldcd,			/* 0x4a	*/
		op_ldce,			/* 0x4b	*/
		op_ldch,			/* 0x4c	*/
		op_ldcl,			/* 0x4d	*/
		op_ldchl,			/* 0x4e	*/
		op_ldca,			/* 0x4f	*/
		op_lddb,			/* 0x50	*/
		op_lddc,			/* 0x51	*/
		op_lddd,			/* 0x52	*/
		op_ldde,			/* 0x53	*/
		op_lddh,			/* 0x54	*/
		op_lddl,			/* 0x55	*/
		op_lddhl,			/* 0x56	*/
		op_ldda,			/* 0x57	*/
		op_ldeb,			/* 0x58	*/
		op_ldec,			/* 0x59	*/
		op_lded,			/* 0x5a	*/
		op_ldee,			/* 0x5b	*/
		op_ldeh,			/* 0x5c	*/
		op_ldel,			/* 0x5d	*/
		op_ldehl,			/* 0x5e	*/
		op_ldea,			/* 0x5f	*/
		op_ldhb,			/* 0x60	*/
		op_ldhc,			/* 0x61	*/
		op_ldhd,			/* 0x62	*/
		op_ldhe,			/* 0x63	*/
		op_ldhh,			/* 0x64	*/
		op_ldhl,			/* 0x65	*/
		op_ldhhl,			/* 0x66	*/
		op_ldha,			/* 0x67	*/
		op_ldlb,			/* 0x68	*/
		op_ldlc,			/* 0x69	*/
		op_ldld,			/* 0x6a	*/
		op_ldle,			/* 0x6b	*/
		op_ldlh,			/* 0x6c	*/
		op_ldll,			/* 0x6d	*/
		op_ldlhl,			/* 0x6e	*/
		op_ldla,			/* 0x6f	*/
		op_ldhlb,			/* 0x70	*/
		op_ldhlc,			/* 0x71	*/
		op_ldhld,			/* 0x72	*/
		op_ldhle,			/* 0x73	*/
		op_ldhlh,			/* 0x74	*/
		op_ldhll,			/* 0x75	*/
		op_halt,			/* 0x76	*/
		op_ldhla,			/* 0x77	*/
		op_ldab,			/* 0x78	*/
		op_ldac,			/* 0x79	*/
		op_ldad,			/* 0x7a	*/
		op_ldae,			/* 0x7b	*/
		op_ldah,			/* 0x7c	*/
		op_ldal,			/* 0x7d	*/
		op_ldahl,			/* 0x7e	*/
		op_ldaa,			/* 0x7f	*/
		op_addb,			/* 0x80	*/
		op_addc,			/* 0x81	*/
		op_addd,			/* 0x82	*/
		op_adde,			/* 0x83	*/
		op_addh,			/* 0x84	*/
		op_addl,			/* 0x85	*/
		op_addhl,			/* 0x86	*/
		op_adda,			/* 0x87	*/
		op_adcb,			/* 0x88	*/
		op_adcc,			/* 0x89	*/
		op_adcd,			/* 0x8a	*/
		op_adce,			/* 0x8b	*/
		op_adch,			/* 0x8c	*/
		op_adcl,			/* 0x8d	*/
		op_adchl,			/* 0x8e	*/
		op_adca,			/* 0x8f	*/
		op_subb,			/* 0x90	*/
		op_subc,			/* 0x91	*/
		op_subd,			/* 0x92	*/
		op_sube,			/* 0x93	*/
		op_subh,			/* 0x94	*/
		op_subl,			/* 0x95	*/
		op_subhl,			/* 0x96	*/
		op_suba,			/* 0x97	*/
		op_sbcb,			/* 0x98	*/
		op_sbcc,			/* 0x99	*/
		op_sbcd,			/* 0x9a	*/
		op_sbce,			/* 0x9b	*/
		op_sbch,			/* 0x9c	*/
		op_sbcl,			/* 0x9d	*/
		op_sbchl,			/* 0x9e	*/
		op_sbca,			/* 0x9f	*/
		op_andb,			/* 0xa0	*/
		op_andc,			/* 0xa1	*/
		op_andd,			/* 0xa2	*/
		op_ande,			/* 0xa3	*/
		op_andh,			/* 0xa4	*/
		op_andl,			/* 0xa5	*/
		op_andhl,			/* 0xa6	*/
		op_anda,			/* 0xa7	*/
		op_xorb,			/* 0xa8	*/
		op_xorc,			/* 0xa9	*/
		op_xord,			/* 0xaa	*/
		op_xore,			/* 0xab	*/
		op_xorh,			/* 0xac	*/
		op_xorl,			/* 0xad	*/
		op_xorhl,			/* 0xae	*/
		op_xora,			/* 0xaf	*/
		op_orb,				/* 0xb0	*/
		op_orc,				/* 0xb1	*/
		op_ord,				/* 0xb2	*/
		op_ore,				/* 0xb3	*/
		op_orh,				/* 0xb4	*/
		op_orl,				/* 0xb5	*/
		op_orhl,			/* 0xb6	*/
		op_ora,				/* 0xb7	*/
		op_cpb,				/* 0xb8	*/
		op_cpc,				/* 0xb9	*/
		op_cpd,				/* 0xba	*/
		op_cpe,				/* 0xbb	*/
		op_cph,				/* 0xbc	*/
		op_cplr,			/* 0xbd	*/
		op_cphl,			/* 0xbe	*/
		op_cpa,				/* 0xbf	*/
		op_retnz,			/* 0xc0	*/
		op_popbc,			/* 0xc1	*/
		op_jpnz,			/* 0xc2	*/
		op_jp,				/* 0xc3	*/
		op_calnz,			/* 0xc4	*/
		op_pushbc,			/* 0xc5	*/
		op_addn,			/* 0xc6	*/
		op_rst00,			/* 0xc7	*/
		op_retz,			/* 0xc8	*/
		op_ret,				/* 0xc9	*/
		op_jpz,				/* 0xca	*/
		op_cb_handel,			/* 0xcb	*/
		op_calz,			/* 0xcc	*/
		op_call,			/* 0xcd	*/
		op_adcn,			/* 0xce	*/
		op_rst08,			/* 0xcf	*/
		op_retnc,			/* 0xd0	*/
		op_popde,			/* 0xd1	*/
		op_jpnc,			/* 0xd2	*/
		op_out,				/* 0xd3	*/
		op_calnc,			/* 0xd4	*/
		op_pushde,			/* 0xd5	*/
		op_subn,			/* 0xd6	*/
		op_rst10,			/* 0xd7	*/
		op_retc,			/* 0xd8	*/
		op_exx,				/* 0xd9	*/
		op_jpc,				/* 0xda	*/
		op_in,				/* 0xdb	*/
		op_calc,			/* 0xdc	*/
		op_dd_handel,			/* 0xdd	*/
		op_sbcn,			/* 0xde	*/
		op_rst18,			/* 0xdf	*/
		op_retpo,			/* 0xe0	*/
		op_pophl,			/* 0xe1	*/
		op_jppo,			/* 0xe2	*/
		op_exsphl,			/* 0xe3	*/
		op_calpo,			/* 0xe4	*/
		op_pushhl,			/* 0xe5	*/
		op_andn,			/* 0xe6	*/
		op_rst20,			/* 0xe7	*/
		op_retpe,			/* 0xe8	*/
		op_jphl,			/* 0xe9	*/
		op_jppe,			/* 0xea	*/
		op_exdehl,			/* 0xeb	*/
		op_calpe,			/* 0xec	*/
		op_ed_handel,			/* 0xed	*/
		op_xorn,			/* 0xee	*/
		op_rst28,			/* 0xef	*/
		op_retp,			/* 0xf0	*/
		op_popaf,			/* 0xf1	*/
		op_jpp,				/* 0xf2	*/
		op_di,				/* 0xf3	*/
		op_calp,			/* 0xf4	*/
		op_pushaf,			/* 0xf5	*/
		op_orn,				/* 0xf6	*/
		op_rst30,			/* 0xf7	*/
		op_retm,			/* 0xf8	*/
		op_ldsphl,			/* 0xf9	*/
		op_jpm,				/* 0xfa	*/
		op_ei,				/* 0xfb	*/
		op_calm,			/* 0xfc	*/
		op_fd_handel,			/* 0xfd	*/
		op_cpn,				/* 0xfe	*/
		op_rst38			/* 0xff	*/
	};

#ifdef WANT_TIM
	register int t = 0;
	struct timespec timer;
#ifdef FRONTPANEL
	register int states;
#endif
#endif

	do {

#ifdef FRONTPANEL	/* update frontpanel */
		fp_led_address = PC - ram;
		fp_led_data = *PC;
		fp_sampleData();
#endif

#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_clock += 7;
		fp_sampleLightGroup(0, 0);
#endif

#ifdef HISIZE		/* write history */
		his[h_next].h_adr = PC - ram;
		his[h_next].h_af = (A << 8) + F;
		his[h_next].h_bc = (B << 8) + C;
		his[h_next].h_de = (D << 8) + E;
		his[h_next].h_hl = (H << 8) + L;
		his[h_next].h_ix = IX;
		his[h_next].h_iy = IY;
		his[h_next].h_sp = STACK - ram;
		h_next++;
		if (h_next == HISIZE) {
			h_flag = 1;
			h_next = 0;
		}
#endif

#ifdef WANT_TIM		/* check for start address of runtime measurement */
		if (PC == t_start && !t_flag) {
			t_flag = 1;	/* switch measurement on */
			t_states = 0L;	/* initialize counted T-states */
		}
#endif

#ifdef WANT_INT		/* CPU interrupt handling */
		if (int_type) // if there is an interrupt available to handle
			// TODO: rewrite the interrupt handling, this is wack

			switch (int_type) {
			case INT_NMI: // NMIs	
				int_type = INT_NONE; // ack the interrupt

				IFF <<= 1;									// TODO: is this the right behavior?

				// this block stores the current execution address on the stack
#ifdef WANT_SPC
				if (STACK <= ram)
					STACK =	ram + 65536L;
#endif
				*--STACK = (PC - ram) >> 8; // put MSB of PC on stack and decrement SP
#ifdef WANT_SPC
				if (STACK <= ram)
					STACK =	ram + 65536L;
#endif
				*--STACK = (PC - ram);			// LSB of PC

				// set the new PC
				PC = ram + 0x66;						// static entry point of NMIs 
		
				if (INT_DEBUG) printf("--- NMI: Jumping to 0x0066\n");

				break;
			case INT_INT:	// maskable ints
				if (3!=IFF) break; 
				//IFF = 0; // why did the interrupt flags get reset here?
	
				switch (int_mode) {
				case 0: // TODO
					printf("Interrupt mode 0 unimplemented! Doing nothing.\n");
					int_type = INT_NONE; // ack the interrupt anyway

					break;
				case 1: // mode 1 works: just jp to 0x0038;
					int_type = INT_NONE;
#ifdef WANT_SPC
					if (STACK <= ram)
						STACK =	ram + 65536L;
#endif
					*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
					if (STACK <= ram)
						STACK =	ram + 65536L;
#endif
					*--STACK = (PC - ram);
					PC = ram + 0x38;

					if (INT_DEBUG) printf("--- Mode 1 Int: Jumping to 0x0038\n");

					break;
				case 2:
					int_vect=(I<<8)+int_lsb; // address of the vector table entry

					// push current PC onto stack
					// TODO: this doesnt bounds check like the others yet
					*--STACK = (PC - ram) >> 8;
					*--STACK = (PC - ram);

					// set the new PC from the vector lookup table
					PC = ram + *(ram + int_vect); 			// LSB of the entry address
					PC += (*(ram + int_vect + 1)) << 8;	// MSB
					
					if (INT_DEBUG) // TODO: mvoe this mode into a cli-configurable var
						printf("--- Mode 2 Int: Lookup from 0x%04x, points to 0x%04lx.\n",int_vect,PC-ram);

					int_type = INT_NONE; // ack the interrupt

					break;
				}
				break;
			}
#endif

#ifdef WANT_TIM
#ifdef FRONTPANEL
		states = (*op_sim[*PC++]) ();	/* execute next opcode */
		t += states;
		fp_clock += states;
#else
		t += (*op_sim[*PC++]) ();
#endif
		if (f_flag) {		/* adjust CPU speed */
			if (t > tmax) {
				timer.tv_sec = 0;
				timer.tv_nsec = 10000000;
				nanosleep(&timer, NULL);
				t = 0;
			}
		}
#else
		(*op_sim[*PC++]) ();
#endif

#ifdef WANT_PCC
		if (PC > ram + 65535)	/* check for PC overrun */
			PC = ram;
#endif

		R++;			/* increment refresh register */

#ifdef WANT_TIM				/* do runtime measurement */
		if (t_flag) {
			t_states += t;	/* add T-states for this opcode */
			if (PC == t_end) /* check for end address */
				t_flag = 0; /* if reached, switch off */
		}
#endif

#ifdef WANT_GUI
                check_gui_break();
#endif

	} while	(cpu_state == CONTIN_RUN);

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
#endif
}

/*
 *	Trap not implemented opcodes. This function may be usefull
 *	later to trap some wanted opcodes.
 */
/*static int op_notimpl(void) // currently unreferenced and generates compiler warnings
{
	cpu_error = OPTRAP1;
	cpu_state = STOPPED;
	return(0);
}*/

static int op_nop(void)			/* NOP */
{
	return(4);
}

static int op_halt(void)		/* HALT */
{
	extern int busy_loop_cnt[];
	struct timespec timer;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_HLTA | CPU_MEMR;
#endif

#ifndef FRONTPANEL
	if (IFF == 0)	{
		cpu_error = OPHALT;
		cpu_state = STOPPED;
	} else
#endif
		while ((int_type == 0) && (cpu_state == CONTIN_RUN)) {
#ifdef FRONTPANEL
			fp_clock += 4;
			fp_sampleData();
#endif
			timer.tv_sec = 0;
			timer.tv_nsec = 1000000L;
			nanosleep(&timer, NULL);
			R += 9999;
		}

	busy_loop_cnt[0] = 0;

	return(0);
}

static int op_scf(void)			/* SCF */
{
	F |= C_FLAG;
	F &= ~(N_FLAG |	H_FLAG);
	return(4);
}

static int op_ccf(void)			/* CCF */
{
	if (F &	C_FLAG)	{
		F |= H_FLAG;
		F &= ~C_FLAG;
	} else {
		F &= ~H_FLAG;
		F |= C_FLAG;
	}
	F &= ~N_FLAG;
	return(4);
}

static int op_cpl(void)			/* CPL */
{
	A = ~A;
	F |= H_FLAG | N_FLAG;
	return(4);
}

static int op_daa(void)			/* DAA */
{
	register int old_a;

	old_a =	A;
	if (F &	N_FLAG)	{		/* subtractions */
		if (((A	& 0x0f)	> 9) ||	(F & H_FLAG)) {
			(((old_a & 0x0f) - 6) <	0) ? (F	|= H_FLAG) : (F	&= ~H_FLAG);
			A = old_a -= 6;
		}
		if (((A	& 0xf0)	> 0x90)	|| (F &	C_FLAG)) {
			A -= 0x60;
			if (old_a - 0x60 < 0)
				F |= C_FLAG;
		}
	} else {			/* additions */
		if (((A	& 0x0f)	> 9) ||	(F & H_FLAG)) {
			(((old_a & 0x0f) + 6) >	0x0f) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
			A = old_a += 6;
		}
		if (((A	& 0xf0)	> 0x90)	|| (F &	C_FLAG)) {
			A += 0x60;
			if (old_a + 0x60 > 255)
				F |= C_FLAG;
		}
	}
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	return(4);
}

static int op_ei(void)			/* EI */
{
	IFF = 3;
	return(4);
}

static int op_di(void)			/* DI */
{
	IFF = 0;
	return(4);
}

static int op_in(void)			/* IN A,(n) */
{
	BYTE io_in();

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_INP;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	A = io_in(*PC++);
	return(11);
}

static int op_out(void)			/* OUT (n),A */
{
	BYTE io_out();

#ifdef BUS_8080
	cpu_bus = CPU_OUT;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	io_out(*PC++, A);
	return(11);
}

static int op_ldan(void)		/* LD A,n */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	A = *PC++;
	return(7);
}

static int op_ldbn(void)		/* LD B,n */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	B = *PC++;
	return(7);
}

static int op_ldcn(void)		/* LD C,n */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	C = *PC++;
	return(7);
}

static int op_lddn(void)		/* LD D,n */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	D = *PC++;
	return(7);
}

static int op_lden(void)		/* LD E,n */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	E = *PC++;
	return(7);
}

static int op_ldhn(void)		/* LD H,n */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	H = *PC++;
	return(7);
}

static int op_ldln(void)		/* LD L,n */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	L = *PC++;
	return(7);
}

static int op_ldabc(void)		/* LD A,(BC) */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	A = *(ram + (B << 8) + C);
	return(7);
}

static int op_ldade(void)		/* LD A,(DE) */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	A = *(ram + (D << 8) + E);
	return(7);
}

static int op_ldann(void)		/* LD A,(nn) */
{
	register unsigned i;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	i = *PC++;
	i += *PC++ << 8;
	A = *(ram + i);
	return(13);
}

static int op_ldbca(void)		/* LD (BC),A */
{
#ifdef BUS_8080
	cpu_bus = 0;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	*(ram +	(B << 8) + C) =	A;
	return(7);
}

static int op_lddea(void)		/* LD (DE),A */
{
#ifdef BUS_8080
	cpu_bus = 0;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	*(ram +	(D << 8) + E) =	A;
	return(7);
}

static int op_ldnna(void)		/* LD (nn),A */
{
	register unsigned i;

#ifdef BUS_8080
	cpu_bus = 0;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	i = *PC++;
	i += *PC++ << 8;
	*(ram +	i) = A;
	return(13);
}

static int op_ldhla(void)		/* LD (HL),A */
{
#ifdef BUS_8080
	cpu_bus = 0;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	*(ram +	(H << 8) + L) =	A;
	return(7);
}

static int op_ldhlb(void)		/* LD (HL),B */
{
#ifdef BUS_8080
	cpu_bus = 0;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	*(ram +	(H << 8) + L) =	B;
	return(7);
}

static int op_ldhlc(void)		/* LD (HL),C */
{
#ifdef BUS_8080
	cpu_bus = 0;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	*(ram +	(H << 8) + L) =	C;
	return(7);
}

static int op_ldhld(void)		/* LD (HL),D */
{
#ifdef BUS_8080
	cpu_bus = 0;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	*(ram +	(H << 8) + L) =	D;
	return(7);
}

static int op_ldhle(void)		/* LD (HL),E */
{
#ifdef BUS_8080
	cpu_bus = 0;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	*(ram +	(H << 8) + L) =	E;
	return(7);
}

static int op_ldhlh(void)		/* LD (HL),H */
{
#ifdef BUS_8080
	cpu_bus = 0;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	*(ram +	(H << 8) + L) =	H;
	return(7);
}

static int op_ldhll(void)		/* LD (HL),L */
{
#ifdef BUS_8080
	cpu_bus = 0;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	*(ram +	(H << 8) + L) =	L;
	return(7);
}

static int op_ldhl1(void)		/* LD (HL),n */
{
#ifdef BUS_8080
	cpu_bus = 0;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	*(ram +	(H << 8) + L) =	*PC++;
	return(10);
}

static int op_ldaa(void)		/* LD A,A */
{
	return(4);
}

static int op_ldab(void)		/* LD A,B */
{
	A = B;
	return(4);
}

static int op_ldac(void)		/* LD A,C */
{
	A = C;
	return(4);
}

static int op_ldad(void)		/* LD A,D */
{
	A = D;
	return(4);
}

static int op_ldae(void)		/* LD A,E */
{
	A = E;
	return(4);
}

static int op_ldah(void)		/* LD A,H */
{
	A = H;
	return(4);
}

static int op_ldal(void)		/* LD A,L */
{
	A = L;
	return(4);
}

static int op_ldahl(void)		/* LD A,(HL) */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	A = *(ram + (H << 8) + L);
	return(7);
}

static int op_ldba(void)		/* LD B,A */
{
	B = A;
	return(4);
}

static int op_ldbb(void)		/* LD B,B */
{
	return(4);
}

static int op_ldbc(void)		/* LD B,C */
{
	B = C;
	return(4);
}

static int op_ldbd(void)		/* LD B,D */
{
	B = D;
	return(4);
}

static int op_ldbe(void)		/* LD B,E */
{
	B = E;
	return(4);
}

static int op_ldbh(void)		/* LD B,H */
{
	B = H;
	return(4);
}

static int op_ldbl(void)		/* LD B,L */
{
	B = L;
	return(4);
}

static int op_ldbhl(void)		/* LD B,(HL) */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	B = *(ram + (H << 8) + L);
	return(7);
}

static int op_ldca(void)		/* LD C,A */
{
	C = A;
	return(4);
}

static int op_ldcb(void)		/* LD C,B */
{
	C = B;
	return(4);
}

static int op_ldcc(void)		/* LD C,C */
{
	return(4);
}

static int op_ldcd(void)		/* LD C,D */
{
	C = D;
	return(4);
}

static int op_ldce(void)		/* LD C,E */
{
	C = E;
	return(4);
}

static int op_ldch(void)		/* LD C,H */
{
	C = H;
	return(4);
}

static int op_ldcl(void)		/* LD C,L */
{
	C = L;
	return(4);
}

static int op_ldchl(void)		/* LD C,(HL) */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	C = *(ram + (H << 8) + L);
	return(7);
}

static int op_ldda(void)		/* LD D,A */
{
	D = A;
	return(4);
}

static int op_lddb(void)		/* LD D,B */
{
	D = B;
	return(4);
}

static int op_lddc(void)		/* LD D,C */
{
	D = C;
	return(4);
}

static int op_lddd(void)		/* LD D,D */
{
	return(4);
}

static int op_ldde(void)		/* LD D,E */
{
	D = E;
	return(4);
}

static int op_lddh(void)		/* LD D,H */
{
	D = H;
	return(4);
}

static int op_lddl(void)		/* LD D,L */
{
	D = L;
	return(4);
}

static int op_lddhl(void)		/* LD D,(HL) */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	D = *(ram + (H << 8) + L);
	return(7);
}

static int op_ldea(void)		/* LD E,A */
{
	E = A;
	return(4);
}

static int op_ldeb(void)		/* LD E,B */
{
	E = B;
	return(4);
}

static int op_ldec(void)		/* LD E,C */
{
	E = C;
	return(4);
}

static int op_lded(void)		/* LD E,D */
{
	E = D;
	return(4);
}

static int op_ldee(void)		/* LD E,E */
{
	return(4);
}

static int op_ldeh(void)		/* LD E,H */
{
	E = H;
	return(4);
}

static int op_ldel(void)		/* LD E,L */
{
	E = L;
	return(4);
}

static int op_ldehl(void)		/* LD E,(HL) */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	E = *(ram + (H << 8) + L);
	return(7);
}

static int op_ldha(void)		/* LD H,A */
{
	H = A;
	return(4);
}

static int op_ldhb(void)		/* LD H,B */
{
	H = B;
	return(4);
}

static int op_ldhc(void)		/* LD H,C */
{
	H = C;
	return(4);
}

static int op_ldhd(void)		/* LD H,D */
{
	H = D;
	return(4);
}

static int op_ldhe(void)		/* LD H,E */
{
	H = E;
	return(4);
}

static int op_ldhh(void)		/* LD H,H */
{
	return(4);
}

static int op_ldhl(void)		/* LD H,L */
{
	H = L;
	return(4);
}

static int op_ldhhl(void)		/* LD H,(HL) */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	H = *(ram + (H << 8) + L);
	return(7);
}

static int op_ldla(void)		/* LD L,A */
{
	L = A;
	return(4);
}

static int op_ldlb(void)		/* LD L,B */
{
	L = B;
	return(4);
}

static int op_ldlc(void)		/* LD L,C */
{
	L = C;
	return(4);
}

static int op_ldld(void)		/* LD L,D */
{
	L = D;
	return(4);
}

static int op_ldle(void)		/* LD L,E */
{
	L = E;
	return(4);
}

static int op_ldlh(void)		/* LD L,H */
{
	L = H;
	return(4);
}

static int op_ldll(void)		/* LD L,L */
{
	return(4);
}

static int op_ldlhl(void)		/* LD L,(HL) */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	L = *(ram + (H << 8) + L);
	return(7);
}

static int op_ldbcnn(void)		/* LD BC,nn */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	C = *PC++;
	B = *PC++;
	return(10);
}

static int op_lddenn(void)		/* LD DE,nn */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	E = *PC++;
	D = *PC++;
	return(10);
}

static int op_ldhlnn(void)		/* LD HL,nn */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	L = *PC++;
	H = *PC++;
	return(10);
}

static int op_ldspnn(void)		/* LD SP,nn */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	STACK =	ram + *PC++;
	STACK += *PC++ << 8;
	return(10);
}

static int op_ldsphl(void)		/* LD SP,HL */
{
	STACK =	ram + (H << 8) + L;
	return(6);
}

static int op_ldhlin(void)		/* LD HL,(nn) */
{
	register unsigned i;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	i = *PC++;
	i += *PC++ << 8;
	L = *(ram + i);
	H = *(ram + i +	1);
	return(16);
}

static int op_ldinhl(void)		/* LD (nn),HL */
{
	register unsigned i;

#ifdef BUS_8080
	cpu_bus = 0;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	i = *PC++;
	i += *PC++ << 8;
	*(ram +	i) = L;
	*(ram +	i + 1) = H;
	return(16);
}

static int op_incbc(void)		/* INC BC */
{
	C++;
	if (!C)
		B++;
	return(6);
}

static int op_incde(void)		/* INC DE */
{
	E++;
	if (!E)
		D++;
	return(6);
}

static int op_inchl(void)		/* INC HL */
{
	L++;
	if (!L)
		H++;
	return(6);
}

static int op_incsp(void)		/* INC SP */
{
	STACK++;
#ifdef WANT_SPC
	if (STACK > ram	+ 65535)
		STACK =	ram;
#endif
	return(6);
}

static int op_decbc(void)		/* DEC BC */
{
	C--;
	if (C == 0xff)
		B--;
	return(6);
}

static int op_decde(void)		/* DEC DE */
{
	E--;
	if (E == 0xff)
		D--;
	return(6);
}

static int op_dechl(void)		/* DEC HL */
{
	L--;
	if (L == 0xff)
		H--;
	return(6);
}

static int op_decsp(void)		/* DEC SP */
{
	STACK--;
#ifdef WANT_SPC
	if (STACK < ram)
		STACK =	ram + 65535;
#endif
	return(6);
}

static int op_adhlbc(void)		/* ADD HL,BC */
{
	register int carry;

	carry = (L + C > 255) ? 1 : 0;
	L += C;
	((H & 0xf) + (B & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(H + B + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += B + carry;
	F &= ~N_FLAG;
	return(11);
}

static int op_adhlde(void)		/* ADD HL,DE */
{
	register int carry;

	carry = (L + E > 255) ? 1 : 0;
	L += E;
	((H & 0xf) + (D & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(H + D + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += D + carry;
	F &= ~N_FLAG;
	return(11);
}

static int op_adhlhl(void)		/* ADD HL,HL */
{
	register int carry;

	carry = (L << 1 > 255) ? 1 : 0;
	L <<= 1;
	((H & 0xf) + (H & 0xf) + carry > 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(H + H + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += H + carry;
	F &= ~N_FLAG;
	return(11);
}

static int op_adhlsp(void)		/* ADD HL,SP */
{
	register int carry;
	BYTE spl = (STACK - ram) & 0xff;
	BYTE sph = (STACK - ram) >> 8;
	
	carry = (L + spl > 255) ? 1 : 0;
	L += spl;
	((H & 0xf) + (sph & 0xf) + carry > 0xf) ? (F |= H_FLAG)
						: (F &= ~H_FLAG);
	(H + sph + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += sph + carry;
	F &= ~N_FLAG;
	return(11);
}

static int op_anda(void)		/* AND A */
{
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(N_FLAG |	C_FLAG);
	return(4);
}

static int op_andb(void)		/* AND B */
{
	A &= B;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(N_FLAG |	C_FLAG);
	return(4);
}

static int op_andc(void)		/* AND C */
{
	A &= C;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(N_FLAG |	C_FLAG);
	return(4);
}

static int op_andd(void)		/* AND D */
{
	A &= D;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(N_FLAG |	C_FLAG);
	return(4);
}

static int op_ande(void)		/* AND E */
{
	A &= E;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(N_FLAG |	C_FLAG);
	return(4);
}

static int op_andh(void)		/* AND H */
{
	A &= H;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(N_FLAG |	C_FLAG);
	return(4);
}

static int op_andl(void)		/* AND L */
{
	A &= L;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(N_FLAG |	C_FLAG);
	return(4);
}

static int op_andhl(void)		/* AND (HL) */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	A &= *(ram + (H	<< 8) +	L);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(N_FLAG |	C_FLAG);
	return(7);
}

static int op_andn(void)		/* AND n */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	A &= *PC++;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= H_FLAG;
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(N_FLAG |	C_FLAG);
	return(7);
}

static int op_ora(void)			/* OR A */
{
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(4);
}

static int op_orb(void)			/* OR B */
{
	A |= B;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(4);
}

static int op_orc(void)			/* OR C */
{
	A |= C;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(4);
}

static int op_ord(void)			/* OR D */
{
	A |= D;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(4);
}

static int op_ore(void)			/* OR E */
{
	A |= E;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(4);
}

static int op_orh(void)			/* OR H */
{
	A |= H;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(4);
}

static int op_orl(void)			/* OR L */
{
	A |= L;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(4);
}

static int op_orhl(void)		/* OR (HL)	*/
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	A |= *(ram + (H	<< 8) +	L);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(7);
}

static int op_orn(void)			/* OR n */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	A |= *PC++;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(7);
}

static int op_xora(void)		/* XOR A */
{
	A = 0;
	F &= ~(S_FLAG |	H_FLAG | N_FLAG	| C_FLAG);
	F |= Z_FLAG | P_FLAG;
	return(4);
}

static int op_xorb(void)		/* XOR B */
{
	A ^= B;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(4);
}

static int op_xorc(void)		/* XOR C */
{
	A ^= C;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(4);
}

static int op_xord(void)		/* XOR D */
{
	A ^= D;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(4);
}

static int op_xore(void)		/* XOR E */
{
	A ^= E;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(4);
}

static int op_xorh(void)		/* XOR H */
{
	A ^= H;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(4);
}

static int op_xorl(void)		/* XOR L */
{
	A ^= L;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(4);
}

static int op_xorhl(void)		/* XOR (HL) */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	A ^= *(ram + (H	<< 8) +	L);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(7);
}

static int op_xorn(void)		/* XOR n */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	A ^= *PC++;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parrity[A]) ? (F &= ~P_FLAG) :	(F |= P_FLAG);
	F &= ~(H_FLAG |	N_FLAG | C_FLAG);
	return(7);
}

static int op_adda(void)		/* ADD A,A */
{
	register int i;

	((A & 0xf) + (A	& 0xf) > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	((A << 1) > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i =	((signed char)	A) << 1;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_addb(void)		/* ADD A,B */
{
	register int i;

	((A & 0xf) + (B	& 0xf) > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + B > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) B;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_addc(void)		/* ADD A,C */
{
	register int i;

	((A & 0xf) + (C	& 0xf) > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + C > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) C;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_addd(void)		/* ADD A,D */
{
	register int i;

	((A & 0xf) + (D	& 0xf) > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + D > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) D;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_adde(void)		/* ADD A,E */
{
	register int i;

	((A & 0xf) + (E	& 0xf) > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + E > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) E;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_addh(void)		/* ADD A,H */
{
	register int i;

	((A & 0xf) + (H	& 0xf) > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + H > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) H;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_addl(void)		/* ADD A,L */
{
	register int i;

	((A & 0xf) + (L	& 0xf) > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + L > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) L;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_addhl(void)		/* ADD A,(HL) */
{
	register int i;
	register BYTE P;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	P = *(ram + (H << 8) + L);
	((A & 0xf) + (P	& 0xf) > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + P > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) P;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(7);
}

static int op_addn(void)		/* ADD A,n */
{
	register int i;
	register BYTE P;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	P = *PC++;
	((A & 0xf) + (P	& 0xf) > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + P > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) P;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(7);
}

static int op_adca(void)		/* ADC A,A */
{
	register int i,	carry;

	carry =	(F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (A	& 0xf) + carry > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	((A << 1) + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = i =	(((signed char) A) << 1) + carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_adcb(void)		/* ADC A,B */
{
	register int i,	carry;

	carry =	(F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (B	& 0xf) + carry > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + B + carry > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) B + carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_adcc(void)		/* ADC A,C */
{
	register int i,	carry;

	carry =	(F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (C	& 0xf) + carry > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + C + carry > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) C + carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_adcd(void)		/* ADC A,D */
{
	register int i,	carry;

	carry =	(F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (D	& 0xf) + carry > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + D + carry > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) D + carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_adce(void)		/* ADC A,E */
{
	register int i,	carry;

	carry =	(F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (E	& 0xf) + carry > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + E + carry > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) E + carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_adch(void)		/* ADC A,H */
{
	register int i,	carry;

	carry =	(F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (H	& 0xf) + carry > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + H + carry > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) H + carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_adcl(void)		/* ADC A,L */
{
	register int i,	carry;

	carry =	(F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (L	& 0xf) + carry > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + L + carry > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) L + carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_adchl(void)		/* ADC A,(HL) */
{
	register int i,	carry;
	register BYTE P;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	P = *(ram + (H << 8) + L);
	carry =	(F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (P	& 0xf) + carry > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + P + carry > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) P + carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(7);
}

static int op_adcn(void)		/* ADC A,n */
{
	register int i,	carry;
	register BYTE P;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	carry =	(F & C_FLAG) ? 1 : 0;
	P = *PC++;
	((A & 0xf) + (P	& 0xf) + carry > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	(A + P + carry > 255) ?	(F |= C_FLAG) :	(F &= ~C_FLAG);
	A = i =	(signed char) A + (signed char) P + carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(7);
}

static int op_suba(void)		/* SUB A,A */
{
	A = 0;
	F &= ~(S_FLAG |	H_FLAG | P_FLAG	| C_FLAG);
	F |= Z_FLAG | N_FLAG;
	return(4);
}

static int op_subb(void)		/* SUB A,B */
{
	register int i;

	((B & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(B > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) B;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_subc(void)		/* SUB A,C */
{
	register int i;

	((C & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(C > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) C;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_subd(void)		/* SUB A,D */
{
	register int i;

	((D & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(D > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) D;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_sube(void)		/* SUB A,E */
{
	register int i;

	((E & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(E > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) E;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_subh(void)		/* SUB A,H */
{
	register int i;

	((H & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(H > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) H;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_subl(void)		/* SUB A,L */
{
	register int i;

	((L & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(L > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) L;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_subhl(void)		/* SUB A,(HL) */
{
	register int i;
	register BYTE P;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	P = *(ram + (H << 8) + L);
	((P & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(P > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) P;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(7);
}

static int op_subn(void)		/* SUB A,n */
{
	register int i;
	register BYTE P;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	P = *PC++;
	((P & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(P > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) P;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(7);
}

static int op_sbca(void)		/* SBC A,A */
{
	if (F &	C_FLAG)	{
		F |= S_FLAG | H_FLAG | N_FLAG |	C_FLAG;
		F &= ~(Z_FLAG |	P_FLAG);
		A = 255;
	} else {
		F |= Z_FLAG | N_FLAG;
		F &= ~(S_FLAG |	H_FLAG | P_FLAG	| C_FLAG);
		A = 0;
	}
	return(4);
}

static int op_sbcb(void)		/* SBC A,B */
{
	register int i,	carry;

	carry =	(F & C_FLAG) ? 1 : 0;
	((B & 0xf) + carry > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(B + carry > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) B - carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_sbcc(void)		/* SBC A,C */
{
	register int i,	carry;

	carry =	(F & C_FLAG) ? 1 : 0;
	((C & 0xf) + carry > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(C + carry > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) C - carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_sbcd(void)		/* SBC A,D */
{
	register int i,	carry;

	carry =	(F & C_FLAG) ? 1 : 0;
	((D & 0xf) + carry > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(D + carry > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) D - carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_sbce(void)		/* SBC A,E */
{
	register int i,	carry;

	carry =	(F & C_FLAG) ? 1 : 0;
	((E & 0xf) + carry > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(E + carry > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) E - carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_sbch(void)		/* SBC A,H */
{
	register int i,	carry;

	carry =	(F & C_FLAG) ? 1 : 0;
	((H & 0xf) + carry > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(H + carry > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) H - carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_sbcl(void)		/* SBC A,L */
{
	register int i,	carry;

	carry =	(F & C_FLAG) ? 1 : 0;
	((L & 0xf) + carry > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(L + carry > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) L - carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_sbchl(void)		/* SBC A,(HL) */
{
	register int i,	carry;
	register BYTE P;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	P = *(ram + (H << 8) + L);
	carry =	(F & C_FLAG) ? 1 : 0;
	((P & 0xf) + carry > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(P + carry > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) P - carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(7);
}

static int op_sbcn(void)		/* SBC A,n */
{
	register int i,	carry;
	register BYTE P;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	P = *PC++;
	carry =	(F & C_FLAG) ? 1 : 0;
	((P & 0xf) + carry > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(P + carry > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	A = i =	(signed char) A - (signed char) P - carry;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(7);
}

static int op_cpa(void)			/* CP A */
{
	F &= ~(S_FLAG |	H_FLAG | P_FLAG	| C_FLAG);
	F |= Z_FLAG | N_FLAG;
	return(4);
}

static int op_cpb(void)			/* CP B */
{
	register int i;

	((B & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(B > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	i = (signed char) A - (signed char) B;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_cpc(void)			/* CP C */
{
	register int i;

	((C & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(C > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	i = (signed char) A - (signed char) C;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_cpd(void)			/* CP D */
{
	register int i;

	((D & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(D > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	i = (signed char) A - (signed char) D;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_cpe(void)			/* CP E */
{
	register int i;

	((E & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(E > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	i = (signed char) A - (signed char) E;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_cph(void)			/* CP H */
{
	register int i;

	((H & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(H > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	i = (signed char) A - (signed char) H;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_cplr(void)		/* CP L */
{
	register int i;

	((L & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(L > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	i = (signed char) A - (signed char) L;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_cphl(void)		/* CP (HL) */
{
	register int i;
	register BYTE P;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	P = *(ram + (H << 8) + L);
	((P & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(P > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	i = (signed char) A - (signed char) P;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(7);
}

static int op_cpn(void)			/* CP n */
{
	register int i;
	register BYTE P;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	P = *PC++;
	((P & 0xf) > (A	& 0xf))	? (F |=	H_FLAG)	: (F &=	~H_FLAG);
	(P > A)	? (F |=	C_FLAG)	: (F &=	~C_FLAG);
	i = (signed char) A - (signed char) P;
	(i < -128 || i > 127) ?	(F |= P_FLAG) :	(F &= ~P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(7);
}

static int op_inca(void)		/* INC A */
{
	((A & 0xf) + 1 > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	A++;
	(A == 128) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_incb(void)		/* INC B */
{
	((B & 0xf) + 1 > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	B++;
	(B == 128) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_incc(void)		/* INC C */
{
	((C & 0xf) + 1 > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	C++;
	(C == 128) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_incd(void)		/* INC D */
{
	((D & 0xf) + 1 > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	D++;
	(D == 128) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_ince(void)		/* INC E */
{
	((E & 0xf) + 1 > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	E++;
	(E == 128) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_inch(void)		/* INC H */
{
	((H & 0xf) + 1 > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	H++;
	(H == 128) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_incl(void)		/* INC L */
{
	((L & 0xf) + 1 > 0xf) ?	(F |= H_FLAG) :	(F &= ~H_FLAG);
	L++;
	(L == 128) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F &= ~N_FLAG;
	return(4);
}

static int op_incihl(void)		/* INC (HL) */
{
	register BYTE *p;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	p = ram	+ (H <<	8) + L;
	((*p & 0xf) + 1	> 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(*p)++;
	(*p == 128) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(*p & 128) ? (F	|= S_FLAG) : (F	&= ~S_FLAG);
	(*p) ? (F &= ~Z_FLAG) :	(F |= Z_FLAG);
	F &= ~N_FLAG;
	return(11);
}

static int op_deca(void)		/* DEC A */
{
	(((A - 1) & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	A--;
	(A == 127) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_decb(void)		/* DEC B */
{
	(((B - 1) & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	B--;
	(B == 127) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_decc(void)		/* DEC C */
{
	(((C - 1) & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	C--;
	(C == 127) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_decd(void)		/* DEC D */
{
	(((D - 1) & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	D--;
	(D == 127) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_dece(void)		/* DEC E */
{
	(((E - 1) & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	E--;
	(E == 127) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_dech(void)		/* DEC H */
{
	(((H - 1) & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	H--;
	(H == 127) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_decl(void)		/* DEC L */
{
	(((L - 1) & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	L--;
	(L == 127) ? (F	|= P_FLAG) : (F	&= ~P_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	F |= N_FLAG;
	return(4);
}

static int op_decihl(void)		/* DEC (HL) */
{
	register BYTE *p;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	p = ram	+ (H <<	8) + L;
	(((*p - 1) & 0xf) == 0xf) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(*p)--;
	(*p == 127) ? (F |= P_FLAG) : (F &= ~P_FLAG);
	(*p & 128) ? (F	|= S_FLAG) : (F	&= ~S_FLAG);
	(*p) ? (F &= ~Z_FLAG) :	(F |= Z_FLAG);
	F |= N_FLAG;
	return(11);
}

static int op_rlca(void)		/* RLCA */
{
	register int i;

	i = (A & 128) ?	1 : 0;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG |	N_FLAG);
	A <<= 1;
	A |= i;
	return(4);
}

static int op_rrca(void)		/* RRCA */
{
	register int i;

	i = A &	1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG |	N_FLAG);
	A >>= 1;
	if (i) A |= 128;
	return(4);
}

static int op_rla(void)			/* RLA */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG |	N_FLAG);
	A <<= 1;
	if (old_c_flag)	A |= 1;
	return(4);
}

static int op_rra(void)			/* RRA */
{
	register int i,	old_c_flag;

	old_c_flag = F & C_FLAG;
	i = A &	1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	F &= ~(H_FLAG |	N_FLAG);
	A >>= 1;
	if (old_c_flag)	A |= 128;
	return(4);
}

static int op_exdehl(void)		/* EX DE,HL */
{
	register unsigned i;

	i = D;
	D = H;
	H = i;
	i = E;
	E = L;
	L = i;
	return(4);
}

static int op_exafaf(void)		/* EX AF,AF' */
{
	register unsigned i;

	i = A;
	A = A_;
	A_ = i;
	i = F;
	F = F_;
	F_ = i;
	return(4);
}

static int op_exx(void)			/* EXX */
{
	register unsigned i;

	i = B;
	B = B_;
	B_ = i;
	i = C;
	C = C_;
	C_ = i;
	i = D;
	D = D_;
	D_ = i;
	i = E;
	E = E_;
	E_ = i;
	i = H;
	H = H_;
	H_ = i;
	i = L;
	L = L_;
	L_ = i;
	return(4);
}

static int op_exsphl(void)		/* EX (SP),HL */
{
	register int i;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	i = *STACK;
	*STACK = L;
	L = i;
	i = *(STACK + 1);
	*(STACK	+ 1) = H;
	H = i;
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	return(19);
}

static int op_pushaf(void)		/* PUSH AF */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = A;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = F;
	return(11);
}

static int op_pushbc(void)		/* PUSH BC */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = B;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = C;
	return(11);
}

static int op_pushde(void)		/* PUSH DE */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = D;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = E;
	return(11);
}

static int op_pushhl(void)		/* PUSH HL */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = H;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = L;
	return(11);
}

static int op_popaf(void)		/* POP AF */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	F = *STACK++;
#ifdef WANT_SPC
	if (STACK >= ram + 65536L)
		STACK =	ram;
#endif
	A = *STACK++;
#ifdef WANT_SPC
	if (STACK >= ram + 65536L)
		STACK =	ram;
#endif
	return(10);
}

static int op_popbc(void)		/* POP BC */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	C = *STACK++;
#ifdef WANT_SPC
	if (STACK >= ram + 65536L)
		STACK =	ram;
#endif
	B = *STACK++;
#ifdef WANT_SPC
	if (STACK >= ram + 65536L)
		STACK =	ram;
#endif
	return(10);
}

static int op_popde(void)		/* POP DE */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	E = *STACK++;
#ifdef WANT_SPC
	if (STACK >= ram + 65536L)
		STACK =	ram;
#endif
	D = *STACK++;
#ifdef WANT_SPC
	if (STACK >= ram + 65536L)
		STACK =	ram;
#endif
	return(10);
}

static int op_pophl(void)		/* POP HL */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	L = *STACK++;
#ifdef WANT_SPC
	if (STACK >= ram + 65536L)
		STACK =	ram;
#endif
	H = *STACK++;
#ifdef WANT_SPC
	if (STACK >= ram + 65536L)
		STACK =	ram;
#endif
	return(10);
}

static int op_jp(void)			/* JP */
{
	register unsigned i;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif

	i = *PC++;
	i += *PC << 8;
	PC = ram + i;
	return(10);
}

static int op_jphl(void)		/* JP (HL) */
{
	PC = ram + (H << 8) + L;
	return(4);
}

static int op_jr(void)			/* JR */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif

	PC += (signed char) *PC + 1;
	return(12);
}

static int op_djnz(void)		/* DJNZ */
{
	if (--B) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		PC += (signed char) *PC + 1;
		return(13);
	} else {
		PC++;
		return(8);
	}
}

static int op_call(void)		/* CALL */
{
	register unsigned i;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	i = *PC++;
	i += *PC++ << 8;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram);
	PC = ram + i;
	return(17);
}

static int op_ret(void)			/* RET */
{
	register unsigned i;

#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
	i = *STACK++;
#ifdef WANT_SPC
	if (STACK >= ram + 65536L)
		STACK =	ram;
#endif
	i += *STACK++ << 8;
#ifdef WANT_SPC
	if (STACK >= ram + 65536L)
		STACK =	ram;
#endif
	PC = ram + i;
	return(10);
}

static int op_jpz(void)			/* JP Z,nn */
{
	register unsigned i;

	if (F &	Z_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
		PC = ram + i;
	} else
		PC += 2;
	return(10);
}

static int op_jpnz(void)		/* JP NZ,nn */
{
	register unsigned i;

	if (!(F	& Z_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
		PC = ram + i;
	} else
		PC += 2;
	return(10);
}

static int op_jpc(void)			/* JP C,nn */
{
	register unsigned i;

	if (F &	C_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
		PC = ram + i;
	} else
		PC += 2;
	return(10);
}

static int op_jpnc(void)		/* JP NC,nn */
{
	register unsigned i;

	if (!(F	& C_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
		PC = ram + i;
	} else
		PC += 2;
	return(10);
}

static int op_jppe(void)		/* JP PE,nn */
{
	register unsigned i;

	if (F &	P_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
		PC = ram + i;
	} else
		PC += 2;
	return(10);
}

static int op_jppo(void)		/* JP PO,nn */
{
	register unsigned i;

	if (!(F	& P_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
		PC = ram + i;
	} else
		PC += 2;
	return(10);
}

static int op_jpm(void)			/* JP M,nn */
{
	register unsigned i;

	if (F &	S_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
		PC = ram + i;
	} else
		PC += 2;
	return(10);
}

static int op_jpp(void)			/* JP P,nn */
{
	register unsigned i;

	if (!(F	& S_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
		PC = ram + i;
	} else
		PC += 2;
	return(10);
}

static int op_calz(void)		/* CALL Z,nn */
{
	register unsigned i;

	if (F &	Z_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram);
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		PC = ram + i;
		return(17);
	} else {
		PC += 2;
		return(10);
	}
}

static int op_calnz(void)		/* CALL NZ,nn */
{
	register unsigned i;

	if (!(F	& Z_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram);
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		PC = ram + i;
		return(17);
	} else {
		PC += 2;
		return(10);
	}
}

static int op_calc(void)		/* CALL C,nn */
{
	register unsigned i;

	if (F &	C_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram);
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		PC = ram + i;
		return(17);
	} else {
		PC += 2;
		return(10);
	}
}

static int op_calnc(void)		/* CALL NC,nn */
{
	register unsigned i;

	if (!(F	& C_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		*--STACK = (PC - ram);
		PC = ram + i;
		return(17);
	} else {
		PC += 2;
		return(10);
	}
}

static int op_calpe(void)		/* CALL PE,nn */
{
	register unsigned i;

	if (F &	P_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram);
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		PC = ram + i;
		return(17);
	} else {
		PC += 2;
		return(10);
	}
}

static int op_calpo(void)		/* CALL PO,nn */
{
	register unsigned i;

	if (!(F	& P_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram);
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		PC = ram + i;
		return(17);
	} else {
		PC += 2;
		return(10);
	}
}

static int op_calm(void)		/* CALL M,nn */
{
	register unsigned i;

	if (F &	S_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram);
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		PC = ram + i;
		return(17);
	} else {
		PC += 2;
		return(10);
	}
}

static int op_calp(void)		/* CALL P,nn */
{
	register unsigned i;

	if (!(F	& S_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *PC++;
		i += *PC++ << 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
		if (STACK <= ram)
			STACK =	ram + 65536L;
#endif
		*--STACK = (PC - ram);
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		PC = ram + i;
		return(17);
	} else {
		PC += 2;
		return(10);
	}
}

static int op_retz(void)		/* RET Z */
{
	register unsigned i;

	if (F &	Z_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *STACK++;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		i += *STACK++ << 8;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		PC = ram + i;
		return(11);
	} else {
		return(5);
	}
}

static int op_retnz(void)		/* RET NZ */
{
	register unsigned i;

	if (!(F	& Z_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *STACK++;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		i += *STACK++ << 8;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		PC = ram + i;
		return(11);
	} else {
		return(5);
	}
}

static int op_retc(void)		/* RET C */
{
	register unsigned i;

	if (F &	C_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *STACK++;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		i += *STACK++ << 8;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		PC = ram + i;
		return(11);
	} else {
		return(5);
	}
}

static int op_retnc(void)		/* RET NC */
{
	register unsigned i;

	if (!(F	& C_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *STACK++;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		i += *STACK++ << 8;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		PC = ram + i;
		return(11);
	} else {
		return(5);
	}
}

static int op_retpe(void)		/* RET PE */
{
	register unsigned i;

	if (F &	P_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *STACK++;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		i += *STACK++ << 8;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		PC = ram + i;
		return(11);
	} else {
		return(5);
	}
}

static int op_retpo(void)		/* RET PO */
{
	register unsigned i;

	if (!(F	& P_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *STACK++;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		i += *STACK++ << 8;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		PC = ram + i;
		return(11);
	} else {
		return(5);
	}
}

static int op_retm(void)		/* RET M */
{
	register unsigned i;

	if (F &	S_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *STACK++;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		i += *STACK++ << 8;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		PC = ram + i;
		return(11);
	} else {
		return(5);
	}
}

static int op_retp(void)		/* RET P */
{
	register unsigned i;

	if (!(F	& S_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_STACK | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		i = *STACK++;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		i += *STACK++ << 8;
#ifdef WANT_SPC
		if (STACK >= ram + 65536L)
			STACK =	ram;
#endif
		PC = ram + i;
		return(11);
	} else {
		return(5);
	}
}

static int op_jrz(void)			/* JR Z,n */
{
	if (F &	Z_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		PC += (signed char) *PC + 1;
		return(12);
	} else {
		PC++;
		return(7);
	}
}

static int op_jrnz(void)		/* JR NZ,n */
{
	if (!(F	& Z_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		PC += (signed char) *PC + 1;
		return(12);
	} else {
		PC++;
		return(7);
	}
}

static int op_jrc(void)			/* JR C,n */
{
	if (F &	C_FLAG)	{
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		PC += (signed char) *PC + 1;
		return(12);
	} else {
		PC++;
		return(7);
	}
}

static int op_jrnc(void)		/* JR NC,n */
{
	if (!(F	& C_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_WO | CPU_MEMR;
#endif
#ifdef FRONTPANEL
		fp_sampleLightGroup(0, 0);
#endif
		PC += (signed char) *PC + 1;
		return(12);
	} else {
		PC++;
		return(7);
	}
}

static int op_rst00(void)		/* RST 00 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram);
	PC = ram;
	return(11);
}

static int op_rst08(void)		/* RST 08 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram);
	PC = ram + 0x08;
	return(11);
}

static int op_rst10(void)		/* RST 10 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram);
	PC = ram + 0x10;
	return(11);
}

static int op_rst18(void)		/* RST 18 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram);
	PC = ram + 0x18;
	return(11);
}

static int op_rst20(void)		/* RST 20 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram);
	PC = ram + 0x20;
	return(11);
}

static int op_rst28(void)		/* RST 28 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram);
	PC = ram + 0x28;
	return(11);
}

static int op_rst30(void)		/* RST 30 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram);
	PC = ram + 0x30;
	return(11);
}

static int op_rst38(void)		/* RST 38 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
	fp_sampleLightGroup(0, 0);
#endif
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram) >> 8;
#ifdef WANT_SPC
	if (STACK <= ram)
		STACK =	ram + 65536L;
#endif
	*--STACK = (PC - ram);
	PC = ram + 0x38;
	return(11);
}
