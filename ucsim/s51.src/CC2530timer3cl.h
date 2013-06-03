#ifndef CC2530timer3cl
#define CC2530timer3cl

#include "memcl.h"
#include "uccl.h"
#include "CC2530timercl.h"

class cl_CC2530_timer3: public cl_CC2530_timer
{

public:

  cl_CC2530_timer3(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);

  //virtual void added_to_uc(void);
  virtual void reset(void);
  virtual int tick(int cycles);

  //virtual void happen(class cl_hw *where, enum hw_event he, void *params);

};


/* End of s51.src/CC2530timer1cl.h */

#endif // CC2530timer1cl 
