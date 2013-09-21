#ifndef CC2530flashctrlcl
#define CC2530flashctrlcl

#include "memcl.h"
#include "uccl.h"

#define PAGE_SIZE 2048
#define FLASHBANK_SIZE 32768

struct Word
{
  int NumberOfWrites;
};

struct flashPage
{
  t_addr StartAddr;
  int NumberOfWrites;
  bool lockbit;
  struct Word WordTab[PAGE_SIZE];//Page size is 2 Kb
};

class cl_CC2530_flash_ctrler: public cl_hw
{
 protected:

  class cl_memory_cell *cell_faddrh;
  class cl_memory_cell *cell_faddrl;
  class cl_memory_cell *cell_fwdata;
  class cl_memory_cell *cell_clkconcmd;
  class cl_memory_cell *cell_fctl;
  class cl_address_space *sfr;
  class cl_address_space *xram;
  class cl_address_space *flash;
  class cl_address_space *flashbank0;
  class cl_address_space *flashbank1;
  class cl_address_space *flashbank2;
  class cl_address_space *flashbank3;
  class cl_address_space *flashbank4;
  class cl_address_space *flashbank5;
  class cl_address_space *flashbank6;
  class cl_address_space *flashbank7;
  struct flashPage PageTab[128];
  double freq;
  double TickCount;
  bool timerEnabled;
  bool writing;
  bool regFull;
  int reg;
  int address;
  int PageNum;
  int WordNum;
  int TXNum;
  t_addr dest;
  t_mem val;

public:

  cl_CC2530_flash_ctrler(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);
  virtual void reset(void);
  double timer(void);
  void erasePage(int pageNumber);
  void ChipErase(void);
  void flashWrite(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual int tick(int cycles);

};

#endif
/* End of s51.src/CC2530flashCtrlercl.h */
