#ifndef CC2530wdt
#define CC2530wdt
#include "memcl.h"
#include "uccl.h"

class cl_CC2530_WDT: public cl_hw
{
 protected:

  class cl_memory_cell *cell_wdctl;
  class cl_memory_cell *cell_clkconcmd;
  class cl_memory_cell *cell_ircon2;
  class cl_address_space *sfr;
  ulong freq;
  ulong CC2530freq;
  ulong systemTicks;
  ulong TimerTicks;
  ulong MemElapsedTime;
  ulong MemSystemTicks;
  short int count;
  int TickINT;
  int mode;
  int interval;
  int clkspd;
  bool enabled;

 public:

  cl_CC2530_WDT(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);
  virtual void added_to_uc(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual ulong get_rtime(void);
  virtual int tick(int cycles);
  virtual void TimerTick(int TimerTicks);
  virtual int overflow(void);
  virtual int do_WDTMode(int cycles);
  virtual int do_TimerMode(int cycles);
  virtual void reset(void);
  //virtual void print_info(class cl_console *con);
  //virtual void print_info(void);
};

#endif
