#ifndef CC2530timercl
#define CC2530timercl

#include "memcl.h"
#include "uccl.h"

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
  class cl_memory_cell  *cell_clkconcmd, *cell_txstat, *cell_txctl, *cell_tl, *cell_th;
  t_mem mask_M0, mask_M1, mask_TF, captureMode;
  t_addr addr_tl, addr_th;
  bool up_down, cc, risingEdge;
  int mode;
  int ChMax;
  int  TR;
  bool capt;
  int ctrl;
  int tickcount;
  int TimerTicks;
  int  tickspd;
  int  prescale;
  double ticks, freq, systemTicks, MemElapsedTime, MemSystemTicks;
  double CC2530xtal;
  struct channel<T> tabCh[5];
  class cl_address_space *sfr, *xram;
  char *modes[4];

public:

  cl_CC2530_timer(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);

  virtual void added_to_uc(void);
  virtual void CaptureCompare(void);
  virtual bool Capture(bool& IOPin, bool& ExIOPin, int captureMode);
  virtual bool Compare(bool IOPinChn, t_addr ctrlReg,  T TxCCn);
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
  virtual void refresh_sfr(char count);
  virtual void refresh_sfr(short int count);
  virtual void print_info(class cl_console *con);
  virtual void print_info();
};

#endif

/* End of s51.src/CC2530timer1cl.h */

