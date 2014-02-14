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

