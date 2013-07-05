#ifndef CC2530dmacl
#define CC2530dmacl

#include "memcl.h"
#include "uccl.h"



struct DMAchannel
{
  t_addr source;
  t_addr destination;
  t_addr ITmask;
  t_addr ConfigAddress;
  t_addr s;
  t_addr d;
  bool armed;
  bool idle;
  bool init;
  bool TxEnd;
  bool single;
  bool repeated;
  int TransferCount;
  int VLEN;
  int LEN;
  int Priority;
  int delta;
  int TriggerEvent;
  int SRCincrement;
  int DESTincrement;
  int SRCinc;
  int DESTinc;
  int TxMode;
  int Byte_Word_tx;
  int M8;
};

class cl_CC2530_dma: public cl_hw
{
 protected:

  class cl_memory_cell *cell_dmaarm;
  class cl_memory_cell *cell_dmareq;
  class cl_memory_cell *cell_dmairq;
  bool Req;
  bool ObjInit;
  struct DMAchannel tabDMACh[5];
  class cl_address_space *sfr; 
  class cl_address_space *xram;

public:

  cl_CC2530_dma(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);
  //virtual void added_to_uc(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  //virtual void reset(void);
  virtual int tick(int cycles);
  virtual void transfer(int i);
  virtual bool DMAwrite(int ChannelID);
  virtual void transferTest(int i);
  virtual void transferTest(void);
  virtual void LoadConfig(int i, t_addr CFGaddr);

};

#endif
/* End of s51.src/CC2530dmacl.h */
