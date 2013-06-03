#ifndef CC2530timercl
#define CC2530timercl

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


class cl_CC2530_timer: public cl_hw
{
protected:
  class cl_memory_cell *cell_txstat, *cell_txctl, *cell_clkconcmd, *cell_tl, *cell_th;
  t_mem mask_M0, mask_M1, mask_TF, captureMode;
  t_addr addr_tl, addr_th, TxCC0L, TxCC0H;
  bool up_down, cc, risingEdge;
  int mode;
  int ChMax;
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

  cl_CC2530_timer(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);

  virtual void added_to_uc(void);
  virtual void CaptureCompare(void);
  virtual bool Capture(bool& IOPin, bool& ExIOPin, int captureMode);
  virtual int Compare(int IOPinChn, t_addr ctrlReg, t_addr T1CCnH, t_addr T1CCnL);
  virtual void reset(void);
  virtual double get_rtime(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);

  virtual int tick(int cycles);
  virtual int do_Stop(int cycles);
  virtual int do_FreeRunningMode(int cycles);
  virtual int do_ModuloMode(int cycles);
  virtual int do_UpDownMode(int cycles);
  virtual int do_DownMode(int cycles);
  virtual void overflow(void);
  //virtual void happen(class cl_hw *where, enum hw_event he, void *params);

  virtual void print_info(class cl_console *con);
  virtual void print_info();
};

#endif

/* End of s51.src/CC2530timer1cl.h */
