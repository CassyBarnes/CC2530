#include "uc51cl.h"


class cl_CC2530_timer1: public cl_hw
{
protected:
  class cl_memory_cell *cell_t1stat, *cell_t1ctl, *cell_clkconcmd, *cell_tl, *cell_th;
  t_mem mask_M0, mask_M1, mask_TF;
  t_addr addr_tl, addr_th;
  int mode, TR, tickspd, prescale, IOPinChn, IOPinCh0, IOPinCh1, IOPinCh2, IOPinCh3, IOPinCh4, captureRisingEdgen, captureRisingEdge0, captureRisingEdge1, captureRisingEdge2, captureRisingEdge3, captureRisingEdge4, captureFallEdgen, captureFallEdge0, captureFallEdge1, captureFallEdge2, captureFallEdge3, captureFallEdge4;
  double ticks, freq;
  double CC2530xtal;
public:

  cl_CC2530_timer1(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);

  virtual void added_to_uc(void);
  virtual void tickspeed(void);
  virtual void prescaler(void);
  virtual void Capture(int i);
  virtual void Compare(int i);
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
};


/* End of s51.src/CC2530timer1cl.h */
