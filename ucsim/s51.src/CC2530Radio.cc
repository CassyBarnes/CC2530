#include "CC2530Radiocl.h"
#include <assert.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"

#define DEBUG
#ifdef DEBUG
#define TRACE() \
fprintf(stderr, "%s:%d in %s()\n", __FILE__, __LINE__, __FUNCTION__)
#else
#define TRACE()
#endif


cl_CC2530_radio::cl_CC2530_radio(class cl_uc *auc, int aid, char *aid_string): cl_hw(auc, HW_CC2530_RADIO, aid, aid_string)
{
  sfr= uc->address_space(MEM_SFR_ID);
  xram= uc->address_space(MEM_XRAM_ID);
  CSP = new cl_CC2530_CSP();
  state = IDLE;
  init();
}

int
cl_CC2530_radio::init()
{
  for (int i = 0; i<128; i++)
    {
      RXFIFO[i] = xram->read(0x6000 + i);
      TXFIFO[i] = xram->read(0x6080 + i);
      TXFifoPosition = 0;
      srand(1);
    }
}

int
cl_CC2530_radio::tick(int cycles)
{
  SystemTicks++;
  CSP->CSP_tick();
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
      if (TXFifoPosition == 128)
	TXFifoPosition = 0;
    }
}

void
cl_CC2530_radio::read(class cl_memory_cell *cell)
{

}

int
cl_CC2530_radio::get_frequency(int channelNumber)
{
  k = channelNumber;
  return(2405 + 5*(k - 11));
}

int
cl_CC2530_radio::get_c_frequency(void)
{
  return(2394 + (cell_freqctrl->get() & 0x7F));//in MHz
}

void
cl_CC2530_radio::RX(void)
{
  RXFIFO[RX_PC] = radio_in;//Receives 1 Byte 
  if (RX_PC == 0)
    {
      //first Byte represents frame length of MDPU
      length = radio_in;
    }
  if (RX_PC == length)
    {
      //RXFIFO is full
    }
  else
    RX_PC++;
}

void
cl_CC2530_radio::TX(void)
{
  radio_out = RXFIFO[TX_PC];
  TX_PC++;
}

int
cl_CC2530_radio::fsm(void)
{
  if (STXON)
    {
      state = TX_CALIBRATION;
      stateNum = 32;
      start_timer(192);
    }
  if (SRFOFF && !TX_ACTIVE)
    {
      state = IDLE;
      stateNum = 0;
    }
  if (RX_ACTIVE)
    {
      if (STXONCCA && (cca == 1))
	{
	  state = TX_CALIBRATION;
	  stateNum = 32;
	  start_timer(192);
	}
      if (SRXON || SFLUSHRX)
	{
	  state = RX_CALIBRATION;
	  stateNum = 2;
	  start_timer(192);
	}
    }
  if (RX_ACTIVE)
    {
      if (SRFOFF || SRXON)
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
	  timer_start(192);
	  stateNum = 2;
	}
      break;
    case RX_CALIBRATION: 
      TX_ACTIVE = false;
      RX_ACTIVE = true;
      if (timeout)
	{
	  state = SFD_WAIT;
	  stateNum = 3;
	  timeout = false;
	}
      break; 
    case SFD_WAIT: 
      TX_ACTIVE = false;
      RX_ACTIVE = true;
      if (rxenable == 0)
	{
	  state = IDLE;
	  stateNum = 0;
	}
      if (Radio_in == 0x7A)
	{
	  state = RX;
	  stateNum = 7;
	}
      break;
    case RX: 
      TX_ACTIVE = false;
      RX_ACTIVE = true;
      RX();
      if (overflow)
	{
	  state = RX_OVERFLOW;
	  stateNum = 17;
	}
      if (slotted_ack)
	{
	  state = ACK_DELAY;
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
	  if (rx2rx_time_off == 1)
	    timeout = true;
	  else
	    start_timer(192);
	  stateNum = 14;
	}
      break;
    case RX_RXWAIT: 
      TX_ACTIVE = false;
      RX_ACTIVE = true;
      if (timeout)
	{
	  state = SFD_WAIT;
	  stateNum = 3;
	  timeout = false;
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
      if (SFLUSHRX)
	{
	  state = RX_CALIBRATION;
	  stateNum = 2;
	}
      break;
    case TX_CALIBRATION: 
      TX_ACTIVE = true;
      RX_ACTIVE = false;
      if (timeout)
	{
	  state = TX;
	  stateNum = 34;
	  timeout = false;
	}
      break;
    case TX: 
      TX_ACTIVE = true;
      RX_ACTIVE = false;
      TX();
      if (underflow)
	{
	  state = TX_UNDERFLOW;
	  stateNum = 56;
	  start_timer(2);
	  underflow = false;
	}
      if (frame_sent)
	{
	  state = TX_FINAL;
	  stateNum = 39;
	  frame_sent = false;
	}
      break;
    case TX_FINAL:
      TX_ACTIVE = true;
      RX_ACTIVE = false;
      if (rxenable != 0)
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
      if (timeout)
	{
	  state = SFD_WAIT;
	  stateNum = 3;
	  timeout = false;
	}
      break;
    case ACK_CALIBRATION: 
      TX_ACTIVE = true;
      RX_ACTIVE = false;
      if (timeout)
	{
	  state = ACK;
	  stateNum = 49;
	  timeout = false;
	}
      break;
    case ACK: 
      TX_ACTIVE = true;
      RX_ACTIVE = false;
      send_ack();
      if (rxenmask != 0)
	{
	  state = TX_RX_TRANSIT;
	  stateNum = 40;
	  start_timer(190);
	}
      if (rxenable == 0)
	{
	  state = TX_SHUTDOWN;
	  stateNum = 26;
	}
      break;
    case ACK_DELAY: 
      TX_ACTIVE = true;
      RX_ACTIVE = false;
      state = ACK_CALIBRATION;
      stateNum = 48;
      break;
    case TX_UNDERFLOW: 
      TX_ACTIVE = true;
      RX_ACTIVE = false;
      if (timeout)
	{
	  state = TX_FINAL;
	  stateNum = 39;
	  timeout = false;
	}
      break;
    case TX_SHUTDOW:
      TX_ACTIVE = true;
      RX_ACTIVE = false;
      state = IDLE;
      stateNum = 0;
      break;
    }
  return(0);
}

//Instructions of the command Strobe processor

cl_CC2530_CSP::cl_CC2530_CSP()
{
  PC = 0;
  csp_stop = false;
  cell_cspstat->set_bit0(bmCSPrunning);
  waitingForAnswer = false;
  WaitingForOvfs = false;
  xram= uc->address_space(MEM_XRAM_ID);
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

  return(condition)
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
  cell_cspy->set(min(cell_cspy->get() + 1, M));
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
      W_b = W;
      waitWend = false;
    }
  if (W_b != 0)
    {
      fprintf(stderr, "Awaiting for %d Mac timer overflows...\n", W);
    } 
  else
    {
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
  N = ((code & 0x08)>>3 == 1)
  if (N)
    cond = !condition(C);
  else
    cond = condition(C);
    
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
  for (int i = 0; i < 128; i++)
    {
      RXFIFO[i] = 0;
      xram->set(0x6000 + i, 0);
    }
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_sflushtx(uchar code)
{
  for (int i = 0; i < 128; i++)
    {
      RTFIFO[i] = 0;
      xram->set(0x6080 + i, 0);
    }
  PC = PC + 1;
}

void
cl_CC2530_CSP::inst_sack(uchar code)
{//WITH PENDING FIELD CLEARED?
  if (!waitingForAnswer)
    {
      CC2530_radio->ack();
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
{//WITH PENDING FIELD SET?
  if (!waitingForAnswer)
    {
      CC2530_radio->ack();
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
cl_CC2530_CSP::inst_snack(uchar code)
{
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
  cell_cspstat->set_bit0(bmCSPrunning);
}

void
cl_CC2530_CSP::IRQ_CSP_WT(void)
{
  
}

void
cl_CC2530_CSP::IRQ_CSP_INT(void)
{
  
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
	case 6: inst_isack(code); break;
	case 7: inst_isackpend(code); break;
	case 8: inst_isnack(code); break;
	case 9: inst_istxon(code); break;
	case 10: inst_istxoncca(code); break;
	case 11: inst_issamplecca(code); break;
	case 12: break;
	case 13: inst_isflushrx(code); break;
	case 14: inst_isflushtx(code); break;
	case 15: inst_isrfoff(code); break;
	}
    }

  switch (code)
    {
    case 0xB8: inst_wait_event1(code); break;
    case 0xB9: inst_wait_event2(code); break;
    case 0xBA: inst_int(code); break;
    case 0xBB: inst_label(code); break;
    case 0xBC: inst_waitx(code); break;
    case 0xBD: inst_randxy(code); break;
    case 0xBE: inst_setcmp1(code); break;
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
  //post_inst();
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
	  cell_rfst->set(PCwrite, *val);
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
	      W_b = W_b - 1;
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

cl_CC2530_CSP::make_csp_watched_cells()
{
  register_cell(xram, CSPPROG0, &cell_cspprog0, wtd_restore_write);
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
  register_cell(xram, CSPPROG23, &cell_cspprog23, wtd_restore_write);

  register_cell(xram, CSPCTRL, &cell_cspctrl, wtd_restore_write);
  register_cell(xram, CSPSTAT, &cell_cspstat, wtd_restore_write);
  register_cell(xram, CSPX, &cell_cspx, wtd_restore_write);
  register_cell(xram, CSPY, &cell_cspy, wtd_restore_write);
  register_cell(xram, CSPZ, &cell_cspz, wtd_restore_write);
  register_cell(xram, CSPT, &cell_cspt, wtd_restore_write);

  register_cell(sfr, RFST, &cell_rfst, wtd_restore_write);
}

cl_CC2530_radio::make_radio_watched_cells()
{
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
  register_cell(xram, RXFIRST, &cell_rxfirst, wtd_restore_write);
  register_cell(xram, RXLAST, &cell_rxlast, wtd_restore_write);
  register_cell(xram, RXP1, &cell_rxp1, wtd_restore_write);
  register_cell(xram, TXFIRST, &cell_txfirst, wtd_restore_write);
  register_cell(xram, TXLAST, &cell_txlast, wtd_restore_write);
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
}
