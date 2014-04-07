/*
 * Z80SIM  -  a	Z80-CPU	simulator
 *
 * Copyright (C) 1987-2008 by Udo Munk
 *
 * This is the configuration I'm using for software testing and debugging
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

/*
 *	The following defines may be activated, commented or modified
 *	by user for her/his own purpose.
 */
#define CPU_SPEED 1	/* default CPU speed */
/*#define Z80_UNDOC*/	/* compile undocumented Z80 instructions */
#define WANT_INT	/* activate CPU's interrupts */
#define WANT_SPC	/* activate SP over-/underrun handling 0000<->FFFF */
#define WANT_PCC	/* activate PC overrun handling FFFF->0000 */
#define	CNTL_C		/* cntl-c will stop running emulation */
#define	CNTL_BS		/* cntl-\ will stop running emulation */
#define	WANT_TIM	/* activate runtime measurement */
#define	HISIZE	100	/* number of entrys in history */
#define	SBSIZE	4	/* number of software breakpoints */
/*#define FRONTPANEL*/	/* no frontpanel emulation */
/*#define BUS_8080*/	/* no emulation of 8080 bus status */
#define WANT_COUNTERS // enable counter management
#define INT_DEBUG 1 // prints when an interrupt fires

// this buffer is large for the following reason: in hardware, typing on a console 
// sends each charactor as it happens. however, we are using UDP sockets to emulate
// serial connections, and client apps buffer locally then send a whole line on 'enter'.
#define DART_BUFSIZE 128

/*
 *	The following defines may be modified and activated by
 *	user, to print her/his copyright for a developed system,
 *	which contains the Z80-CPU simulation as a part.
 */

#define	USR_COM	"Jack Carrozzo <jack@crepinc.com>"
#define	USR_REL	"Fixes and updates:"
#define	USR_CPR	"Copyright (C) 2014"


/*
 *	The following lines of this file should not be modified by user
 */
#define	COPYR	"Copyright (C) 1987-2008 by Udo Munk"
#define	RELEASE	"1.17"

#define	LENCMD		80		/* length of command buffers etc */

#define	S_FLAG		128		/* bit definitions of CPU flags */
#define	Z_FLAG		64
#define	N2_FLAG		32
#define	H_FLAG		16
#define	N1_FLAG		8
#define	P_FLAG		4
#define	N_FLAG		2
#define	C_FLAG		1

#define CPU_MEMR	128		/* bit definitions for CPU bus status */
#define CPU_INP		64
#define CPU_M1		32
#define CPU_OUT		16
#define CPU_HLTA	8
#define CPU_STACK	4
#define CPU_WO		2
#define	CPU_INTA	1
					/* operation of simulated CPU */
#define	SINGLE_STEP	3		/* single step */
#define	CONTIN_RUN	1		/* continual run */
#define	STOPPED		0		/* stop CPU because of error */

					/* causes of error */
#define	NONE		0		/* no error */
#define	OPHALT		1		/* HALT	op-code	trap */
#define	IOTRAP		2		/* IN/OUT trap */
#define IOERROR		3		/* fatal I/O error */
#define	OPTRAP1		4		/* illegal 1 byte op-code trap */
#define	OPTRAP2		5		/* illegal 2 byte op-code trap */
#define	OPTRAP4		6		/* illegal 4 byte op-code trap */
#define	USERINT		7		/* user	interrupt */
#define POWEROFF	255		/* CPU off, no error */

					/* type of CPU interrupt */
#define INT_NONE	0
#define	INT_NMI		1		/* non maskable interrupt */
#define	INT_INT		2		/* maskable interrupt */

typedef	unsigned short WORD;		/* 16 bit unsigned */
typedef	unsigned char  BYTE;		/* 8 bit unsigned */

#ifdef HISIZE
struct history {			/* structure of a history entry */
	WORD	h_adr;			/* address of execution */
	WORD	h_af;			/* register AF */
	WORD	h_bc;			/* register BC */
	WORD	h_de;			/* register DE */
	WORD	h_hl;			/* register HL */
	WORD	h_ix;			/* register IX */
	WORD	h_iy;			/* register IY */
	WORD	h_sp;			/* register SP */
};
#endif

#ifdef SBSIZE
struct softbreak {			/* structure of a breakpoint */
	WORD	sb_adr;			/* address of breakpoint */
	BYTE	sb_oldopc;		/* op-code at address of breakpoint */
	int	sb_passcount;		/* pass counter of breakpoint */
	int	sb_pass;		/* no. of pass to break */
};
#endif
