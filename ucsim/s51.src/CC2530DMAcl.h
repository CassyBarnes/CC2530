#ifndef CC2530dmacl
#define CC2530dmacl

#include "memcl.h"
#include "uccl.h"


class cl_CC2530_dma: public cl_hw
{
 protected:

  enum DMAChStates
  {
    DMAChannelIdle,
    LoadDMAChannelConfig,
    DMAChannelArmed,
    TransferOneByteOrWord,//+modify address
    SetInterruptFlag,
    DisarmChannel
  };

  struct DMAchannel
  {
    enum DMAChStates DmaChState;
    t_addr source;
    t_addr destination;
    t_addr ConfigAddress;
    t_addr s;
    t_addr d;
    bool ITmask;
    bool armed;
    bool init;
    bool single;
    bool repeated;
    bool triggered;
    bool Req;
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

  class cl_memory_cell *cell_dmaarm;
  class cl_memory_cell *cell_dmareq;
  class cl_memory_cell *cell_dmairq;
  class cl_memory_cell *cell_ircon;
  bool ObjInit;
  struct DMAchannel tabDMACh[5];
  class cl_address_space *sfr; 
  class cl_address_space *xram;

  void config_channel_addresses();
  void transfer(int i);
  void LoadConfig(int i, t_addr CFGaddr);
  void StateTrans(int i);
  void happen(cl_hw*, hw_event, void*);

public:

  cl_CC2530_dma(class cl_uc *auc, int aid, char *aid_string);
  int init(void);
  virtual void reset(void);
  void channel_init(int i);
  //virtual void added_to_uc(void);
  void write(class cl_memory_cell *cell, t_mem *val);
  //virtual void reset(void);
  int tick(int cycles);
};

#endif
/* End of s51.src/CC2530dmacl.h */
