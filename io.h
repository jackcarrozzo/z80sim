// i've used BYTE to match the original code here, but 
// unsigned char or uint8_t would work identically

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
	BYTE rx_char_avail;	// RR0 D0
	BYTE all_sent;			// RR1 D0
	BYTE rx_buf_overrun;// RR1 D5 (latched till reset)
} dart_state;
