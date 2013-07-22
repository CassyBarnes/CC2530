#ifndef CC2530radiocl
#define CC2530radiocl

#include "memcl.h"
#include "uccl.h"

enum fsm_state {
  IDLE,
  RX_CALIBRATION, 
  SFD_WAIT,
  RX,
  RX_RXWAIT,
  RXFIFO_RESET,
  RX_OVERFLOW,
  TX_CALIBRATION,
  TX,
  TX_FINAL,
  TX_RX_TRANSIT,
  ACK_CALIBRATION,
  ACK,
  ACK_DELAY,
  TX_UNDERFLOW,
  TX_SHUTDOWN
};

struct Frame
{
  int preamble;//4 Bytes
  char SFD;
  char FrameLength;
  short int FCF;
  char DataSequenceNumber;
  char AddressInformation[];
  char Payload[];
  short int FCS;
};

class cl_CC2530_flash_ctrler: public cl_hw
{
 protected:

  class cl_memory_cell *cell_faddrh;
  class cl_memory_cell *cell_faddrl;
  enum fsm_state state;
  char TXFIFO[128];
  char RXFIFO[128];
  double SystemTicks;

public:

  cl_CC2530_flash_ctrler(class cl_uc *auc, int aid, char *aid_string);

};

#endif
