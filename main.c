/*
 *	This modul contains the 'main()' function of the simulator,
 *	where the options are checked and variables are initialized.
 *	After initialization of the UNIX interrupts ( int_on() )
 *	and initialization of the I/O simulation ( init_io() )
 *	the user interface ( mon() ) is called.
 */

// changed: help, added getopt, fixed file loading. -jc

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <memory.h>
#include <getopt.h>

#include "config.h"
#include "global.h"

#define BUFSIZE	256		/* buffer size for file I/O */

int load_core(void);
static void save_core(void);
static int load_mos(int, char *), load_hex(char *), load_bin(char *), checksum(char *);
extern void int_on(void), int_off(void), mon(void);
extern void init_io(void), exit_io(void);
extern int exatoi(char *);

void help(char *name) {
#ifndef Z80_UNDOC
	printf("usage:\t%s -s -l -i -mn -fn -xfilename\n",name);
#else
	printf("usage:\t%s -s -l -i -z -mn -fn -xfilename\n",name);
#endif
	puts("\ts = save core and cpu on exit");
	puts("\tl = load core and cpu on start");
	puts("\ti = trap on I/O to unused ports");
#ifdef Z80_UNDOC
	puts("\tz = trap on undocumented Z80 ops");
#endif
	puts("\tm = init memory with n");
	puts("\tf = CPU frequenzy n in MHz");
	puts("\tx = load and execute filename");
	exit(1);
}

int main(int argc, char **argv) {
	register char *s, *p;
	register char *pn = argv[0];

#ifdef CPU_SPEED
	f_flag = CPU_SPEED;
	tmax = CPU_SPEED * 10000;
#endif

	const struct option long_opts[] = {
		{"help", no_argument, NULL, 'h'},
		{"savecore", required_argument, NULL, 's'}, 
		{"loadcore", required_argument, NULL, 'l'},
		{"trapio", no_argument, NULL, 'i'},
#ifdef Z80_UNDOC
		{"trapundoc", no_argument, NULL, 'z'},
#endif
		{"initmem", required_argument, NULL, 'm'},
		{"cpufreq", required_argument, NULL, 'f'},
		{"run", required_argument, NULL, 'x'},
		{NULL,0,NULL,0}
	};

	const char *short_opts = "hs:l:izm:f:x:";
	int option_index=0;
	int c;

	while ((c=getopt_long(argc,argv,short_opts,long_opts,&option_index))!=-1) {
		switch (c) {
			case 'h':
				help(pn);
			case 's':
				printf("Save core set.\n");
				s_flag=1;
				break;
			case 'l':
				printf("load core set.\n");
				l_flag=1;
				break;
			case 'i':
				printf("trap io.\n");
				break;
#ifdef Z80_UNDOC
			case 'z':
				printf("trap undoc.\n");
				z_flag=1;
				break;
#endif
			case 'm':
				m_flag=exatoi(optarg);
				break;
			case 'f':
				f_flag=atoi(optarg);
				tmax=f_flag*10000;
				break;
			case 'x':
				x_flag=1;
				p=xfn;
				s=&optarg[0];
				while (*s) *p++=*s++;
				*p='\0';
				break;
			case '?':
				help(pn);
			default:
				help(pn);
		}
	}

	putchar('\n');
	puts("#######  #####    ###            #####    ###   #     #");
	puts("     #  #     #  #   #          #     #    #    ##   ##");
	puts("    #   #     # #     #         #          #    # # # #");
	puts("   #     #####  #     #  #####   #####     #    #  #  #");
	puts("  #     #     # #     #               #    #    #     #");
	puts(" #      #     #  #   #          #     #    #    #     #");
	puts("#######  #####    ###            #####    ###   #     #");
	printf("\nRelease %s, %s\n",RELEASE,COPYR);
#ifdef USR_COM
	printf("%s %s,%s\n",USR_REL,USR_CPR,USR_COM);
#endif

if (f_flag > 0)
	printf("\nCPU speed is %d MHz\n", f_flag);
	else
	printf("\nCPU speed is unlimited\n");

	fflush(stdout);

	wrk_ram	= PC = ram;
	STACK = ram + 0xffff;
	memset((char *)	ram, m_flag, 65536);
if (l_flag)
	if (load_core()) return(1);
	int_on();
	init_io();
	mon();
	if (s_flag) save_core();
	exit_io();
	int_off();
	return(0);
	}

/*
 *	This function saves the CPU and the memory into the file core.z80
 *
 */
static void save_core(void)
{
	int fd;

	if ((fd	= open("core.z80", O_WRONLY | O_CREAT, 0600)) == -1) {
		puts("can't open file core.z80");
		return;
	}
	write(fd, (char	*) &A, sizeof(A));
	write(fd, (char	*) &F, sizeof(F));
	write(fd, (char	*) &B, sizeof(B));
	write(fd, (char	*) &C, sizeof(C));
	write(fd, (char	*) &D, sizeof(D));
	write(fd, (char	*) &E, sizeof(E));
	write(fd, (char	*) &H, sizeof(H));
	write(fd, (char	*) &L, sizeof(L));
	write(fd, (char	*) &A_,	sizeof(A_));
	write(fd, (char	*) &F_,	sizeof(F_));
	write(fd, (char	*) &B_,	sizeof(B_));
	write(fd, (char	*) &C_,	sizeof(C_));
	write(fd, (char	*) &D_,	sizeof(D_));
	write(fd, (char	*) &E_,	sizeof(E_));
	write(fd, (char	*) &H_,	sizeof(H_));
	write(fd, (char	*) &L_,	sizeof(L_));
	write(fd, (char	*) &I, sizeof(I));
	write(fd, (char	*) &IFF, sizeof(IFF));
	write(fd, (char	*) &R, sizeof(R));
	write(fd, (char	*) &PC,	sizeof(PC));
	write(fd, (char	*) &STACK, sizeof(STACK));
	write(fd, (char	*) &IX,	sizeof(IX));
	write(fd, (char	*) &IY,	sizeof(IY));
	write(fd, (char	*) ram,	65536);
	close(fd);
}

/*
 *	This function loads the CPU and memory from the file core.z80
 *
 */
int load_core(void)
{
	int fd;

	if ((fd	= open("core.z80", O_RDONLY)) == -1) {
		puts("can't open file core.z80");
		return(1);
	}
	read(fd, (char *) &A, sizeof(A));
	read(fd, (char *) &F, sizeof(F));
	read(fd, (char *) &B, sizeof(B));
	read(fd, (char *) &C, sizeof(C));
	read(fd, (char *) &D, sizeof(D));
	read(fd, (char *) &E, sizeof(E));
	read(fd, (char *) &H, sizeof(H));
	read(fd, (char *) &L, sizeof(L));
	read(fd, (char *) &A_, sizeof(A_));
	read(fd, (char *) &F_, sizeof(F_));
	read(fd, (char *) &B_, sizeof(B_));
	read(fd, (char *) &C_, sizeof(C_));
	read(fd, (char *) &D_, sizeof(D_));
	read(fd, (char *) &E_, sizeof(E_));
	read(fd, (char *) &H_, sizeof(H_));
	read(fd, (char *) &L_, sizeof(L_));
	read(fd, (char *) &I, sizeof(I));
	read(fd, (char *) &IFF,	sizeof(IFF));
	read(fd, (char *) &R, sizeof(R));
	read(fd, (char *) &PC, sizeof(PC));
	read(fd, (char *) &STACK, sizeof(STACK));
	read(fd, (char *) &IX, sizeof(IX));
	read(fd, (char *) &IY, sizeof(IY));
	read(fd, (char *) ram, 65536);
	close(fd);

	return(0);
}

/*
 *	Read a file into the memory of the emulated CPU.
 *	The following file formats are supported:
 *
 *		binary images with Mostek header
 *		Intel hex
 */
int load_file(char *s)
{
	char fn[LENCMD];
	BYTE fileb[5];
	register char *pfn = fn;
	int fd;

	while (isspace((int)*s))
		s++;
	while (*s != ',' && *s != '\n' && *s !=	'\0')
		*pfn++ = *s++;
	*pfn = '\0';
	if (strlen(fn) == 0) {
		puts("no input file given");
		return(1);
	}
	if ((fd	= open(fn, O_RDONLY)) == -1) {
		printf("can't open file %s\n", fn);
		return(1);
	}
	if (*s == ',')
		wrk_ram	= ram +	exatoi(++s);
	else
		wrk_ram	= NULL;
	read(fd, (char *) fileb, 5); /*	read first 5 bytes of file */
	if (*fileb == (BYTE) 0xff) {	/* Mostek header ? */
		lseek(fd, 0l, 0);
		printf("Reading as Mostek file.\n");
		return (load_mos(fd, fn));
	}
	else {
		close(fd);
		//printf("Reading as Intel hex file.\n"); // TODO: add flag for this
		//return (load_hex(fn));
		printf("Reading as flat binary file...\n");
		return (load_bin(fn));
	}
}

/*
 *	Loader for binary images with Mostek header.
 *	Format of the first 3 bytes:
 *
 *	0xff ll	lh
 *
 *	ll = load address low
 *	lh = load address high
 */
static int load_mos(int fd, char *fn)
{
	BYTE fileb[3];
	unsigned count,	readed;
	int rc = 0;

	read(fd, (char *) fileb, 3);	/* read load address */
	if (wrk_ram == NULL)		/* and set if not given */
		wrk_ram	= ram +	(fileb[2] * 256	+ fileb[1]);
	count =	ram + 65535 - wrk_ram;
	if ((readed = read(fd, (char *)	wrk_ram, count)) == count) {
		puts("Too much to load, stopped at 0xffff");
		rc = 1;
	}
	close(fd);
	printf("\nLoader statistics for file %s:\n", fn);
	printf("START : %04x\n", (unsigned int)(wrk_ram - ram));
	printf("END   : %04x\n", (unsigned int)(wrk_ram - ram + readed - 1));
	printf("LOADED: %04x\n\n", readed);
	PC = wrk_ram;
	return(rc);
}

/*
 *  Loader for flat binary
 */
static int load_bin(char *fn)
{
	FILE *fd;
	int addr = 0;
	int saddr = 0;
	int eaddr = 0;

	if ((fd = fopen(fn, "rb")) == NULL) {
		printf("can't open file %s\n", fn);
		return(1);
	}

	fseek(fd, 0, SEEK_END);
	unsigned long flen=ftell(fd);
	fseek(fd, 0, SEEK_SET);

	fread(ram+addr,flen,1,fd);
	eaddr=addr+flen;

	fclose(fd);
	printf("\nLoader statistics for file %s:\n", fn);
	printf("START : 0x%04x\n", saddr);
	printf("END   : 0x%04x\n", eaddr);
	printf("LOADED: 0x%04x (%d)\n\n", eaddr - saddr + 1, eaddr - saddr + 1);
	PC = wrk_ram = ram + saddr;

	return(0);
}

/*
 *	Loader for Intel hex
 */
static int load_hex(char *fn)
{
	register int i;
	FILE *fd;
	char buf[BUFSIZE];
	char *s;
	int count = 0;
	int addr = 0;
	int saddr = 0xffff;
	int eaddr = 0;
	int data;

	if ((fd = fopen(fn, "r")) == NULL) {
		printf("can't open file %s\n", fn);
		return(1);
	}

	while (fgets(&buf[0], BUFSIZE, fd) != NULL) {
		s = &buf[0];
		while (isspace(*s))
			s++;
		if (*s != ':')
			continue;
		if (checksum(s + 1) != 0) {
			printf("invalid checksum in hex record: %s\n", s);
			return(1);
		}
		s++;
		count = (*s <= '9') ? (*s - '0') << 4 :
			(*s - 'A' + 10) << 4;
		s++;
		count += (*s <= '9') ? (*s - '0') :
			(*s - 'A' + 10);
		s++;
		if (count == 0)
			break;
		addr = (*s <= '9') ? (*s - '0') << 4 :
			(*s - 'A' + 10) << 4;
		s++;
		addr += (*s <= '9') ? (*s - '0') :
			(*s - 'A' + 10);
		s++;
		addr *= 256;
		addr += (*s <= '9') ? (*s - '0') << 4 :
			(*s - 'A' + 10) << 4;
		s++;
		addr += (*s <= '9') ? (*s - '0') :
			(*s - 'A' + 10);
		s++;
		if (addr < saddr)
			saddr = addr;
		eaddr = addr + count - 1;
		s += 2;
		for (i = 0; i < count; i++) {
			data = (*s <= '9') ? (*s - '0') << 4 :
				(*s - 'A' + 10) << 4;
			s++;
			data += (*s <= '9') ? (*s - '0') :
				(*s - 'A' + 10);
			s++;
			*(ram + addr + i) = data;
		}
	}

	fclose(fd);
	printf("\nLoader statistics for file %s:\n", fn);
	printf("START : %04x\n", saddr);
	printf("END   : %04x\n", eaddr);
	printf("LOADED: %04x\n\n", eaddr - saddr + 1);
	PC = wrk_ram = ram + saddr;

	return(0);
}

/*
 *	Verify checksum of Intel hex records
 */
static int checksum(char *s)
{
	int chk = 0;

	while (*s != '\n') {
		chk += (*s <= '9') ?
			(*s - '0') << 4 :
			(*s - 'A' + 10) << 4;
		s++;
		chk += (*s <= '9') ?
			(*s - '0') :
			(*s - 'A' + 10);
		s++;
	}

	if ((chk & 255) == 0)
		return(0);
	else
		return(1);
}
