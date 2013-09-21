#ifndef CC2530usart
#define CC2530usart
#include "memcl.h"
#include "uccl.h"

#ifndef CC2530xtal
#define CC2530xtal 32000000
#endif

class cl_CC2530_usart: public cl_hw
{
 protected:

  class cl_memory_cell *cell_uXdbuf;
  class cl_memory_cell *cell_uXcsr;
  class cl_memory_cell *cell_uXucr;
  class cl_memory_cell *cell_uXgcr;
  class cl_memory_cell *cell_tcon;
  class cl_memory_cell *cell_ircon2;
  class cl_memory_cell *cell_uXbaud;
  class cl_address_space *sfr;

 public:

  t_addr UXCSR;
  t_addr UXDBUF;
  t_addr UXUCR;
  t_addr UXGCR;
  t_addr UXBAUD;
  t_addr bmURXxIF;
  t_addr bmUTXxIF;
  t_addr bmURXxIE;
  t_addr bmUTXxIE;
  t_mem addressRxISR;
  t_mem addressTxISR;
  t_mem regForTx;
  t_mem regForRx;
  char *messageRx;
  char *messageTx;
  bool TXD;
  bool RXD;
  bool MISO;
  bool MOSI;
  bool SSN;
  bool RxParity;
  bool TxParity;
  bool stopBitLevel;
  bool startBitLevel;
  bool parity;
  int mode;
  int slave;
  int TickCountForBaud;
  int RX;
  int TX;
  int bitRxCnt;
  int bitTxCnt;
  int BitNumber;
  int StopBits;
  int queue;
  int baud_m;
  int baud_e;
  int UsartID;
  double BaudFactor;
  double BaudRate;
  int testTab[11];
  

  cl_CC2530_usart(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);
  virtual void added_to_uc(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem read(class cl_memory_cell *cell);
  virtual int tick(int cycles);
  virtual void reset(void);
  void BaudTick(void);
  bool Shift_in(t_mem& Rxreg, bool& IPin);
  void Shift_out(t_mem& Txreg, bool& OPin);
  bool parity_check(t_mem byte);
  void SCK(void);
  void ExtSCK(void);
  virtual void print_info(class cl_console *con);
  virtual void print_info(void);
};

#endif
