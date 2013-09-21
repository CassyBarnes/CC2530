#ifndef CC2530timer1cl
#define CC2530timer1cl

#include "memcl.h"
#include "uccl.h"
#include "CC2530timercl.h"

class cl_CC2530_timer1: public cl_CC2530_timer<short int>
{
 protected:

  class cl_memory_cell *cell_t1cc0l;
  class cl_memory_cell *cell_t1cc0h;
  class cl_memory_cell *cell_t1cc1l;
  class cl_memory_cell *cell_t1cc1h;
  class cl_memory_cell *cell_t1cc2l;
  class cl_memory_cell *cell_t1cc2h;
  class cl_memory_cell *cell_t1cc3l;
  class cl_memory_cell *cell_t1cc3h;
  class cl_memory_cell *cell_t1cc4l;
  class cl_memory_cell *cell_t1cc4h;
  int channelID;
  bool high;

public:

  cl_CC2530_timer1(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);
  virtual void added_to_uc(void);
  virtual void CaptureCompare(void);
  virtual void TimerTick(int TimerTicks);
  virtual void get_next_cc_event();
  virtual void reset(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);

  //virtual void happen(class cl_hw *where, enum hw_event he, void *params);

};


/* End of s51.src/CC2530timer1cl.h */

#endif // CC2530timer1cl
