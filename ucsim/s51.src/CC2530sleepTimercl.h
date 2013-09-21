#include "memcl.h"
#include "uccl.h"

struct port {
  char pin[8];
};

class cl_CC2530_sleep_timer: public cl_hw
{
protected:
  int count;
  class cl_memory_cell  *cell_clkconcmd, *cell_st0, *cell_st1, *cell_st2, *cell_stcv0, *cell_stcv1, *cell_stcv2, *cell_stload, *cell_stcc;
  int tickcount;
  int PortNum;
  int PinNum;
  int cmp;
  char *pinPointer;
  int TimerTicks;
  int  tickspd;
  bool captureEnabled;
  char *powerMode;
  ulong ticks;
  ulong freq;
  ulong systemTicks;
  ulong MemElapsedTime;
  ulong MemSystemTicks;
  struct port tabPORT[3];
  class cl_address_space *sfr;

public:
  cl_CC2530_sleep_timer(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);
  virtual void added_to_uc(void);
  virtual void reset(void);
  virtual ulong get_rtime(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual int tick(int cycles);
  virtual int STtick(int cycles);
  virtual void print_info(class cl_console *con);
  virtual void print_info();
};
