#ifndef CC2530timer1cl
#define CC2530timer1cl

#include "memcl.h"
#include "uccl.h"

struct channel {
  bool IOPin;
  bool ExIOPin;
  int captureRisingEdge;
  int captureFallEdge;
  t_addr RegCMPL;
  t_addr RegCMPH;
  t_addr RegCTL;
};


class cl_CC2530_timer1: public cl_hw
{
protected:
  class cl_memory_cell *cell_t1stat, *cell_t1ctl, *cell_clkconcmd, *cell_tl, *cell_th;
  t_mem mask_M0, mask_M1, mask_TF, captureMode;
  t_addr addr_tl, addr_th;
  bool up_down, cc, risingEdge;
  int mode;
  int  TR;
  bool capt;
  int ctrl;
  int tickcount;
  int  tickspd;
  int  prescale;
  double ticks, freq;
  double CC2530xtal;
  struct channel tabCh[5];
  class cl_address_space *sfr, *xram;

public:

  cl_CC2530_timer1(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);

  virtual void added_to_uc(void);
  virtual void CaptureCompare(void);
  virtual bool Capture(bool& IOPin, bool& ExIOPin, int captureMode);
  virtual int Compare(int IOPinChn, t_addr ctrlReg, t_addr T1CCnH, t_addr T1CCnL);
  virtual void reset(void);
  virtual double get_rtime(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);

  virtual int tick(int cycles);
  virtual int do_mode0(int cycles);
  virtual int do_mode1(int cycles);
  virtual int do_mode2(int cycles);
  virtual int do_mode3(int cycles);
  virtual void overflow(void);
  virtual void happen(class cl_hw *where, enum hw_event he, void *params);

  virtual void print_info(class cl_console *con);
  virtual void print_info();
};


/* End of s51.src/CC2530timer1cl.h */

#endif // CC2530timer1cl
