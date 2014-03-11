typedef struct {
  char pio;
  char ctc;
  char dart;
  char other;
} t_iodebug;

// i've used BYTE to match the original code here, but 
// unsigned char or uint8_t would work identically

// 82c55-specific (mode 0 only)
typedef struct {
	// last values latched here
	BYTE port_a;
	BYTE port_b;
	BYTE port_c;
	BYTE control;

	// internal states
	BYTE conf_port_a;
	BYTE conf_port_b;
	BYTE conf_port_c_lower;
	BYTE conf_port_c_upper;
} pio_state;

typedef struct {
  BYTE ints_enabled;
  BYTE tc_next;
  BYTE tc;
  BYTE ivector;
  BYTE c_val;
  BYTE prescaler;
  BYTE p_val;
} ctc_state;

typedef struct { // see note in io.c about compatibility
	// conf vars
	BYTE clk_prescale;	// WR4
	BYTE rx_bits; 			// WR3 (bits per char)
	BYTE tx_bits;				// WR5
	BYTE rx_enabled;		// WR5
	BYTE tx_enabled;		// WR3
	BYTE stopbits;      // WR4
	BYTE parity;				// WR4
	BYTE interrupt_mode;// WR1

	BYTE reg_ptr;				// bits 2-0 in WR0

	// runtime vars
	BYTE tx_buf_empty;	// RR0 D2 
	BYTE rx_char_avail;	// RR0 D0 (actually 0-4 in our case: used internally for buf size)
	BYTE all_sent;			// RR1 D0
	BYTE rx_buf_overrun;// RR1 D5 (latched till reset)
	
	// socket things
	struct sockaddr_in ouraddr;
	struct sockaddr_in remaddr;
	socklen_t addrlen;
	int recvlen;
	int sock;
	int have_client;

	// received data is read into rx_buf, then copied into rx_fifo for a number of reasons 
	BYTE rx_buf[DART_BUFSIZE]; 	// the intermediate
	BYTE rx_fifo[DART_BUFSIZE];	// the circular ring buf
	int cbhead,cbtail,cbused;		// ring buffer management

	// status pins 
	// can be read and written via register, but currently
	// are not automatically set from internal conditions. (TODO)
	BYTE rts_;
	BYTE dtr_;
	BYTE cts_;
	BYTE dcd_;
} dart_state;

void init_io(void);
void exit_io(void);

void run_counters(void);

BYTE io_in(BYTE);
void io_out(BYTE,BYTE);
static BYTE io_trap(BYTE);

static BYTE p_8255_in(BYTE);
static void p_8255_out(BYTE,BYTE);

static BYTE p_ctc_in(BYTE);
static void p_ctc_out(BYTE,BYTE);

static BYTE p_dart_in(BYTE);
static void p_dart_out(BYTE,BYTE);
static void dart_reset(dart_state *);

