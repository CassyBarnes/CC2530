#ifndef CC2530timercl
#define CC2530timercl

#include "memcl.h"
#include "uccl.h"

#ifndef CC2530xtal
#define CC2530xtal 32000000
#endif

template <typename T>
struct channel {
  bool IOPin;
  bool ExIOPin;
  t_addr RegCTL;
  t_addr RegCMPL;
  t_addr RegCMPH;
  T ValRegCMP;
};

template <class T>
class cl_CC2530_timer: public cl_hw
{
 protected:
  T count;
  T NextCmpEvent;
  T cmpEventIn;
  class cl_memory_cell *cell_clkconcmd; 
  class cl_memory_cell *cell_txstat; 
  class cl_memory_cell *cell_txctl; 
  class cl_memory_cell *cell_tl; 
  class cl_memory_cell *cell_th; 
  class cl_memory_cell *cell_OvfMaskReg; 
  class cl_memory_cell *cell_OvfFlagReg; 
  class cl_memory_cell *cell_FlagReg; 
  class cl_memory_cell *cell_ircon;
  t_mem  mask_M0;
  t_mem  mask_M1;
  t_mem  mask_TF;
  t_mem  captureMode;
  t_addr addr_tl;
  t_addr  addr_th;
  bool up_down;
  bool cc;
  bool risingEdge;
  bool capt;
  bool PinEvent;
  bool up_down_changed;
  int mode;
  int decal;
  int ChMax;
  int TR;
  int IrconFlag;
  int OVFIFMask;
  int OVFMaskMask;
  int ctrl;
  int tickcount;
  int TimerTicks;
  int  tickspd;
  int  prescale;
  double ticks; 
  double freq;
  double systemTicks;
  double MemElapsedTime;
  double MemSystemTicks;
  struct channel<T> tabCh[5];
  class cl_address_space *sfr; 
  class cl_address_space *xram;
  char *modes[4];

public:

  cl_CC2530_timer(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);

  virtual void added_to_uc(void){}
  virtual void CaptureCompare(void){}
  virtual bool Capture(bool& IOPin, bool& ExIOPin, int captureMode);
  virtual bool Compare(bool IOPinChn, t_addr ctrlReg,  T TxCCn);
  virtual void reset(void);
  virtual double get_rtime(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual int tick(int cycles);
  virtual void TimerTick(int TimerTicks){}
  virtual int do_Stop(int cycles);
  virtual int do_FreeRunningMode(int cycles);
  virtual int do_ModuloMode(int cycles);
  virtual int do_UpDownMode(int cycles);
  virtual void do_DownMode(int cycles);
  virtual void get_next_cc_event(){}
  virtual void overflow(void);
  virtual void refresh_sfr(char count);
  virtual void refresh_sfr(short int count);
  virtual void print_info(class cl_console *con);
  virtual void print_info();
};

#endif

/* End of s51.src/CC2530timer1cl.h */

