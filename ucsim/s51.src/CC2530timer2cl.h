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
  class cl_memory_cell *cell_clkconcmd;
  class cl_memory_cell *cell_cspt;
  class cl_memory_cell *cell_1;
  class cl_memory_cell *cell_2;

  class cl_memory_cell *cell_3;


  char *modes[2];
  char *events[7];
  int tickspd; 
  int t2msel; 
  int t2movfsel; 
  int evtcfg1; 
  int evtcfg2;
  bool T2_EVENT1; 
  bool T2_EVENT2;
  bool event;
  bool run;
  class cl_address_space *sfr;
  class cl_address_space *xram;
  double ticks;
  double TimerTicks;
  double systemTicks;
  double MemElapsedTime;
  double MemSystemTicks;
  double freq;
  int mode;
  char OVF0; 
  char OVF1; 
  char OVF2; 
  char OVFcap0;
  char OVFcap1; 
  char OVFcap2;
  char OVFper0; 
  char OVFper1;
  char OVFper2;
  char OVFcmp1_0;
  char OVFcmp1_1;
  char OVFcmp1_2;
  char OVFcmp2_0;
  char  OVFcmp2_1;
  char OVFcmp2_2;
  char event1;
  char event2; 
  char noEvent;
  char bm_evtcfg1;
  char bm_evtcfg2;
  t_mem count;
  short int t2_cap;
  short int t2_per;  
  short int t2_cmp1;
  short int t2_cmp2;

public:

  cl_CC2530_timer2(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);
  virtual void reset(void);
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
