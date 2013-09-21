#ifndef CC2530radiocl
#define CC2530radiocl

#include "memcl.h"
#include "uccl.h"
#include <stdint.h> 

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
  char FrameLength;
  int FCF;
  int FrameType;
  bool securityEnabled;
  bool FramePending;
  bool AckReq;
  bool PanCompression;
  int FrameVersion;
  char DataSequenceNumber;
  int SrcAddressMode;
  int SourceAddress;
  int SourcePanID;
  int ExtSrcAddressH;
  int ExtSrcAddressL;
  int DestAddressMode;
  int DestinationAddress;
  int DestinationPanID;
  int ExtDestAddressH;
  int ExtDestAddressL;
  char Payload[100];
  short int FCS;
};

struct RadioLink
{
  int frequency;
  int ChNum;
  int LQI;
  bool RSSI_valid;
  bool CCA;
};

class cl_CC2530_radio: public cl_hw
{
 protected:

  class cl_address_space *sfr;
  class cl_address_space *xram;
  class cl_memory_cell *cell_rfirqf0;
  class cl_memory_cell *cell_s1con;
  class cl_memory_cell *cell_rfirqf1;
  class cl_memory_cell *cell_rfd;
  class cl_memory_cell *cell_clkconcmd;
  class cl_memory_cell *cell_frmfilt0;
  class cl_memory_cell *cell_frmfilt1;
  class cl_memory_cell *cell_srcmatch;
  class cl_memory_cell *cell_srcshorten0;
  class cl_memory_cell *cell_srcshorten1;
  class cl_memory_cell *cell_srcshorten2;
  class cl_memory_cell *cell_srcexten0;
  class cl_memory_cell *cell_srcexten1;
  class cl_memory_cell *cell_srcexten2;
  class cl_memory_cell *cell_frmctrl0;
  class cl_memory_cell *cell_frmctrl1;
  class cl_memory_cell *cell_rxenable;
  class cl_memory_cell *cell_rxmaskset;
  class cl_memory_cell *cell_rxmaskclr;
  class cl_memory_cell *cell_freqtune;
  class cl_memory_cell *cell_freqctrl;
  class cl_memory_cell *cell_txpower;
  class cl_memory_cell *cell_txctrl;
  class cl_memory_cell *cell_fsmstat0;
  class cl_memory_cell *cell_fsmstat1;
  class cl_memory_cell *cell_fifopctrl;
  class cl_memory_cell *cell_fsmctrl;
  class cl_memory_cell *cell_ccactrl0;
  class cl_memory_cell *cell_ccactrl1;
  class cl_memory_cell *cell_rssi;
  class cl_memory_cell *cell_rssistat;
  class cl_memory_cell *cell_rxfirst;
  class cl_memory_cell *cell_rxfifocnt;
  class cl_memory_cell *cell_txfifocnt;
  class cl_memory_cell *cell_rxfirst_ptr;
  class cl_memory_cell *cell_rxlast_ptr;
  class cl_memory_cell *cell_rxp1_ptr;
  class cl_memory_cell *cell_txfirst_ptr;
  class cl_memory_cell *cell_txlast_ptr;
  class cl_memory_cell *cell_rfirqm0;
  class cl_memory_cell *cell_rfirqm1;
  class cl_memory_cell *cell_rferrm;
  class cl_memory_cell *cell_monmux;
  class cl_memory_cell *cell_rfrnd;
  class cl_memory_cell *cell_mdmctrl0;
  class cl_memory_cell *cell_mdmctrl1;
  class cl_memory_cell *cell_freqest;
  class cl_memory_cell *cell_rxctrl;
  class cl_memory_cell *cell_fsctrl;
  class cl_memory_cell *cell_fscal1;
  class cl_memory_cell *cell_fscal2;
  class cl_memory_cell *cell_fscal3;
  class cl_memory_cell *cell_agcctrl0;
  class cl_memory_cell *cell_agcctrl1;
  class cl_memory_cell *cell_agcctrl2;
  class cl_memory_cell *cell_agcctrl3;
  class cl_memory_cell *cell_adctest0;
  class cl_memory_cell *cell_adctest1;
  class cl_memory_cell *cell_adctest2;
  class cl_memory_cell *cell_mdmtest0;
  class cl_memory_cell *cell_mdmtest1;
  class cl_memory_cell *cell_dactest0;
  class cl_memory_cell *cell_dactest1;
  class cl_memory_cell *cell_dactest2;
  class cl_memory_cell *cell_atest;
  class cl_memory_cell *cell_ptest0;
  class cl_memory_cell *cell_ptest1;
  class cl_memory_cell *cell_panid_00_0;
  class cl_memory_cell *cell_srcmask0;
  class cl_memory_cell *cell_srcmask1;
  class cl_memory_cell *cell_srcmask2;
  class cl_memory_cell *cell_srcresindex;

  class cl_CC2530_CSP *CSP;
  struct Frame* RXframe;
  struct Frame* TXframe;
  enum fsm_state state;
  unsigned char TXFIFO[128];
  unsigned char RXFIFO[128];
  int ExtAddr[12][2];
  /*uint16_t*/t_mem shortAddr[24];
  uint16_t PanId[24];
  bool slotted_ack;
  bool unslotted_ack;
  bool frame_completed;
  bool overflow;
  bool frame_not_for_me;
  bool Timeout;
  bool TX_ACTIVE;
  bool RX_ACTIVE;
  bool underflow;
  bool frame_sent;
  bool RxEnableWhenTxComplete;
  bool cancelACK;
  bool ACKsent;
  bool autopendEnable;
  double SystemTicks;
  double MemSystemTicks;
  double MemElapsedTime;
  double freq;
  double Timer;
  int RX_PC;
  int TX_PC;
  int TXFifoPosition;
  int length;
  int k;
  int ReadC;
  int tickspd;
  int radio_in;
  int radio_out;
  int stateNum;
  int rxEnable;
  int rxenmask;
  int ShortAddress;
  int PanID;
  int ExtAddressH;
  int ExtAddressL;

public:

  bool MovInstruction;

  cl_CC2530_radio(class cl_uc *auc, int aid, char *aid_string);
  virtual int init(void);
  virtual void reset(void);

  virtual void added_to_uc(void);
  void make_radio_watched_cells(void);
  virtual int tick(int);
  virtual double get_rtime();
  virtual void write(cl_memory_cell*, t_mem*);
  virtual t_mem read(cl_memory_cell*);
  bool filter_frame(void);
  void start_timer(double);
  int get_frequency(int);
  void doRX();
  void Decode_frame();
  void doTX();
  int Dest_match_address();
  int fsm();
  int get_c_frequency();
  void send_ack();
  bool set_ack();
  virtual void happen(cl_hw*, hw_event, void*);
};

class cl_CC2530_CSP: public cl_hw
{
 protected:
  class cl_memory_cell *cell_cspt;
  class cl_memory_cell *cell_cspctrl;
  class cl_memory_cell *cell_cspstat;
  class cl_memory_cell *cell_cspx;
  class cl_memory_cell *cell_cspy;
  class cl_memory_cell *cell_cspz;
  class cl_memory_cell *cell_t2msel;
  class cl_memory_cell *cell_t2m2;
  class cl_memory_cell *cell_t2m1;
  class cl_memory_cell *cell_t2m0;
  class cl_memory_cell *cell_frmctrl1;
  class cl_memory_cell *cell_rxenable;
  class cl_memory_cell *cell_sampledcca;
  class cl_memory_cell *cell_rfst;
  class cl_memory_cell *cell_rfirqf0;
  class cl_memory_cell *cell_s1con;
  class cl_memory_cell *cell_rfirqf1;
  class cl_memory_cell *cell_rfirqm1;
  class cl_address_space *sfr;
  class cl_address_space *xram;

  bool waitingForAnswer;
  bool WaitingForOvfs;
  bool WaitingForWOvfs;
  bool waitWend;
  bool OVFcountdown;
  bool event1;
  bool WaitingForEvent1;
  bool event2;
  bool WaitingForEvent2;
  bool cond;
  bool csp_stop;
  bool RxEnableWhenTxComplete;
 

  int PC;
  int PCwrite;
  int label;

  t_mem S;
  t_mem C;
  t_mem N;
  t_mem X;
  t_mem Y;
  t_mem Z;
  t_mem M;
  t_mem W;

 public:
  cl_CC2530_CSP(class cl_uc *auc, int aid, char *aid_string);

  int CCA;
  int SFD;
  bool STXON;
  bool SRXON;
  bool STXONCCA;
  bool SRFOFF;
  bool SFLUSHRX;
  bool SFLUSHTX;
  bool RSSI_valid;
  bool CPU_ctrl;
  bool pendingBit;
  bool radioAnswer;

  virtual int init(void);
  virtual void reset(void);

  void inst_wait_event1(uchar code); 
  void inst_wait_event2(uchar code); 
  void inst_int(uchar code); 
  void inst_label(uchar code); 
  void inst_waitx(uchar code); 
  void inst_rand_xy(uchar code); 
  void inst_set_cmp1(uchar code); 
  void inst_inc_x(uchar code); 
  void inst_inc_y(uchar code); 
  void inst_inc_z(uchar code); 
  void inst_dec_x(uchar code); 
  void inst_dec_y(uchar code); 
  void inst_dec_z(uchar code); 
  void inst_isclear(uchar code); 
  void inst_snop(uchar code); 
  void inst_stop(uchar code); 
  void inst_srxon(uchar code); 
  void inst_srx_mask_bit_set(uchar code); 
  void inst_srx_mask_bit_clr(uchar code); 
  void inst_sack(uchar code); 
  void inst_sackpend(uchar code); 
  void inst_snack(uchar code); 
  void inst_stxon(uchar code); 
  void inst_stxoncca(uchar code); 
  void inst_ssamplecca(uchar code); 
  void inst_sflushrx(uchar code); 
  void inst_sflushtx(uchar code); 
  void inst_srfoff(uchar code); 
  void inst_isstart(uchar code); 
  void inst_isstop(uchar code); 
  void inst_isrxon(uchar code); 
  void inst_isrx_mask_bit_set(uchar code); 
  void inst_isrx_mask_bit_clr(uchar code); 
  void inst_isack(uchar code); 
  void inst_isackpend(uchar code); 
  void inst_isnack(uchar code); 
  void inst_istxon(uchar code); 
  void inst_istxoncca(uchar code); 
  void inst_issamplecca(uchar code); 
  void inst_isflushrx(uchar code); 
  void inst_isflushtx(uchar code); 
  void inst_isrfoff(uchar code); 
  void inst_skipCS(uchar code);
  void inst_waitw(uchar code);
  void inst_repeatC(uchar code);
  void inst_inc_maxy(uchar code);


  int exec_inst(t_mem code);
  void CSP_tick(void);
  bool condition(int C);
  void IRQ_CSP_STOP(void);
  void IRQ_CSP_WT(void);
  void IRQ_CSP_INT(void);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual void happen(class cl_hw *where, enum hw_event he, void *params);
  void make_csp_watched_cells(void);
};
#endif
