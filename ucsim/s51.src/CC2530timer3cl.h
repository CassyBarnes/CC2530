#ifndef CC2530timer3cl
#define CC2530timer3cl

#include "memcl.h"
#include "uccl.h"
#include "CC2530timercl.h"

class cl_CC2530_timer3: public cl_CC2530_timer<char>
{
 protected:

  class cl_memory_cell *cell_t3cc0;
  class cl_memory_cell *cell_t3cc1;

public:

  cl_CC2530_timer3(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);
  virtual void added_to_uc(void);
  virtual void CaptureCompare(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual void reset(void);
  virtual void TimerTick(int TimerTicks);
  virtual void get_next_cc_event();
  //virtual int tick(int cycles);

};


/* End of s51.src/CC2530timer1cl.h */

#endif // CC2530timer1cl 
