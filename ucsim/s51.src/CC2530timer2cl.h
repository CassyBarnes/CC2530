#ifndef CC2530timer2cl
#define CC2530timer2cl

#include "memcl.h"
#include "uccl.h"

class cl_CC2530_timer2: public cl_hw
{
 protected:

  class cl_memory_cell *cell_t2ctrl;
  class cl_memory_cell *cell_t2evtcfg;
  class cl_memory_cell *cell_t2msel;
  class cl_memory_cell *cell_t2m0;
  class cl_memory_cell *cell_t2m1;
  class cl_memory_cell *cell_ircon;
  class cl_memory_cell *cell_t2irqf;
  class cl_memory_cell *cell_t2movf0;
  class cl_memory_cell *cell_t2movf1;
  class cl_memory_cell *cell_t2movf2;

  char *modes[2];
  bool T2_EVENT1, T2_EVENT2, event, run;
  class cl_address_space *sfr, *xram;
  double CC2530xtal;
  double ticks, TimerTicks, freq;
  int mode;
  char OVF0, OVF1, OVF2; 
  char OVFcap0, OVFcap1, OVFcap2;
  char OVFper0, OVFper1, OVFper2;
  char t2_OVFcmp1_0, t2_OVFcmp1_1, t2_OVFcmp1_2;
  char t2_OVFcmp2_0, t2_OVFcmp2_1, t2_OVFcmp2_2;
  t_mem count;
  short int t2_cap;
  short int t2_per;  
  short int t2_cmp1;
  short int t2_cmp2;

public:

  cl_CC2530_timer2(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);
  virtual void added_to_uc(void);
  virtual double get_rtime(void);
  virtual int do_UpMode(int cycles);
  virtual int do_DeltaMode(int cycles);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem read(class cl_memory_cell *cell);
  virtual int tick(int cycles);
  virtual void overflow(void);
  virtual void CountCompare(void);
  virtual void print_info(class cl_console *con);
  virtual void print_info();

};


/* End of s51.src/CC2530timer2cl.h */

#endif // CC2530timer2cl
