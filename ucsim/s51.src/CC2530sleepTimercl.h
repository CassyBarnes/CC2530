#include "memcl.h"
#include "uccl.h"

struct port {
  char pin[8];
};

class cl_CC2530_timer: public cl_hw
{
protected:
  int count;
  class cl_memory_cell  *cell_clkconcmd, *cell_txstat, *cell_txctl, *cell_tl, *cell_th;
  int tickcount;
  int PortNum;
  int PinNum;
  int cmp;
  char *pinPointer;
  int TimerTicks;
  int  tickspd;
  bool captureEnabled;
  char *powerMode;
  double ticks, freq, systemTicks, MemElapsedTime, MemSystemTicks;
  struct port tabPORT[3];
  class cl_address_space *sfr;

public:
  cl_CC2530_sleep_timer(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);
  virtual void added_to_uc(void);
  virtual void reset(void);
  virtual double get_rtime(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual int tick(int cycles);
  virtual int STtick(int cycles);
  virtual void print_info(class cl_console *con);
  virtual void print_info();
};