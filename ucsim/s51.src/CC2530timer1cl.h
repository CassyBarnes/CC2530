#ifndef CC2530timer1cl
#define CC2530timer1cl

#include "memcl.h"
#include "uccl.h"
#include "CC2530timercl.h"

class cl_CC2530_timer1: public cl_CC2530_timer
{
protected:
  /*  class cl_memory_cell *cell_txstat, *cell_txctl, *cell_clkconcmd, *cell_tl, *cell_th;
  t_mem mask_M0, mask_M1, mask_TF, captureMode;
  t_addr addr_tl, addr_th, TxCC0L, TxCC0H;
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
  class cl_address_space *sfr, *xram;*/

public:

  cl_CC2530_timer1(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);

  virtual void reset(void);

  virtual int tick(int cycles);

  //virtual void happen(class cl_hw *where, enum hw_event he, void *params);

};


/* End of s51.src/CC2530timer1cl.h */

#endif // CC2530timer1cl
