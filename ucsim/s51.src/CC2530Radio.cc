#include <algorithm>
#include <assert.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <math.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "CC2530Radiocl.h"
using namespace std;
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"

#define RADIOTEST

#define PANID_00_0 0x6100

#define SRCMASK0 0x6160
#define SRCMASK1 0x6161
#define SRCMASK2 0x6162
#define SRCMASK 0x6160
#define bmAUTOPEND 2

//FRMFILT1 masks
#define MODIFY_FT_FILTER 0x6
#define ACCEPT_FT0_BEACON 0x8
#define ACCEPT_FT1_DATA 0x10
#define ACCEPT_FT2_ACK 0x20
#define ACCEPT_FT3_MAC_CMD 0x40
#define ACCEPT_FT4TO7_RESERVED 0x80

//FRMFILT0 masks
#define MAX_FRAME_VERSION 0xC
#define PAN_COORDINATOR 0x2
#define FRM_FILTER_EN 0x1


#define DEBUG
#ifdef DEBUG
#define TRACE() \
fprintf(stderr, "%s:%d in %s()\n", __FILE__, __LINE__, __FUNCTION__)
#else
#define TRACE()
#endif

#define bmCSPrunning 0x20
#define bmSetRxenmaskOnTx 1
#define SFD 0x7A
#define bmRX2RX_TIME_OFF 1
#define RSSI_VALUE -1
#define SRCRESINDEX 0x6163
#define SRCSHORTPENDEN0 0x6167 
#define SRCEXTPENDEN0 0x6164 
#define bmPENDING_OR 4

//A correlation value of ~110 indicates a maximum-quality frame, whereas a 
//value of ~50 is typically the lowest-quality frame detectable by the radio
#define CORRELATION_VALUE 100

//FSMSTAT1 masks
#define bmFIFO 0x80
#define bmFIFOP 0x40
#define SFDbm 0x20
#define bmTX_ACTIVE 2
#define bmRX_ACTIVE 1

#define bmRFERRIE 0x01
#define bmRFERRIF 0x02
#define bmRFIE 0x01
#define bmRFIF 0x03 //2 interrupt flags? see page 46 of CC2530 user's guide

//RFIRQF0 (0xE9) RF Interrupt Flags

#define RXMASKZERO 0x80
#define RXPKTDONE 0x40
#define FRAME_ACCEPTED 0x20
#define SRC_MATCH_FOUND 0x10
#define SRC_MATCH_DONE 0x08
#define FIFOP 0x04
#define SFD_IF 0x02
#define ACT_UNUSED 0x01

//RFIRQF1 (0x91) RF Interrupt Flags

#define CSP_WAIT 0x20
#define CSP_STOP 0x10
#define CSP_MANINT 0x08
#define RFIDLE 0x04
#define TXDONE 0x02
#define TXACKDONE 0x01

//RFERRF (0xBF) – RF Error Interrupt Flags

#define STROBEERR 0x40
#define TXUNDERF 0x20
#define TXOVERF 0x10
#define RXUNDERF 0x08
#define RXOVERF 0x04
#define RXABO 0x02
#define NLOCK 0x01

//RFIRQM0 (0x61A3) – RF Interrupt Masks

#define bmRXMASKZERO 0x80
#define bmRXPKTDONE 0x40
#define bmFRAME_ACCEPTED 0x20
#define bmSRC_MATCH_FOUND 0x10
#define bmSRC_MATCH_DONE 0x08
#define FIFOP 0x04
#define bmSFD 0x02
#define bmACT_UNUSED 0x01

//RFIRQM1 (0x61A4 ) – RF Interrupt Masks

#define bmCSP_WAIT 0x20
#define bmCSP_STOP 0x10 
#define bmCSP_MANINT 0x08
#define bmRFIDLE 0x04
#define bmTXDONE 0x02
#define bmTXACKDONE 0x01

//RFERRM (0x61A5) – RF Error Interrupt Masks

#define bmSTROBEERR 0x40
#define bmTXUNDERF 0x20
#define bmTXOVERF 0x10
#define bmRXUNDERF 0x08
#define bmRXOVERF 0x04
#define bmRXABO 0x02
#define bmNLOCK 0x01

//FRMCTRL0 (0x6188) – Frame Handling Masks
#define bmAPPEND_DATA_MODE 0x80
#define bmAUTOCRC 0x40

#define BEACON  0
#define DATA 1
#define ACKNOLEDGEMENT 2
#define MAC_COMMAND 3 
#define RESERVED0 4		 
#define RESERVED1 5 
#define RESERVED2 6 
#define RESERVED3 7

#define bmSLOTTEDACK 2

cl_CC2530_radio::cl_CC2530_radio(class cl_uc *auc, int aid, char *aid_string): cl_hw(auc, HW_CC2530_RADIO, aid, aid_string)
{
  cell_rfirqf0 = (cl_memory_cell *)(NULL);
  cell_s1con = (cl_memory_cell *)(NULL);
  cell_rfirqf1 = (cl_memory_cell *)(NULL);
  cell_rfd = (cl_memory_cell *)(NULL);
  cell_clkconcmd = (cl_memory_cell *)(NULL);
  cell_frmfilt0 = (cl_memory_cell *)(NULL);
  cell_frmfilt1 = (cl_memory_cell *)(NULL);
  cell_srcmatch = (cl_memory_cell *)(NULL);
  cell_srcshorten0 = (cl_memory_cell *)(NULL);
  cell_srcshorten1 = (cl_memory_cell *)(NULL);
  cell_srcshorten2 = (cl_memory_cell *)(NULL);
  cell_srcexten0 = (cl_memory_cell *)(NULL);
  cell_srcexten1 = (cl_memory_cell *)(NULL);
  cell_srcexten2 = (cl_memory_cell *)(NULL);
  cell_frmctrl0 = (cl_memory_cell *)(NULL);
  cell_frmctrl1 = (cl_memory_cell *)(NULL);
  cell_rxenable = (cl_memory_cell *)(NULL);
  cell_rxmaskset = (cl_memory_cell *)(NULL);
  cell_rxmaskclr = (cl_memory_cell *)(NULL);
  cell_freqtune = (cl_memory_cell *)(NULL);
  cell_freqctrl = (cl_memory_cell *)(NULL);
  cell_txpower = (cl_memory_cell *)(NULL);
  cell_txctrl = (cl_memory_cell *)(NULL);
  cell_fsmstat0 = (cl_memory_cell *)(NULL);
  cell_fsmstat1 = (cl_memory_cell *)(NULL);
  cell_fifopctrl = (cl_memory_cell *)(NULL);
  cell_fsmctrl = (cl_memory_cell *)(NULL);
  cell_ccactrl0 = (cl_memory_cell *)(NULL);
  cell_ccactrl1 = (cl_memory_cell *)(NULL);
  cell_rssi = (cl_memory_cell *)(NULL);
  cell_rssistat = (cl_memory_cell *)(NULL);
  cell_rxfirst = (cl_memory_cell *)(NULL);
  cell_rxfifocnt = (cl_memory_cell *)(NULL);
  cell_txfifocnt = (cl_memory_cell *)(NULL);
  cell_rxfirst_ptr =   (cl_memory_cell *)(NULL);
  cell_rxlast_ptr =   (cl_memory_cell *)(NULL);
  cell_rxlast_ptr =    (cl_memory_cell *)(NULL);
  cell_txfirst_ptr =  (cl_memory_cell *)(NULL);
  cell_txlast_ptr =   (cl_memory_cell *)(NULL);
  cell_rfirqm0 = (cl_memory_cell *)(NULL);
  cell_rfirqm1 = (cl_memory_cell *)(NULL);
  cell_rferrm = (cl_memory_cell *)(NULL);
  cell_monmux = (cl_memory_cell *)(NULL);
  cell_rfrnd = (cl_memory_cell *)(NULL);
  cell_mdmctrl0 = (cl_memory_cell *)(NULL);
  cell_mdmctrl1 = (cl_memory_cell *)(NULL);
  cell_freqest = (cl_memory_cell *)(NULL);
  cell_rxctrl = (cl_memory_cell *)(NULL);
  cell_fsctrl = (cl_memory_cell *)(NULL);
  cell_fscal1 = (cl_memory_cell *)(NULL);
  cell_fscal2 = (cl_memory_cell *)(NULL);
  cell_fscal3 = (cl_memory_cell *)(NULL);
  cell_agcctrl0 = (cl_memory_cell *)(NULL);
  cell_agcctrl1 = (cl_memory_cell *)(NULL);
  cell_agcctrl2 = (cl_memory_cell *)(NULL);
  cell_agcctrl3 = (cl_memory_cell *)(NULL);
  cell_adctest0 = (cl_memory_cell *)(NULL);
  cell_adctest1 = (cl_memory_cell *)(NULL);
  cell_adctest2 = (cl_memory_cell *)(NULL);
  cell_mdmtest0 = (cl_memory_cell *)(NULL);
  cell_mdmtest1 = (cl_memory_cell *)(NULL);
  cell_dactest0 = (cl_memory_cell *)(NULL);
  cell_dactest1 = (cl_memory_cell *)(NULL);
  cell_dactest2 = (cl_memory_cell *)(NULL);
  cell_atest = (cl_memory_cell *)(NULL);
  cell_ptest0 = (cl_memory_cell *)(NULL);
  cell_ptest1 = (cl_memory_cell *)(NULL);

  cell_srcmask0 = (cl_memory_cell *)(NULL);
  cell_srcmask1 = (cl_memory_cell *)(NULL);
  cell_srcmask2 = (cl_memory_cell *)(NULL);

  RXframe = NULL;
  TXframe = NULL;

  sfr= uc->address_space(MEM_SFR_ID);
  xram= uc->address_space(MEM_XRAM_ID);
  make_radio_watched_cells();
  CSP = new cl_CC2530_CSP(auc, 1, "CSP");
  reset();
}

int
cl_CC2530_radio::init()
{
  state = IDLE;
  cancelACK = false;
  RXframe = new struct Frame;
  TXframe = new struct Frame;
  slotted_ack = false;
  unslotted_ack = false;
  frame_completed = false;
  overflow = false;
  frame_not_for_me = false;
  Timeout = false;
  TX_ACTIVE = false;
  RX_ACTIVE = false;
  underflow = false;
  frame_sent = false;
  RxEnableWhenTxComplete = false;
  SystemTicks = 0;
  MemSystemTicks = 0;
  MemElapsedTime = 0;
  freq = 32000000;
  Timer = 0;
  RX_PC = 0;
  TX_PC = 0;
  TXFifoPosition = 0;
  length = 0;
  k = 0;
  ReadC = 0;
  tickspd = 0;
  radio_in = 0;
  radio_out = 0;
  stateNum = 0;
  rxEnable = 0;
  rxenmask = 0;
  ShortAddress = 0;
  PanID = 0;
  ExtAddressH = 0;
  ExtAddressL = 0;
  MovInstruction = false;

  for (int i = 0; i<128; i++)
    {
      RXFIFO[i] = xram->read(0x6000 + i);
      TXFIFO[i] = xram->read(0x6080 + i);
      TXFifoPosition = 0;
      srand(1);
    }

  ShortAddress = (xram->read(0x6175)<<8)+ xram->read(0x6174);
  PanID = (xram->read(0x6173)<<8)+ xram->read(0x6172);
  ExtAddressH = (xram->read(0x6171)<<24) + (xram->read(0x6170)<<16) +
    (xram->read(0x616F)<<8)+ xram->read(0x616E);
  ExtAddressL = (xram->read(0x616D)<<24) + (xram->read(0x616C)<<16) +
    (xram->read(0x616B)<<8)+ xram->read(0x616A);
  for (int i = 0; i< 12; i++)
    {
      ExtAddr[i][0] = 0xCCCCCCCC;
      ExtAddr[i][1] = 0xCCCCCCCC;
    }
  for (int i = 0; i< 24; i++)
    {
      shortAddr[i] = 0xCCCC;
      PanId[i] = 0xCCCC;
    }
#ifdef RADIOTEST
  RXFIFO[0] = 0x40;//length = 64 bytes
  RXFIFO[1] = 0x88;//src and dest address mode is short @
  RXFIFO[2] = 0x01;//data frame
  RXFIFO[3] = 0;//sequence number
  RXFIFO[4] = 0x3D;//dest pan
  RXFIFO[5] = 0x0B;
  RXFIFO[6] = 0x71;//dest short @
  RXFIFO[7] = 0xA8;
  RXFIFO[8] = 0x7D;//src PAN
  RXFIFO[9] = 0x83;
  RXFIFO[10] = 0x64;//src short @
  RXFIFO[11] = 0x50;
  for (int i = 12; i<128; i++)
    RXFIFO[i] = 0x0C;//payload

  // Decode_frame();used for test

#endif
  return(resGO);
}

int
cl_CC2530_radio::tick(int cycles)
{
  SystemTicks++;
  if (Timer>0)
    Timer--;//decrement timer
  fprintf(stderr,"Radio tick. FSM state %d\tTimer countdouwn: %g\n", stateNum, Timer);
  if (Timer == 0)
    Timeout = true;
  fsm();
  CSP->CSP_tick();
  Timeout = false;//timeout reset if not reset in CSP_tick()
  if (stateNum == 3)
    radio_in = SFD;
  else 
    radio_in = 0;
  return(resGO);
}

void 
cl_CC2530_radio::reset(void)
{
  init();
  //see reset values of registers in radio registers description of CC2530 users guide
  cell_frmfilt0->set(0x0D);
  cell_frmfilt1->set(0x78);
  cell_srcmatch->set(0x03);
  cell_srcshorten0->set(0);
  cell_srcshorten1->set(0);
  cell_srcshorten2->set(0);
  cell_srcexten0->set(0);
  cell_srcexten1->set(0);
  cell_srcexten2->set(0);
  cell_frmctrl0->set(0x40);
  cell_frmctrl1->set(0x1);
  cell_rxenable->set(0);
  cell_rxmaskset->set(0);
  cell_rxmaskclr->set(0);
  cell_freqtune->set(0x0F);
  cell_freqctrl->set(0x0B);
  cell_txpower->set(0xF5);
  cell_txctrl->set(0x69);
  cell_fsmstat0->set(0);
  cell_fsmstat1->set(0);
  cell_fifopctrl->set(0x40);
  cell_fsmctrl->set(0x1);
  cell_ccactrl0->set(0xE0);
  cell_ccactrl1->set(0x1A);
  cell_rssi->set(RSSI_VALUE);
  cell_rssistat->set(1);//setting rssi valid
  cell_rxfirst->set(0);
  cell_rxfifocnt->set(0);
  cell_txfifocnt->set(0);
  cell_rxfirst_ptr->set(0);
  cell_rxlast_ptr->set(0);
  cell_rxp1_ptr->set(0);
  cell_txfirst_ptr->set(0);
  cell_txlast_ptr->set(0);
  cell_rfirqm0->set(0);
  cell_rfirqm1->set(0);
  cell_rferrm->set(0);
  cell_monmux->set(0);
  cell_rfrnd->set(0);
  cell_mdmctrl0->set(0x85);
  cell_mdmctrl1->set(0x14);
  cell_freqest->set(0);
  cell_rxctrl->set(0x3F);
  cell_fsctrl->set(0x5A);
  cell_fscal1->set(0x2B);
  cell_fscal2->set(0x20);
  cell_fscal3->set(0x2A);
  cell_agcctrl0->set(0x5F);
  cell_agcctrl1->set(0x11);
  cell_agcctrl2->set(0);
  cell_agcctrl3->set(0x2E);
  cell_adctest0->set(0x10);
  cell_adctest1->set(0x0E);
  cell_adctest2->set(0x3);
  cell_mdmtest0->set(0x75);
  cell_mdmtest1->set(0x8);
  cell_dactest0->set(0);
  cell_dactest1->set(0);
  cell_dactest2->set(0x28);
  cell_atest->set(0);
  cell_ptest0->set(0);
  cell_ptest1->set(0);
  cell_srcmask0->set(0);
  cell_srcmask1->set(0);
  cell_srcmask2->set(0);
  autopendEnable = false;
}

void
cl_CC2530_radio::added_to_uc(void)
{
  //Timer interrupt
  uc->it_sources->add(new cl_it_src(IEN0, bmRFERRIE, TCON, bmRFERRIF, 0x0003, true,
				    "radio error interrupt", 4));
  uc->it_sources->add(new cl_it_src(IEN2, bmRFIE, S1CON, bmRFIF, 0x0083, true,
				    "radio interrupt", 4));
}

double
cl_CC2530_radio::get_rtime(void)
{
  return(MemElapsedTime + SystemTicks/freq);
}

void
cl_CC2530_radio::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_clkconcmd)
    {
      tickspd= 1<<((*val & bmTickSpd) >> 3);
      MemElapsedTime = get_rtime();
      MemSystemTicks = SystemTicks;
      SystemTicks=0;
      freq= CC2530xtal/(tickspd);
      fprintf(stderr,"Modification of CLKCONCMD.\n");

    }
  if (cell == cell_rfd)
    {
      TXFIFO[TXFifoPosition] = *val;
      TXFifoPosition++;
      if (TXFifoPosition >= 128)
	TXFifoPosition = 0;
    }
  int CellAddrInRange = xram->IsWithinRange(cell);
  if (CellAddrInRange != 0)
    {
      int cellNumber = (CellAddrInRange - 0x6100);
      int SourceAddrTableIDX = cellNumber  >> 2;//cell Num divided by 4
      
      //update short @ & PanId @
      switch(cellNumber - (SourceAddrTableIDX<<2)) //<=>(cellNumber % 4)
	{
	case 0: 
	  PanId[SourceAddrTableIDX] = (PanId[SourceAddrTableIDX] & 0xFF00) 
	    + (*val & 0xFF);
	  break;
	case 1:
	  PanId[SourceAddrTableIDX] = (PanId[SourceAddrTableIDX] & 0x00FF) 
	    + ((*val & 0xFF)<<8);
	  break;
	case 2:
	  shortAddr[SourceAddrTableIDX] = (shortAddr[SourceAddrTableIDX] & 0xFF00) 
	    + (*val & 0xFF);
	  break;
	case 3:
	  shortAddr[SourceAddrTableIDX] = (shortAddr[SourceAddrTableIDX] & 0x00FF) 
	    + ((*val & 0xFF)<<8);
	  break;
	default:
	  assert(false);
	  break;
	}

      //update Ext

      SourceAddrTableIDX = SourceAddrTableIDX >> 1; //Divide again by 2 int division by 8)
      int j = cellNumber - (SourceAddrTableIDX<<3);
      t_mem n = 0;
      if (j>= 4)
	{
	  n = 1;
	  j -= 4;
	}

      ExtAddr[SourceAddrTableIDX][n] = 
	(ExtAddr[SourceAddrTableIDX][n] & ((0xFF<<(j<<3))^0xFFFFFFFF)) 
	|(*val<<(j<<3));

      fprintf( stderr, "Short@: 0x%04x\nPanId: 0x%04x\nExt@: 0x%08x%08x\n", 
	       shortAddr[SourceAddrTableIDX<<1], 
	       PanId[SourceAddrTableIDX<<1], 
	       ExtAddr[SourceAddrTableIDX][1],
	       ExtAddr[SourceAddrTableIDX][0]);
    }

#ifdef RADIOTEST // remove me - for bug investigation
  if (CellAddrInRange == 0x6107)
    {
      TRACE();
      //doRX();
    }
#endif
}

t_mem
cl_CC2530_radio::read(class cl_memory_cell *cell)
{
  if (cell == cell_rfd)
    {
      t_mem temp = RXFIFO[ReadC];
      //FIFOP is set low again when a byte is read from the RXFIFO and this leaves
      //FIFOP_THR bytes in the FIFO.
      if (RXframe->FrameLength - ReadC == cell_fifopctrl->get())
	{
	  cell_fsmstat1->set_bit0(bmFIFOP);
	}

      if (ReadC < RXframe->FrameLength)
	{
	  //DMA trigger when data is read from the RXFIFO (through RFD)
	  // and there is still more data available in the RXFIFO
	  inform_partners(EV_RADIO, 0);
	}
      else //No more data in the fifo
	{
	  cell_fsmstat1->set_bit0(bmFIFO);
	}

      if (MovInstruction)
	ReadC++;

      return(temp);  
    }
  return(cell->get());
}

void
cl_CC2530_radio::start_timer(double time)//unit is µs
{
  double cycleTime = 1/freq;
  Timer = (time/cycleTime)/1000000;
}

int
cl_CC2530_radio::get_frequency(int channelNumber)//returns channel frequency
{
  k = channelNumber;
  return(2405 + 5*(k - 11));
}

int
cl_CC2530_radio::get_c_frequency(void)//returns carrier frequency
{
  return(2394 + (cell_freqctrl->get() & 0x7F));//in MHz
}

void
cl_CC2530_radio::doRX(void)
{

  RXFIFO[RX_PC] = radio_in;//Receives 1 Byte from radio input

  //Register contains rxfifo count
  cell_rxfifocnt->set(RX_PC);

  if (RX_PC == 0)
    {
      //first Byte represents frame length of MDPU
      //first byte present in RXFIFO triggers the DMA
      cell_rxfirst->set(radio_in & 0x7F);//register contains first byte of rxfifo
      cell_fsmstat1->set_bit1(bmFIFO);
      inform_partners(EV_RADIO, 0);
      length = radio_in & 0x7F;
    }
  //The FSMSTAT1.FIFOP signal goes high when:
  // The number of valid bytes in the RXFIFO exceeds the FIFOP threshold value
  if ((RX_PC == cell_fifopctrl->get())&&((cell_frmfilt0->get() & FRM_FILTER_EN) == 0))
    cell_fsmstat1->set_bit1(bmFIFOP);

  if (RX_PC == length)
    {
      //last byte received -> set fifop, sfd stat goes low
      cell_fsmstat1->set_bit1(bmFIFOP);
      cell_fsmstat1->set_bit0(SFDbm);

      //frame received interrupt
      if ((cell_rfirqm0->get() & bmRXPKTDONE) != 0) //interrupt enable
	{
	  cell_rfirqf0->set_bit1(RXPKTDONE); 
	  cell_s1con->set_bit1(bmRFIF);
	}
      Decode_frame();
      //frame received
      if (!Dest_match_address())
	frame_not_for_me = true;
      else if (RX_PC > 127)
	overflow = true;
      else if ((cell_frmfilt0->get() & FRM_FILTER_EN) != 0)//if frame filter enabled
	{
	  bool accept_frame = filter_frame();
	  //frame accepted interrupt
	  if (accept_frame && 
	      ((cell_rfirqm0->get() & bmFRAME_ACCEPTED) != 0)) //interrupt enable
	    {
	      cell_rfirqf0->set_bit1(FRAME_ACCEPTED); 
	      cell_s1con->set_bit1(bmRFIF);
	    }
	  if ((RX_PC == cell_fifopctrl->get()))
	    cell_fsmstat1->set_bit1(bmFIFOP);

	  if (!accept_frame)
	    {
	      cell_fsmstat1->set_bit0(SFDbm | bmFIFO | bmFIFOP);
	    }

	}

      frame_completed = true;
	
      if (RXframe->AckReq)//If ack requested check if it is slotted or not
	{
	  slotted_ack = (cell_fsmctrl->get() & bmSLOTTEDACK) != 0;
	  unslotted_ack = !slotted_ack;
	}

      //If autocrc is 1 The CRC-16 is checked in hardware, and replaced in the  
      //RXFIFO by a 16-bit status word which contains a CRC OK bit. The status 
      //word is controllable through APPEND_DATA_MODE.
      if ((cell_frmctrl0->get() & bmAUTOCRC) != 0)
	{
	  if ((cell_frmctrl0->get() & bmAPPEND_DATA_MODE) == 0)
	    {
	      RXFIFO[RX_PC - 1] = 0x80 | CORRELATION_VALUE;
	      RXFIFO[RX_PC - 2] = RSSI_VALUE;
	    }
	  else
	    {
	      RXFIFO[RX_PC - 1] = 0x80 | cell_srcresindex->get();
	      RXFIFO[RX_PC - 2] = RSSI_VALUE;
	    }
	}
      RX_PC = 0;
    }
  else
    RX_PC++;
}

void
cl_CC2530_radio::Decode_frame(void)
{
  RXframe->FrameLength = RXFIFO[0];
  fprintf(stderr, "Frame length: %d\n", RXframe->FrameLength);
  RXframe->FCF = ((RXFIFO[1] & 0xFF)<<8)+(RXFIFO[2] & 0xFF);
  fprintf(stderr, "Frame Control Field: 0x%02x\n", RXframe->FCF);
  RXframe->FrameType = RXframe->FCF & 7;
  char* FrameTyp;
  switch(RXframe->FrameType)
    {
    case 0 : FrameTyp = "Beacon"; break;
    case 1 :   FrameTyp = "Data"; break;
    case 2 :   FrameTyp = "Acknolegnement"; break;
    case 3 :   FrameTyp = "Mac Command"; break;
    default :  FrameTyp = "Reserved";
    }
  fprintf(stderr, "Frame type: %s\n", FrameTyp);
  RXframe->securityEnabled = (RXframe->FCF & 8) != 0;
  fprintf(stderr, "Security Enabled: %s\n", RXframe->securityEnabled?"Yes":"No");
  RXframe->FramePending = (RXframe->FCF & 10) != 0;
fprintf(stderr, "Frame Pending: %s\n", RXframe->FramePending?"Yes":"No");
  RXframe->AckReq = (RXframe->FCF & 0x20) != 0;
  fprintf(stderr, "Acknoledge Requested: %s\n", RXframe->AckReq?"Yes":"No");
  RXframe->PanCompression = (RXframe->FCF & 0x40) != 0;
  fprintf(stderr, "Pan compression: %s\n", RXframe->PanCompression?"Yes":"No");
  RXframe->DestAddressMode = (RXframe->FCF >> 0xA) & 3;
  fprintf(stderr, "Destination addressing mode: Short address\n");
  RXframe->SrcAddressMode = (RXframe->FCF  >> 0xE) & 3;
  fprintf(stderr, "Source addressing mode: Short address\n");
  RXframe->SourceAddress = 0;//Making sure Source address is 0 before calculation 
  RXframe->ExtSrcAddressH = 0;
  RXframe->ExtSrcAddressL = 0;
  bool match = false;
    
   if (RXframe->SrcAddressMode != 0)
    {
      //get source address
      //Need to know addressing modes to get position of src @ in RXFIFO
      if (RXframe->SrcAddressMode == 2)//frame contains short src @
	{
	  if (RXframe->DestAddressMode == 2)//frame contains short dest @
	    {//10 =1(Length)+ 2(FC) + 1 (seq#) + 2 (destPanID)+ 2 (dest@) +2 (srcPan)
	      RXframe->SourceAddress = (RXFIFO[10]<<8) + RXFIFO[11];//checkbytesorder!
	      RXframe->SourcePanID = (RXFIFO[8]<<8) + RXFIFO[9];
	    }
	  if (RXframe->DestAddressMode == 3)//frame contains extended dest @
	    {//16 =1(Length)+ 2(FC) + 1 (seq#) + 2 (destPanID)+ 8 (dest@) +2 (srcPan)
	      RXframe->SourceAddress = (RXFIFO[16]<<8) + RXFIFO[17];//checkbytesorder!
	      RXframe->SourcePanID = (RXFIFO[14]<<8) + RXFIFO[15];
	    }
	  fprintf(stderr, "Source address: 0x%04x\n", RXframe->SourceAddress);
	  fprintf(stderr, "Source Pan ID: 0x%04x\n", RXframe->SourcePanID);
	}
      if (RXframe->SrcAddressMode == 3)//frame contains extended src @
	{
	  if (RXframe->DestAddressMode == 2)//frame contains short dest @
	    {//10 = 1 (Length) + 2 (FC) + 1 (seq#) + 2(destPanID)+ 2(dest@) +2(srcPan)
	      for (int i = 0; i<4; i++)
		{
		  RXframe->ExtSrcAddressH += RXFIFO[10+i]<<((4-i)<<3);
		  RXframe->ExtSrcAddressL += RXFIFO[14+i]<<((4-i)<<3);
		}
	    }	
	  if (RXframe->DestAddressMode == 3)//frame contains extended dest @
	    {//16 = 1 (Length) + 2 (FC) + 1 (seq#) + 2 (destPanID)+ 8 (dest@) +2 (srcPan)
	      for (int i = 0; i<4; i++)
		{
		  RXframe->ExtSrcAddressH += (RXFIFO[16+i]<<((4-i)<<3));
		  RXframe->ExtSrcAddressL += (RXFIFO[20+i]<<((4-i)<<3));
		}
	    }
	  fprintf(stderr, "Source address: 0x%04x%04x\n", RXframe->ExtSrcAddressH, RXframe->ExtSrcAddressL);
	}

      //Compare source @ with those in table
      int maskRegNum = 0;
      int PositionInReg = 0;
      for (int i = 0; i<12; i++)
	{
	  if ((RXframe->ExtSrcAddressH == ExtAddr[i][1]) 
	      && (RXframe->ExtSrcAddressL == ExtAddr[i][0]))
	    { 
	      maskRegNum = i/4;
	      PositionInReg = (i%4)<<1;

	      //EXT_ADDR_EN that enables/disables source address matching for each 
	      //of the 12 extended address table entries
	      if ((xram->get(SRCEXTEN0 + maskRegNum) & 3<<PositionInReg) != 0)
		{
		  match = true;
		  //SRCEXTPENDEN enables/disables automatic pending for each 
		  //of the 12 extended addresses
		  autopendEnable = 
		    (xram->get(SRCEXTPENDEN0 + maskRegNum) & 1<<PositionInReg) != 0;
		  //SRCRESMASK indicates source address match for each individual 
		  //entry in the source address table
		  xram->write(SRCMASK+maskRegNum, 3<<PositionInReg); 
		  cell_srcresindex->set(0x20 | i<<1);
		}
	    }
	}
      shortAddr[0]= 0x6450;
      PanId[0]= 0x7D83;
      for (int k = 0; k<24; k++)
	{
	  //fprintf(stderr, "i: %d\tshort: 0x%04x\t Pan: 0x%04x\n", k, shortAddr[k],PanId[k]); 
	  if ((RXframe->SourceAddress == shortAddr[k]) 
	      && (RXframe->SourcePanID == PanId[k]))
	    { 
	      maskRegNum = k/8;
	      PositionInReg = k%8;

	      //SHORT_ADDR_EN enables/disables source address matching for 
	      //each of the 24 short address table entries
	      //	      if((xram->get(SRCSHORTEN0+maskRegNum) & 1<<PositionInReg) != 0)
	      //{
		  match = true;
		  //SRCSHORTPENDEN enables/disables automatic pending for each 
		  //of the 12 extended addresses
		  autopendEnable = 
		    (xram->get(SRCSHORTPENDEN0 + maskRegNum) & 1<<PositionInReg) != 0;
		  //SRCRESMASK indicates source address match for each individual 
		  //entry in the source address table
		  xram->write(SRCMASK+maskRegNum, 1<<PositionInReg);
		  cell_srcresindex->set(k);
		  //	}
 
	    }
	}

      //SRCRESINDEX bit 6 is 1 when the conditions for automatic pending 
      //bit in acknowledgment have been met
      if (autopendEnable && ((cell_srcmatch->get() & bmAUTOPEND)!=0))
	cell_srcresindex->set(cell_srcresindex->get() | 0x40);

      //if Matching source address found
      if (match)
	{
	  fprintf(stderr, "Source Address Match Found!\n");
	  if ((cell_rfirqm0->get() & bmSRC_MATCH_FOUND) != 0) //interrupt enable
	    {
	      TRACE();
	      cell_rfirqf0->set_bit1(SRC_MATCH_FOUND); 
	      cell_s1con->set_bit1(bmRFIF);
	    }
	}
      else
	cell_srcresindex->set(0x3F);
      fprintf(stderr, "Result Register: 0x%02x\n", cell_srcresindex->get());
    }

   if (RXframe->DestAddressMode != 0)
     {
       //get Destination address
       //Need to know addressing modes to get position of src @ in RXFIFO
       if (RXframe->DestAddressMode == 2)//frame contains short dest @
	 {
	   //3 =1(Length)+ 2(FC) + 1 (seq#)
	   RXframe->DestinationPanID = (RXFIFO[4]<<8) + RXFIFO[5];
	   //5 =1(Length)+ 2(FC) + 1 (seq#) + 2(Dest short @)
	   RXframe->DestinationAddress = (RXFIFO[6]<<8) + RXFIFO[7];
	  fprintf(stderr, "Destination address: 0x%04x... Match!\n", RXframe->DestinationAddress);
	  fprintf(stderr, "Destination Pan ID: 0x%04x... Match!\n", RXframe->DestinationPanID);
	 }
       if (RXframe->DestAddressMode == 3)//frame contains extended dest @
	 {
	   //3 =1(Length)+ 2(FC)
	   for (int i = 0; i<4; i++)
	     {
	       RXframe->ExtDestAddressH += RXFIFO[4+i]<<((4-i)<<3);
	       RXframe->ExtDestAddressL += RXFIFO[8+i]<<((4-i)<<3);
	     }
	 }
     }

   if ((cell_rfirqm0->get() & bmSRC_MATCH_DONE) != 0) //interrupt enable
     {
       cell_rfirqf0->set_bit1(SRC_MATCH_DONE); 
       cell_s1con->set_bit1(bmRFIF);
     }
   if(filter_frame())
     fprintf(stderr, "Frame has passed the filter without being rejected.\n");
   else
     fprintf(stderr, "Frame has been rejected by the filter.\n");
}

bool
cl_CC2530_radio::filter_frame(void)
{
  RXframe->FrameLength = RXframe->FrameLength & 0x7F;

  //If FRMFILT1.MODIFY_FT_FILTER is not equal to zero, the MSB of the frame 
  //type subfield of the FCF is either inverted or forced to 0 or 1.
  switch(cell_frmfilt1->get() & MODIFY_FT_FILTER)
    {
    case 0: 
      RXframe->FrameType = (RXframe->FCF & 7); 
      break;
    case 1: 
      RXframe->FrameType = (RXframe->FCF & 7)^4;
      break;
    case 2:
      RXframe->FrameType = (RXframe->FCF & 3);
      break;
    case 3:
      RXframe->FrameType = (RXframe->FCF & 3) | 4;
      break;
    default:
      RXframe->FrameType = (RXframe->FCF & 7); 
      break;
    }

    
  //The length byte must be equal to or higher than the minimum frame 
  //length, which is derived from the source- and destination-address 
  //mode and PAN ID compression subfields of the FCF.
  int MinimumLength = 5;//5 = 2 (frame ctrl) + 1 (seq #) + 2 (FCS)
  bool CheckSrcPan = false;
  bool CheckDestPan = false;
  switch(RXframe->SrcAddressMode)
    {
    case 2 :
      MinimumLength += 2; //2 extra bytes in minimum length (short address)
      CheckSrcPan = true;
      break;
    case 3 :
      MinimumLength += 8; //8 extra bytes in minimum length (Ext address)
      CheckSrcPan = true;
      break;
    case 0 : case 1 : default : //No extra bytes for source address
      break;
    }

  switch(RXframe->DestAddressMode)
    {
    case 2 :
      MinimumLength += 2; //2 extra bytes in minimum length (short address)
      CheckDestPan = true;
      break;
    case 3 :
      MinimumLength += 8; //8 extra bytes in minimum length (Ext address)
      CheckDestPan = true;
      break;
    case 0 : case 1 : default : //No extra bytes for destination address
      break;
    }

  if (CheckDestPan != CheckSrcPan)
    //<=>logical xor, if true, only one pan address included
    {
	MinimumLength += 2;
    }

  if (CheckDestPan && CheckSrcPan)//Frame might contain one or both Pan addresses
    {
      if (RXframe->PanCompression)
	MinimumLength += 2; //Only the Dest Pan is included
      else
	MinimumLength += 4; //Both Pan addresses are included
    }

  if (RXframe->FrameLength < MinimumLength)
    return(false);
    

  //The reserved FCF bits [9:7] ANDed together with 
  //FRMFILT0.FCF_RESERVED_BITMASK must equal 0.
  if (((((RXframe->FCF)>>7) & 7) & ((cell_frmfilt0->get()>>4) & 7)) != 0)
    return(false);

  //The value of the frame version subfield of the FCF cannot be 
  //higher than FRMFILT0.MAX_FRAME_VERSION. -- IGNORED FOR SIMULATOR
  //All frames are supposed from same version 

  //The source and destination address modes cannot be reserved values
  if ((RXframe->SrcAddressMode == 1) || (RXframe->DestAddressMode == 1))
    return(false);

  //If a destination PAN ID is included in the frame, it must match 
  //PAN_ID or must be the broadcast PAN identifier (0xFFFF).
  if ((RXframe->DestAddressMode) != 0)
    {
      if ((RXframe->DestinationPanID != PanID) 
	  && (RXframe->DestinationPanID != 0xFFFF))
	return(false);
    }

  //If a short destination address is included in the frame, it 
  //must match either SHORT_ADDR or the broadcast address (0xFFFF).
  if ((RXframe->DestAddressMode) == 2)
    {
      if ((RXframe->DestinationAddress != ShortAddress) 
	  && (RXframe->DestinationAddress != 0xFFFF))
	return(false);
    }

  //If an extended destination address is included in the frame, *
  //it must match EXT_ADDR
  if (((RXframe->DestAddressMode) == 3) 
      && ((RXframe->ExtDestAddressH != ExtAddressH)
	  && (RXframe->ExtDestAddressL != ExtAddressL)) )
    return(false);

  if (RXframe->FrameType == BEACON)
    {
      //Beacon frames are only accepted when FRMFILT1.ACCEPT_FT0_BEACON = 1
      //Length byte ≥ 9, no destination address (mode 0), source address is 
      //included, source PAN ID matches PAN_ID, or PAN_ID equals 0xFFFF.
      if (((cell_frmfilt1->get() & ACCEPT_FT0_BEACON) == 0) ||
	  (RXframe->FrameLength < 9) ||
	  ((RXframe->DestAddressMode) != 0) ||
	  ((RXframe->SrcAddressMode) == 0) ||
	  (((RXframe->SourcePanID != PanID) 
	    && (RXframe->SourcePanID != 0xFFFF))))
	return(false);
    }

  if (RXframe->FrameType == DATA)
    {
      //Data frames are only accepted when FRMFILT1.ACCEPT_FT1_DATA = 1,
      //Length byte ≥ 9, destination address and/or source address is 
      //included in the frame, If no destination address, the 
      //FRMFILT0.PAN_COORDINATOR bit must be set, and the source
      //PAN ID must equal PAN_ID.
      if (((cell_frmfilt1->get() & ACCEPT_FT1_DATA) == 0) ||
	  (RXframe->FrameLength < 9) ||
	  (((RXframe->SrcAddressMode)==0)&&((RXframe->DestAddressMode)==0)) ||
	  (((RXframe->DestAddressMode)==0)&&
	   (((cell_frmfilt0->get() & PAN_COORDINATOR) == 0)||
	    (RXframe->SourcePanID != PanID))))
	return(false);
    }

  if (RXframe->FrameType == ACKNOLEDGEMENT)
    {
      //Acknowledgment frames are only accepted when:
      //FRMFILT1.ACCEPT_FT2_ACK = 1, Length byte = 5
      if (((cell_frmfilt1->get() & ACCEPT_FT2_ACK) == 0) ||
	  (RXframe->FrameLength != 5))
	return(false);
    }

  if (RXframe->FrameType == MAC_COMMAND)
    {
      //MAC command frames are only accepted when:
      //FRMFILT1.ACCEPT_FT3_MAC_CMD = 1, Length byte ≥ 9
      //destination address and/or source address included. If no 
      //destination address, FRMFILT0.PAN_COORDINATOR bit must be set, 
      //and the source PAN ID must equal PAN_ID.
      if (((cell_frmfilt1->get() & ACCEPT_FT3_MAC_CMD) == 0) ||
	  (RXframe->FrameLength < 9) ||
	  (((RXframe->SrcAddressMode)==0)&&((RXframe->DestAddressMode)==0))||
	  (((RXframe->DestAddressMode)==0)&&
	   (((cell_frmfilt0->get() & PAN_COORDINATOR) == 0)||
	    (RXframe->SourcePanID != PanID))))
	return(false);
    }

  if ((RXframe->FrameType == RESERVED0) ||
      (RXframe->FrameType == RESERVED1) ||
      (RXframe->FrameType == RESERVED2) ||
      (RXframe->FrameType == RESERVED3))
    {
      //Reserved frame types (4, 5, 6, and 7) are only accepted when
      //FRMFILT1.ACCEPT_FT4TO7_RESERVED = 1 (default is 0), Length byte ≥ 9
      if (((cell_frmfilt1->get() & ACCEPT_FT4TO7_RESERVED) == 0) ||
	  (RXframe->FrameLength < 9))
	return(false);
    }

  //If function hasn't already returned false, the frame passes the filter.  
  return(true);
}

void
cl_CC2530_radio::doTX(void)
{
  if ((cell_fsmstat1->get() & SFDbm) == 0)//SFD sent status low -> send SFD first
    {
      radio_out = SFD;
      cell_fsmstat1->set_bit1(SFDbm);//SFD sent status goes high
      TX_PC = 0;//reinitialize TX count
    }
  else
    {
      radio_out = TXFIFO[TX_PC];
      if (TX_PC == TXframe->FrameLength)
	{
	  if (RxEnableWhenTxComplete)
	    rxEnable = true;
	  cell_fsmstat1->set_bit0(SFDbm);//SFD sent status goes low
	  //TX END
	}
      TX_PC++;
      //Number of bytes left in the fifo that haven't been sent
      cell_txfifocnt->set(TXframe->FrameLength - TX_PC++);
    }
}

bool
cl_CC2530_radio::Dest_match_address(void)
{
  if (RXframe->DestAddressMode != 0)
    {
      if (RXframe->DestAddressMode == 1)
	return(true);//if reserved accept and filter comes later
      else if (RXframe->DestAddressMode == 2)
	{
	  if ((RXframe->DestinationAddress == ShortAddress) 
	      && (RXframe->DestinationPanID == PanID))
	    return(true);
	}
      else if (RXframe->DestAddressMode == 3)
	{
    	  if ((RXframe->ExtDestAddressH == ExtAddressH) 
	      && (RXframe->ExtDestAddressL == ExtAddressL))
	    return(true);
	}
      return(false);
    }
  return(true);//if no destination address accept frame, filter comes later
} 

bool cl_CC2530_radio::set_ack()
{
  TXFIFO[0] = 6;//length of ack
  int AckFCF = 2; //all fields of FCF are set to 0 except Frame type, Ack (010)

  //Check if frame pending bit should be 1 in FCF
  //depends on SACKPEND strobe, PENDING_OR register bit, and AUTOPEND feature

  if (CSP->pendingBit || 
      ((cell_frmctrl1->get() & bmPENDING_OR) != 0) ||
      (((cell_srcmatch->get() & bmAUTOPEND) != 0) && autopendEnable))
    AckFCF = AckFCF | 0x10;//set pending bit
  TXFIFO[1] = AckFCF & 0xFF;
  TXFIFO[2] = (AckFCF & 0xFF00)>>8;
  TXFIFO[3] = RXframe->DataSequenceNumber;
  TXFIFO[4] = 0; //FCS not used in simulator
  TXFIFO[5] = 0; //FCS not used in simulator
  return(true);
}

void cl_CC2530_radio::send_ack()
{
  ACKsent = false;
  //TX one Byte
  doTX();
  //if SFD status has gone low again, ack has been sent
  if ((cell_fsmstat1->get() & SFDbm) == 0)
    ACKsent = true;
}

int
cl_CC2530_radio::fsm(void)
{

  if (CSP->STXON)
    {
      state = TX_CALIBRATION;
      stateNum = 32;
      start_timer(192);
    }

  if (CSP->SRFOFF && !TX_ACTIVE)
    {
      state = IDLE;
      stateNum = 0;
      //Radio idle state interrupt
      if ((cell_rfirqm1->get() & bmRFIDLE) != 0) //interrupt enable
	{
	  cell_rfirqf1->set_bit1(RFIDLE); 
	  cell_s1con->set_bit1(bmRFIF);
	}
    }

  if (RX_ACTIVE)
    {
      if (CSP->STXONCCA && (CSP->CCA == 1))
	{
	  state = TX_CALIBRATION;
	  stateNum = 32;
	  start_timer(192);
	}
      if (CSP->SRXON || CSP->SFLUSHRX)
	{
	  state = RX_CALIBRATION;
	  stateNum = 2;
	  start_timer(192);
	}
    }

  if (TX_ACTIVE)
    {
      if (CSP->SRFOFF || CSP->SRXON)
	{
	  state = TX_SHUTDOWN;
	  stateNum = 26;
	  start_timer(192);
	}
    }

  switch (state)
    {
    case IDLE:    
 
      TX_ACTIVE = false;
      RX_ACTIVE = false;

      if (rxEnable != 0)
	{
	  state = RX_CALIBRATION;
	  //SET TIMER TO 192 µs, counting down
	  start_timer(1);
	  stateNum = 2;
	}

      break;

    case RX_CALIBRATION: 
      TX_ACTIVE = false;
      RX_ACTIVE = true;

      if (Timeout)
	{
	  state = SFD_WAIT;
	  stateNum = 3;
	  Timeout = false;
	}

      break; 

    case SFD_WAIT: 
      TX_ACTIVE = false;
      RX_ACTIVE = true;

      if (rxEnable == 0)
	{
	  state = IDLE;
	  stateNum = 0;
	  //Radio idle state interrupt
	  if ((cell_rfirqm1->get() & bmRFIDLE) != 0) //interrupt enable
	    {
	      cell_rfirqf1->set_bit1(RFIDLE); 
	      cell_s1con->set_bit1(bmRFIF);
	    }
	}

      if (radio_in == SFD)
	{
	  cell_fsmstat1->set_bit1(SFDbm);//SFD received status
	  //SFD received interrupt
	  if ((cell_rfirqm0->get() & bmSFD) != 0) //interrupt enable
	    {
	      cell_rfirqf0->set_bit1(SFD_IF); 
	      cell_s1con->set_bit1(bmRFIF);
	    }
	  state = RX;
	  stateNum = 7;
	}

      break;

    case RX: 
      //at the end of rx, doRX() changes slotted_ack or unslotted_ack to true
      slotted_ack = false;
      unslotted_ack = false;
      TX_ACTIVE = false;
      RX_ACTIVE = true;
      doRX();

      if (overflow)
	{
	  state = RX_OVERFLOW;
	  stateNum = 17;
	}

      if (slotted_ack)
	{
	  state = ACK_DELAY;
	  int delay = rand() % 31 + 12; //random number between 12 and 31 
	  start_timer(delay<<4);// random delay time of 12 to 31 symbol periods (16µs)
	  stateNum = 55;
	}

      if (unslotted_ack)
	{
	  state = ACK_CALIBRATION;
	  start_timer(192);
	  stateNum = 48;
	}

      if (frame_not_for_me)
	{
	  state = RXFIFO_RESET;
	  stateNum = 16;
	}

      if (frame_completed)
	{
	  state = RX_RXWAIT;
	  if ((cell_fsmctrl->get() & bmRX2RX_TIME_OFF) == 0)
	    Timeout = true;
	  else
	    start_timer(192);//12 symbol period time out
	  stateNum = 14;
	  cell_fsmstat1->set_bit0(SFDbm);//SFD received status goes low
	}

      break;

    case RX_RXWAIT: 
      TX_ACTIVE = false;
      RX_ACTIVE = true;

      if (Timeout)
	{
	  state = SFD_WAIT;
	  stateNum = 3;
	  Timeout = false;
	}

      break;

    case RXFIFO_RESET: 
      TX_ACTIVE = false;
      RX_ACTIVE = true;

      for (int i = 0; i < 128; i ++)
	{
	  RXFIFO[i] = 0;
	  xram->write(0x6000 + i, 0);
	}

      state = SFD_WAIT;
      stateNum = 3;
      break;

    case RX_OVERFLOW: 
      TX_ACTIVE = false;
      RX_ACTIVE = false;

      //SIGNAL OVF
      if (CSP->SFLUSHRX)
	{
	  state = RX_CALIBRATION;
	  stateNum = 2;
	}

      break;

    case TX_CALIBRATION: 
      TX_ACTIVE = true;
      RX_ACTIVE = false;

      if (Timeout)
	{
	  state = TX;
	  stateNum = 34;
	  Timeout = false;
	}

      break;

    case TX: 
      TX_ACTIVE = true;
      RX_ACTIVE = false;
      doTX();

      if (underflow)
	{
	  state = TX_UNDERFLOW;
	  stateNum = 56;
	  start_timer(2);
	  underflow = false;
	}

      if (frame_sent)
	{
	  //Radio end of tx interrupt
	  if ((cell_rfirqm1->get() & bmTXDONE) != 0) //interrupt enable
	    {
	      cell_rfirqf1->set_bit1(TXDONE); 
	      cell_s1con->set_bit1(bmRFIF);
	    }
	  state = TX_FINAL;
	  stateNum = 39;
	  frame_sent = false;
	}

      break;

    case TX_FINAL:
      TX_ACTIVE = true;
      RX_ACTIVE = false;

      if (rxEnable != 0)
	{
	  state = TX_RX_TRANSIT;
	  stateNum = 40;
	  start_timer(190);
	}
      else
	{
	  state = TX_SHUTDOWN;
	  stateNum = 26;
	}
      break;

    case TX_RX_TRANSIT: 
      TX_ACTIVE = true;
      RX_ACTIVE = false;

      if (Timeout)
	{
	  state = SFD_WAIT;
	  stateNum = 3;
	  Timeout = false;
	}
      break;

    case ACK_CALIBRATION: 
      TX_ACTIVE = true;
      RX_ACTIVE = false;

      if (Timeout)
	{
	  state = ACK;
	  stateNum = 49;
	  Timeout = false;
	}

      break;

    case ACK: 
      TX_ACTIVE = true;
      RX_ACTIVE = false;

      if (cancelACK)
	{
	  state = TX_SHUTDOWN;
	  stateNum = 26;
	}
      else
	send_ack();
 
      if (ACKsent)
	{
	  CSP->radioAnswer = true;
	  //Radio tx ack sent interrupt
	  if ((cell_rfirqm1->get() & bmTXACKDONE) != 0) //interrupt enable
	    {
	      cell_rfirqf1->set_bit1(TXACKDONE); 
	      cell_s1con->set_bit1(bmRFIF);
	    }


	  if (rxEnable == 0)
	    {
	      state = TX_SHUTDOWN;
	      stateNum = 26;
	      ACKsent = false;
	    }

	  if (rxenmask != 0)
	    {
	      state = TX_RX_TRANSIT;
	      stateNum = 40;
	      start_timer(190);
	      ACKsent = false;
	    }

	}

      break;

    case ACK_DELAY: 
      TX_ACTIVE = true;
      RX_ACTIVE = false;

      if (Timeout)
	{
	  state = ACK_CALIBRATION;
	  start_timer(192);
	  stateNum = 48;
	}

      break;

    case TX_UNDERFLOW: 
      TX_ACTIVE = true;
      RX_ACTIVE = false;

      if (Timeout)
	{
	  state = TX_FINAL;
	  stateNum = 39;
	  Timeout = false;
	}

      break;

    case TX_SHUTDOWN:
      TX_ACTIVE = true;
      RX_ACTIVE = false;
      state = IDLE;
      stateNum = 0;

      //Radio idle state interrupt
      if ((cell_rfirqm1->get() & bmRFIDLE) != 0) //interrupt enable
	{
	  cell_rfirqf1->set_bit1(RFIDLE); 
	  cell_s1con->set_bit1(bmRFIF);
	}

      break;
    }

  if (TX_ACTIVE)
    {
      cell_fsmstat1->set_bit1(bmTX_ACTIVE);
      cell_fsmstat1->set_bit0(bmRX_ACTIVE);
    }
  else if (RX_ACTIVE)
    {
      cell_fsmstat1->set_bit1(bmRX_ACTIVE);
      cell_fsmstat1->set_bit0(bmTX_ACTIVE);
    }
  else
    {
      cell_fsmstat1->set_bit0(bmRX_ACTIVE);
      cell_fsmstat1->set_bit0(bmTX_ACTIVE);
    }

  return(0);
}

void
cl_CC2530_radio::happen(class cl_hw *where, enum hw_event he, void *params)
{
  if (he == EV_RADIO_ACK)//Command sent from CSP
    set_ack();
  if (he == EV_NO_ACK)
    cancelACK = true;
}

//Instructions of the command Strobe processor

cl_CC2530_CSP::cl_CC2530_CSP(class cl_uc *auc, int aid, char *aid_string): cl_hw(auc, HW_CC2530_RADIO, aid, aid_string)
{
  make_partner(HW_CC2530_DMA, 1);
  make_partner(HW_CC2530_RADIO, 1);
  xram= uc->address_space(MEM_XRAM_ID);
  sfr= uc->address_space(MEM_SFR_ID);
  make_csp_watched_cells();
  init();
}

int
cl_CC2530_CSP::init(void)
{
  reset();
  return(0);
}

void
cl_CC2530_CSP::reset(void)
{
  PC = 0;
  csp_stop = false;
  cell_cspstat->set_bit0(bmCSPrunning);
  waitingForAnswer = false;
  WaitingForOvfs = false;
  pendingBit = false;
  STXON = false;
  SRFOFF = false;
  SRXON = false;
  SFLUSHRX = false;
  cell_cspt->set(0xFF);
  cell_cspctrl->set(0);
  cell_cspstat->set(0);
  cell_cspx->set(0);
  cell_cspy->set(0);
  cell_cspz->set(0);


  //all csp instr register initialized at 0xD0 (NOP)
  for (int i = 0; i<24; i++)
    xram->write(0x61C0 + i, 0xD0);
}

bool
cl_CC2530_CSP::condition(int C)
{
  bool condition;

  switch(C)
    {
    case 0: condition = (CCA == 1); break;
    case 1: condition = (SFD == 1); break;
    case 2: condition = CPU_ctrl; break;
    case 3: break;
    case 4: condition = (cell_cspx->get() == 0); break;
    case 5: condition = (cell_cspy->get() == 0); break;
    case 6: condition = (cell_cspz->get() == 0); break;
    case 7: condition = RSSI_valid; break;
    }

  return(condition);
}

void
cl_CC2530_CSP::inst_dec_x(uchar code)
{
  cell_cspx->set(cell_cspx->get() - 1);
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_dec_y(uchar code)
{
  cell_cspy->set(cell_cspy->get() - 1);
}

void
cl_CC2530_CSP::inst_dec_z(uchar code)
{
  cell_cspz->set(cell_cspz->get() - 1);
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_inc_x(uchar code)
{
  cell_cspx->set(cell_cspx->get() + 1);
  X = X + 1;
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_inc_y(uchar code)
{
  cell_cspy->set(cell_cspy->get() + 1);
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_inc_z(uchar code)
{
  cell_cspz->set(cell_cspz->get() + 1);
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_inc_maxy(uchar code)
{
  M = code & 7;
  cell_cspy->set(min(cell_cspy->get() + 1, (int)M));
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_rand_xy(uchar code)
{
  Y = cell_cspy->get();
  if (Y<=7)
    cell_cspx->set(rand() & (0xFF>>(8-Y)));
  else
    cell_cspx->set(rand() & 0xFF);
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_int(uchar code)
{
  IRQ_CSP_INT();
  PC = PC + 1;
}


void
cl_CC2530_CSP::inst_waitx(uchar code)
{
  WaitingForOvfs = true;
  if (cell_cspx->get() != 0)
    {
      fprintf(stderr, "Awaiting Mac timer overflows...\n");
    } 
  else
    {
      IRQ_CSP_WT();
      PC = PC + 1;
      IRQ_CSP_WT();
      WaitingForOvfs = false;
    }
}

void
cl_CC2530_CSP::inst_set_cmp1(uchar code)
{
  //Set the compare value of the MAC Timer to the current timer value.
  cell_t2msel->set((cell_t2msel->get() & 0xF8) + 011);//set to write to cmp1 reg
  cell_t2m2->set(0);//WHICH timer value?
  cell_t2m1->set(0);
  cell_t2m0->set(0);
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_waitw(uchar code)
{
  W = code & 0x1F;
  WaitingForWOvfs = true;
  if (waitWend)
    {
      OVFcountdown = W;//W is original value of the number of overflows needed
      //OVFcountdown is the number that will be decremented to count the overflows
      waitWend = false;
    }
  if (OVFcountdown != 0)
    {
      fprintf(stderr, "Awaiting for %d Mac timer overflows...\n", W);
    } 
  else
    {
      IRQ_CSP_WT();
      PC = PC + 1;
      WaitingForWOvfs = false;
      waitWend = true;
    }
}

void
cl_CC2530_CSP::inst_wait_event1(uchar code)
{
  WaitingForEvent1 = true;
  if (!event1)
    {
      fprintf(stderr, "Awaiting Mac timer event 1...\n");
    } 
  else
    {
      IRQ_CSP_WT();
      PC = PC + 1;
      event1 = false;
      WaitingForEvent1 = false;
    }
}

void
cl_CC2530_CSP::inst_wait_event2(uchar code)
{
  WaitingForEvent2 = true;
  if (!event2)
    {
      fprintf(stderr, "Awaiting Mac timer event 2...\n");
    } 
  else
    {
      IRQ_CSP_WT();
      PC = PC + 1;
      event2 = false;
      WaitingForEvent2 = false;
    }
}

void
cl_CC2530_CSP::inst_label(uchar code)
{
  label = PC + 1; 
}

void
cl_CC2530_CSP::inst_repeatC(uchar code)
{
  C = code & 7;
  N = (code & 8) != 0;
  if (N)
    cond = !condition(C);
  else
    cond = condition(C);
    
  if (cond)
    {
      PC = label;
    }
  else
    {
      PC = PC + 1;
    }
}

void
cl_CC2530_CSP::inst_skipCS(uchar code)
{
  S = (code & 0x70)>>4;
  C = code & 7;
  N = ((code & 0x08)>>3 == 1);
  if(N)
    {
      cond = !condition(C);
    }
  else
    {
      cond = condition(C);
    }
    
  if (cond)
    {
      PC = PC + S + 1; 
    }
  else
    {
      PC = PC + 1;
    }
}

void
cl_CC2530_CSP::inst_stop(uchar code)
{
  IRQ_CSP_STOP();
  //stop csp program execution
}

void
cl_CC2530_CSP::inst_snop(uchar code)
{
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_srxon(uchar code)
{
  SRXON = true;
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_stxon(uchar code)
{
  STXON = true;
  if ((cell_frmctrl1->get() & bmSetRxenmaskOnTx)!=0)
    {
      cell_rxenable->set_bit1(0x40);
      RxEnableWhenTxComplete = true;
    }
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_stxoncca(uchar code)
{
  if (CCA == 1)
    STXON = true;
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_ssamplecca(uchar code)
{
  cell_sampledcca->set(CCA);
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_srfoff(uchar code)
{
  STXON = false;
  SRXON = false;
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_sflushrx(uchar code)
{
  /*for (int i = 0; i < 128; i++)
    {
      RXFIFO[i] = 0;
      xram->set(0x6000 + i, 0);
      }*/
  SFLUSHRX = true;
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_sflushtx(uchar code)
{
  /*for (int i = 0; i < 128; i++)
    {
      TXFIFO[i] = 0;
      xram->set(0x6080 + i, 0);
      }*/
  SFLUSHTX = true;
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_sack(uchar code)
{
  if (!SRXON)
    {
      //Radio strobe error interrupt
      if ((cell_rfirqm1->get() & bmSTROBEERR) != 0) //interrupt enable
	{
	  cell_rfirqf1->set_bit1(STROBEERR); 
	  cell_s1con->set_bit1(bmRFIF);
	}
    }
  if (!waitingForAnswer)
    {
      //CC2530_radio->ack();
      pendingBit = false;//pending bit 0 in ack frame
      inform_partners(EV_RADIO_ACK, 0);
      waitingForAnswer = true;
    }
  if (radioAnswer)
    {
      PC = PC + 1;
      radioAnswer = false;
      waitingForAnswer = false;
    }
}

void
cl_CC2530_CSP::inst_sackpend(uchar code)
{
  if (!SRXON)
    {
      //Radio strobe error interrupt
      if ((cell_rfirqm1->get() & bmSTROBEERR) != 0) //interrupt enable
	{
	  cell_rfirqf1->set_bit1(STROBEERR); 
	  cell_s1con->set_bit1(bmRFIF);
	}
    }
  if (!waitingForAnswer)
    {
      pendingBit = true;//pending field set
      //CC2530_radio->send_ack();
      //Inform radio that it has to send automatic ack frame:
      inform_partners(EV_RADIO_ACK, 0);
      waitingForAnswer = true;
      //Waiting for Radio to answer that it has finished sending ack
    }
  if (radioAnswer)
    {
      PC = PC + 1;
      radioAnswer = false;
      waitingForAnswer = false;
    }
}

void
cl_CC2530_CSP::inst_snack(uchar code)
{
  if (!SRXON)
    {
      //Radio strobe error interrupt
      if ((cell_rfirqm1->get() & bmSTROBEERR) != 0) //interrupt enable
	{
	  cell_rfirqf1->set_bit1(STROBEERR); 
	  cell_s1con->set_bit1(bmRFIF);
	}
    }
  inform_partners(EV_NO_ACK, 0);
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_srx_mask_bit_set(uchar code)
{
  cell_rxenable->set_bit1(0x20);//sets bit 5 in rxenable reg
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_srx_mask_bit_clr(uchar code)
{
  cell_rxenable->set_bit0(0x20);//sets bit 5 in rxenable reg
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_isstop(uchar code)
{
  IRQ_CSP_STOP();
}

void
cl_CC2530_CSP::inst_isstart(uchar code)
{
  csp_stop = false;
  cell_cspstat->set_bit1(bmCSPrunning);
  PC = 0;
}

void
cl_CC2530_CSP::inst_isrxon(uchar code)
{
  SRXON = true;
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_isrx_mask_bit_set(uchar code)
{
  cell_rxenable->set_bit1(0x20);//sets bit 5 in rxenable reg
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_isrx_mask_bit_clr(uchar code)
{
  cell_rxenable->set_bit0(0x20);//sets bit 5 in rxenable reg
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_istxon(uchar code)
{
  STXON = true;
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_isclear(uchar code)
{
  csp_stop = true;
  cell_cspstat->set_bit0(bmCSPrunning);
  for (int i = 0; i < 24; i++)
    {
      xram->set(0x6080 + i, 0xD0);//0xD0 <=> SNOP
    }
  PC = 0;
  PCwrite = 0;
}

void
cl_CC2530_CSP::IRQ_CSP_STOP(void)
{
  csp_stop = true;
  //CSP stop interrupt
  if ((cell_rfirqm1->get() & bmCSP_STOP) != 0) //interrupt enable
    {
      cell_rfirqf1->set_bit1(CSP_STOP); 
      cell_s1con->set_bit1(bmRFIF);
    }
  cell_cspstat->set_bit0(bmCSPrunning);
}

void
cl_CC2530_CSP::IRQ_CSP_WT(void)
{
  //End of CSP wait interrupt
  if ((cell_rfirqm1->get() & bmCSP_WAIT) != 0) //interrupt enable
    {
      cell_rfirqf1->set_bit1(CSP_WAIT); 
      cell_s1con->set_bit1(bmRFIF);
    }
}

void
cl_CC2530_CSP::IRQ_CSP_INT(void)
{
  //Manual interrupt generated from CSP
  if ((cell_rfirqm1->get() & bmCSP_MANINT) != 0) //interrupt enable
    {
      cell_rfirqf1->set_bit1(CSP_MANINT); 
      cell_s1con->set_bit1(bmRFIF);
    }
}

void 
cl_CC2530_CSP::CSP_tick(void)
{
  cell_cspstat->set((cell_cspstat->get() & 0xE0) + PC);
  if (cell_cspt->get() == 0)
    IRQ_CSP_STOP();
  if (!csp_stop)
    {
      t_mem code = xram->read(0x61C0 + PC);
      exec_inst(code);
    }
}

int
cl_CC2530_CSP::exec_inst(t_mem code)
{
  int res= resGO;

  tick(1);

  if ((code & 0x80) == 0)
    inst_skipCS(code);
  if ((code & 0xE0) == 0x80)
    inst_waitw(code);
  if ((code & 0xF0) == 0xA0)
    inst_repeatC(code);
  if ((code & 0xF8) == 0xC8)
    inst_inc_maxy(code);
  if ((code & 0xF0) == 0xD0)
    {
      switch (code & 0x0F)
	{
	case 0: inst_snop(code); break;
	case 1: break;
	case 2: inst_stop(code); break;
	case 3: inst_srxon(code); break;
	case 4: inst_srx_mask_bit_set(code); break;
	case 5: inst_srx_mask_bit_clr(code); break;
	case 6: inst_sack(code); break;
	case 7: inst_sackpend(code); break;
	case 8: inst_snack(code); break;
	case 9: inst_stxon(code); break;
	case 10: inst_stxoncca(code); break;
	case 11: inst_ssamplecca(code); break;
	case 12: break;
	case 13: inst_sflushrx(code); break;
	case 14: inst_sflushtx(code); break;
	case 15: inst_srfoff(code); break;
	}
    }
  if ((code & 0xF0) == 0xE0)
    {
      switch (code & 0x0F)
	{
	case 0: break;
	case 1: inst_isstart(code); break;
	case 2: inst_isstop(code); break;
	case 3: inst_isrxon(code); break;
	case 4: inst_isrx_mask_bit_set(code); break;
	case 5: inst_isrx_mask_bit_clr(code); break;
	case 6: inst_sack(code); break;
	case 7: inst_sackpend(code); break;
	case 8: inst_snack(code); break;
	case 9: inst_istxon(code); break;
	case 10: inst_stxoncca(code); break;
	case 11: inst_ssamplecca(code); break;
	case 12: break;
	case 13: inst_sflushrx(code); break;
	case 14: inst_sflushtx(code); break;
	case 15: inst_srfoff(code); break;
	}
    }

  switch (code)
    {
    case 0xB8: inst_wait_event1(code); break;
    case 0xB9: inst_wait_event2(code); break;
    case 0xBA: inst_int(code); break;
    case 0xBB: inst_label(code); break;
    case 0xBC: inst_waitx(code); break;
    case 0xBD: inst_rand_xy(code); break;
    case 0xBE: inst_set_cmp1(code); break;
    case 0xC0: inst_inc_x(code); break;
    case 0xC1: inst_inc_y(code); break;
    case 0xC2: inst_inc_z(code); break;
    case 0xC3: inst_dec_x(code); break;
    case 0xC4: inst_dec_y(code); break;
    case 0xC5: inst_dec_z(code); break;

    case 0xFF: inst_isclear(code); break;
    }

  if (PC == 24)
    PC = 0;

  return(res);
}

void
cl_CC2530_CSP::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_rfst)
    {
      fprintf(stderr, "Writing to RFST: Program for CSP.\n");
      if (*val == 0xE1)//inst isstart
	{
	  csp_stop = false;
	  cell_cspstat->set_bit1(bmCSPrunning);
	}
      if (*val == 0xE2)//inst isstop
	{
	  csp_stop = true;  
	  cell_cspstat->set_bit0(bmCSPrunning);
	}
      if (*val == 0xFF)//inst isclear
	{
	  inst_isclear(*val);
	}
      if ((*val & 0xF0) == 0xE0)//inst isrxon
	{
	  exec_inst(*val);
	}
      else
	{
	  xram->write(0x61C0 + PCwrite, *val);
	  PCwrite++;
	  if (PCwrite == 24)
	    PCwrite = 0;
	}
    }
}

void
cl_CC2530_CSP::happen(class cl_hw *where, enum hw_event he, void *params)
{
 if (where->cathegory == HW_MAC_TIMER)
    {
      if (he == EV_OVERFLOW)
	{
	  if (WaitingForOvfs)//Counting overflows (WAITX)
	    {
	      cell_cspx->write(cell_cspx->get() - 1);
	    }
	  if (WaitingForWOvfs)//Counting overflows (WAITX)
	    {
	      OVFcountdown = OVFcountdown - 1;
	    }
	}
      if (he == EV_T2_EVENT1)
	{
	  if (WaitingForEvent1)
	    event1 = true;
	}
      if (he == EV_T2_EVENT1)
	{
	  if (WaitingForEvent2)
	    event2 = true;
	}
    }
}

void
cl_CC2530_CSP::make_csp_watched_cells()
{
  /*register_cell(xram, CSPPROG0, &cell_cspprog0, wtd_restore_write);
  register_cell(xram, CSPPROG1, &cell_cspprog1, wtd_restore_write);
  register_cell(xram, CSPPROG2, &cell_cspprog2, wtd_restore_write);
  register_cell(xram, CSPPROG3, &cell_cspprog3, wtd_restore_write);
  register_cell(xram, CSPPROG4, &cell_cspprog4, wtd_restore_write);
  register_cell(xram, CSPPROG5, &cell_cspprog5, wtd_restore_write);
  register_cell(xram, CSPPROG6, &cell_cspprog6, wtd_restore_write);
  register_cell(xram, CSPPROG7, &cell_cspprog7, wtd_restore_write);
  register_cell(xram, CSPPROG8, &cell_cspprog8, wtd_restore_write);
  register_cell(xram, CSPPROG9, &cell_cspprog9, wtd_restore_write);
  register_cell(xram, CSPPROG10, &cell_cspprog10, wtd_restore_write);
  register_cell(xram, CSPPROG11, &cell_cspprog11, wtd_restore_write);
  register_cell(xram, CSPPROG12, &cell_cspprog12, wtd_restore_write);
  register_cell(xram, CSPPROG13, &cell_cspprog13, wtd_restore_write);
  register_cell(xram, CSPPROG14, &cell_cspprog14, wtd_restore_write);
  register_cell(xram, CSPPROG15, &cell_cspprog15, wtd_restore_write);
  register_cell(xram, CSPPROG16, &cell_cspprog16, wtd_restore_write);
  register_cell(xram, CSPPROG17, &cell_cspprog17, wtd_restore_write);
  register_cell(xram, CSPPROG18, &cell_cspprog18, wtd_restore_write);
  register_cell(xram, CSPPROG19, &cell_cspprog19, wtd_restore_write);
  register_cell(xram, CSPPROG20, &cell_cspprog20, wtd_restore_write);
  register_cell(xram, CSPPROG21, &cell_cspprog21, wtd_restore_write);
  register_cell(xram, CSPPROG22, &cell_cspprog22, wtd_restore_write);
  register_cell(xram, CSPPROG23, &cell_cspprog23, wtd_restore_write);*/
  register_cell(xram, RFIRQF0, &cell_rfirqf0, wtd_restore_write);
  register_cell(xram, S1CON, &cell_s1con, wtd_restore_write);
  register_cell(xram, RFIRQF1, &cell_rfirqf1, wtd_restore_write);
  register_cell(xram, RFIRQM1, &cell_rfirqm1, wtd_restore_write);

  register_cell(xram, CSPCTRL, &cell_cspctrl, wtd_restore_write);
  register_cell(xram, CSPSTAT, &cell_cspstat, wtd_restore_write);
  register_cell(xram, CSPX, &cell_cspx, wtd_restore_write);
  register_cell(xram, CSPY, &cell_cspy, wtd_restore_write);
  register_cell(xram, CSPZ, &cell_cspz, wtd_restore_write);
  register_cell(xram, CSPT, &cell_cspt, wtd_restore_write);

  register_cell(sfr, RFST, &cell_rfst, wtd_restore_write);
}

void
cl_CC2530_radio::make_radio_watched_cells()
{
  register_cell(xram, RFIRQF0, &cell_rfirqf0, wtd_restore_write);
  register_cell(xram, S1CON, &cell_s1con, wtd_restore_write);
  register_cell(xram, RFIRQF1, &cell_rfirqf1, wtd_restore_write);
  register_cell(xram, FRMFILT0, &cell_frmfilt0, wtd_restore_write);
  register_cell(xram, FRMFILT1, &cell_frmfilt1, wtd_restore_write);
  register_cell(xram, SRCMATCH, &cell_srcmatch, wtd_restore_write);
  register_cell(xram, SRCSHORTEN0, &cell_srcshorten0, wtd_restore_write);
  register_cell(xram, SRCSHORTEN1, &cell_srcshorten1, wtd_restore_write);
  register_cell(xram, SRCSHORTEN2, &cell_srcshorten2, wtd_restore_write);
  register_cell(xram, SRCEXTEN0, &cell_srcexten0, wtd_restore_write);
  register_cell(xram, SRCEXTEN1, &cell_srcexten1, wtd_restore_write);
  register_cell(xram, SRCEXTEN2, &cell_srcexten2, wtd_restore_write);
  register_cell(xram, FRMCTRL0, &cell_frmctrl0, wtd_restore_write);
  register_cell(xram, FRMCTRL1, &cell_frmctrl1, wtd_restore_write);
  register_cell(xram, RXENABLE, &cell_rxenable, wtd_restore_write);
  register_cell(xram, RXMASKSET, &cell_rxmaskset, wtd_restore_write);
  register_cell(xram, RXMASKCLR, &cell_rxmaskclr, wtd_restore_write);
  register_cell(xram, FREQTUNE, &cell_freqtune, wtd_restore_write);
  register_cell(xram, FREQCTRL, &cell_freqctrl, wtd_restore_write);
  register_cell(xram, TXPOWER, &cell_txpower, wtd_restore_write);
  register_cell(xram, TXCTRL, &cell_txctrl, wtd_restore_write);
  register_cell(xram, FSMSTAT0, &cell_fsmstat0, wtd_restore_write);
  register_cell(xram, FSMSTAT1, &cell_fsmstat1, wtd_restore_write);
  register_cell(xram, FIFOPCTRL, &cell_fifopctrl, wtd_restore_write);
  register_cell(xram, FSMCTRL, &cell_fsmctrl, wtd_restore_write);
  register_cell(xram, CCACTRL0, &cell_ccactrl0, wtd_restore_write);
  register_cell(xram, CCACTRL1, &cell_ccactrl1, wtd_restore_write);
  register_cell(xram, RSSI, &cell_rssi, wtd_restore_write);
  register_cell(xram, RSSISTAT, &cell_rssistat, wtd_restore_write);
  register_cell(xram, RXFIRST, &cell_rxfirst, wtd_restore_write);
  register_cell(xram, RXFIFOCNT, &cell_rxfifocnt, wtd_restore_write);
  register_cell(xram, TXFIFOCNT, &cell_txfifocnt, wtd_restore_write);
  register_cell(xram, RXFIRST_PTR, &cell_rxfirst_ptr, wtd_restore_write);
  register_cell(xram, RXLAST_PTR, &cell_rxlast_ptr, wtd_restore_write);
  register_cell(xram, RXP1_PTR, &cell_rxp1_ptr, wtd_restore_write);
  register_cell(xram, TXFIRST_PTR, &cell_txfirst_ptr, wtd_restore_write);
  register_cell(xram, TXLAST_PTR, &cell_txlast_ptr, wtd_restore_write);
  register_cell(xram, RFIRQM0, &cell_rfirqm0, wtd_restore_write);
  register_cell(xram, RFIRQM1, &cell_rfirqm1, wtd_restore_write);
  register_cell(xram, RFERRM, &cell_rferrm, wtd_restore_write);
  register_cell(xram, MONMUX, &cell_monmux, wtd_restore_write);
  register_cell(xram, RFRND, &cell_rfrnd, wtd_restore_write);
  register_cell(xram, MDMCTRL0, &cell_mdmctrl0, wtd_restore_write);
  register_cell(xram, MDMCTRL1, &cell_mdmctrl1, wtd_restore_write);
  register_cell(xram, FREQEST, &cell_freqest, wtd_restore_write);
  register_cell(xram, RXCTRL, &cell_rxctrl, wtd_restore_write);
  register_cell(xram, FSCTRL, &cell_fsctrl, wtd_restore_write);
  register_cell(xram, FSCAL1, &cell_fscal1, wtd_restore_write);
  register_cell(xram, FSCAL2, &cell_fscal2, wtd_restore_write);
  register_cell(xram, FSCAL3, &cell_fscal3, wtd_restore_write);
  register_cell(xram, AGCCTRL0, &cell_agcctrl0, wtd_restore_write);
  register_cell(xram, AGCCTRL1, &cell_agcctrl1, wtd_restore_write);
  register_cell(xram, AGCCTRL2, &cell_agcctrl2, wtd_restore_write);
  register_cell(xram, AGCCTRL3, &cell_agcctrl3, wtd_restore_write);
  register_cell(xram, ADCTEST0, &cell_adctest0, wtd_restore_write);
  register_cell(xram, ADCTEST1, &cell_adctest1, wtd_restore_write);
  register_cell(xram, ADCTEST2, &cell_adctest2, wtd_restore_write);
  register_cell(xram, MDMTEST0, &cell_mdmtest0, wtd_restore_write);
  register_cell(xram, MDMTEST1, &cell_mdmtest1, wtd_restore_write);
  register_cell(xram, DACTEST0, &cell_dactest0, wtd_restore_write);
  register_cell(xram, DACTEST1, &cell_dactest1, wtd_restore_write);
  register_cell(xram, DACTEST2, &cell_dactest2, wtd_restore_write);
  register_cell(xram, ATEST, &cell_atest, wtd_restore_write);
  register_cell(xram, PTEST0, &cell_ptest0, wtd_restore_write);
  register_cell(xram, PTEST1, &cell_ptest1, wtd_restore_write);

  register_cell(xram, SRCMASK0, &cell_srcmask0, wtd_restore_write);
  register_cell(xram, SRCMASK1, &cell_srcmask1, wtd_restore_write);
  register_cell(xram, SRCMASK2, &cell_srcmask2, wtd_restore_write);

  register_cell(xram, SRCRESINDEX, &cell_srcresindex, wtd_restore_write);

  //watch Pan_id and address cells of registered sensors
  for (int i = 0; i<96; i++)
    register_cell(xram, PANID_00_0 + i, &cell_panid_00_0, wtd_restore_write);
  
}

