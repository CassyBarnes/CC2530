#include "CC2530timer1cl.h"
#include <assert.h>
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"

//#define TESTING
#define T1INFO
#undef DEBUG
#ifdef T1INFO
#define DEBUG
#endif
#ifdef DEBUG
#define TRACE() \
fprintf(stderr, "%s:%d in %s()\n", __FILE__, __LINE__, __FUNCTION__)
#else
#define TRACE()
#endif

#ifndef CC2530xtal
#define CC2530xtal 32000000
#endif

cl_CC2530_timer1::cl_CC2530_timer1(class cl_uc *auc, int aid, char *aid_string):
  cl_CC2530_timer<short int>(auc, aid, aid_string)
{
  addr_tl  = T1CNTL;
  addr_th  = T1CNTH;
  sfr = uc->address_space(MEM_SFR_ID);
  xram = uc->address_space(MEM_XRAM_ID);
  ChMax=5;
  init();
}


int
cl_CC2530_timer1::init(void)
{
  assert(sfr);
  up_down=0;//0 => count up, 1=> count down
  tickspd=1;
  tickcount=0;
  systemTicks=0;
  prescale=1;
  ticks=0;
  freq=CC2530xtal;
  OVFMaskMask = 0x40;
  OVFIFMask = 0x20;
  IrconFlag = 0x02;
  count=0;
  modes[0]= "  Timer stopped  ";
  modes[1]= "Free running mode";
  modes[2]= "   Modulo mode   ";
  modes[3]= "  Up/down Mode   ";
  tabCh[0].ValRegCMP=256*sfr->read(T1CC0H)+sfr->read(T1CC0L);
      fprintf(stderr,"Short int : %d Bytes, char: %d Bytes.\n\n",
	      sizeof(short int), sizeof(char));

  register_cell(sfr, T1STAT, &cell_txstat, wtd_restore_write);
  register_cell(sfr, T1CTL, &cell_txctl, wtd_restore_write); 
  register_cell(sfr, T1CC0H, &cell_t1cc0h, wtd_restore_write);
  register_cell(sfr, T1CC0L, &cell_t1cc0l, wtd_restore_write);
  register_cell(sfr, T1CC1H, &cell_t1cc1h, wtd_restore_write);
  register_cell(sfr, T1CC1L, &cell_t1cc1l, wtd_restore_write);
  register_cell(sfr, T1CC2H, &cell_t1cc2h, wtd_restore_write);
  register_cell(sfr, T1CC2L, &cell_t1cc2l, wtd_restore_write);
  register_cell(xram, T1CC3H, &cell_t1cc3h, wtd_restore_write);
  register_cell(xram, T1CC3L, &cell_t1cc3l, wtd_restore_write);
  register_cell(xram, T1CC4H, &cell_t1cc4h, wtd_restore_write);
  register_cell(xram, T1CC4L, &cell_t1cc4l, wtd_restore_write);
  register_cell(sfr, TIMIF, &cell_OvfMaskReg, wtd_restore_write);
  register_cell(sfr, T1STAT, &cell_OvfFlagReg, wtd_restore_write);
  register_cell(sfr, T1STAT, &cell_FlagReg, wtd_restore_write);

  //cell_tl = NULL;
  use_cell(sfr, addr_tl, &cell_tl, wtd_restore);
  //assert(cell_tl);
  use_cell(sfr, addr_th, &cell_th, wtd_restore);

  tabCh[0]={0, 0, T1CCTL0, T1CC0L, T1CC0H, (sfr->read(T1CC0H)<<8)+sfr->read(T1CC0L)};
  tabCh[1]={0, 0, T1CCTL1, T1CC1L, T1CC1H, (sfr->read(T1CC1H)<<8)+sfr->read(T1CC1L)};
  tabCh[2]={0, 0, T1CCTL2, T1CC2L, T1CC2H, (sfr->read(T1CC2H)<<8)+sfr->read(T1CC2L)};
  tabCh[3]={0, 0, T1CCTL3, T1CC3L, T1CC3H,(xram->read(T1CC3H)<<8)+xram->read(T1CC3L)};
  tabCh[4]={0, 0, T1CCTL4, T1CC4L, T1CC4H,(xram->read(T1CC4H)<<8)+xram->read(T1CC4L)};
  
  return(0);
}

void
cl_CC2530_timer1::added_to_uc(void)
{
  //Timer interrupt
  uc->it_sources->add(new cl_it_src(IEN1, bmT1IE, IRCON, bmT1IF, 0x004b, true,
				    "timer #1 interrupt", 4));
}

void
cl_CC2530_timer1::reset(void)
{
  cell_tl->write(0);
  cell_th->write(0);
  ticks=0;
}

void
cl_CC2530_timer1::CaptureCompare(void)
{
  cc=0;
  TRACE();
  for (int i=0; i<3; i++)
    {
      //Possible capture/compare cases
      if ((sfr->read(tabCh[i].RegCTL) & 0x04) == 0)//capt enabled
	{
	  captureMode = sfr->read(tabCh[i].RegCTL) & 0x03;
	  capt=Capture(tabCh[i].IOPin, tabCh[i].ExIOPin, captureMode);
	  if (capt == true)
	    {
	      if (cell_th != NULL)
		sfr->write(tabCh[i].RegCMPH, count>>8);
	      sfr->write(tabCh[i].RegCMPL, count & 0xFF);
#ifdef T1INFO
	      fprintf(stderr,"\nCount: 0x%04x\n",count);
	      fprintf(stderr,"\nCapture: in %s of value: 0x%04x\n\n", 
		      id_string, tabCh[i].ValRegCMP);
#endif
	      int flag=1<<i;
	      if ((sfr->read(tabCh[i].RegCTL) & 0x40) != 0)
		cell_FlagReg->set_bit1(flag);
	    }
	}
      else//compare mode
	{
#ifdef T1INFO
	  fprintf(stderr, "Channel %d in compare mode...\n", i);
	  fprintf(stderr, "Count: 0x%04x\n", count);
	  fprintf(stderr, "Compare reg val: 0x%04x\n", tabCh[i].ValRegCMP);
	  fprintf(stderr, "Channel 0 Compare reg val: 0x%04x\n", tabCh[0].ValRegCMP);
#endif
	  if ((count == tabCh[0].ValRegCMP)
	      ||(count == tabCh[i].ValRegCMP)
	      ||(count == 0))
	    {
	      ////TRACE();
	      tabCh[i].IOPin=Compare(tabCh[i].IOPin, tabCh[i].RegCTL, tabCh[i].ValRegCMP);
	      flagsReg = sfr->read(0x8E);
	      flagsReg = flagsReg | (0x04 << i);
	      sfr->write(0x8E, flagsReg);
#ifdef T1INFO
	      fprintf(stderr, "Event flag: 0x%04x\n", flagsReg);
#endif
	      cc=1;
	    }
	}
    }
  for (int i=3; i<5; i++)//xram read instead of sfr read for T1CC3 and T1CC4
    {
      //Possible capture/compare cases
      if ((xram->read(tabCh[i].RegCTL) & 0x04) == 0)//capt enabled
	{
	  captureMode = xram->read(tabCh[i].RegCTL) & 0x03;
	  capt=Capture(tabCh[i].IOPin, tabCh[i].ExIOPin, captureMode);
	  if (capt == true)
	    {
	      xram->write(tabCh[i].RegCMPH, count>>8);
	      xram->write(tabCh[i].RegCMPL, count & 0xFF);
	      int flag=1<<i;
	      if ((xram->read(tabCh[i].RegCTL) & 0x40) != 0)
		{
		  cell_FlagReg->set_bit1(flag);
		  cell_ircon->set_bit1(IrconFlag);
		}
	    }
	}
      else
	{
	  if ((count == tabCh[0].ValRegCMP)
	      ||(count == tabCh[i].ValRegCMP)
	      ||(count == 0))
	    {
	      ////TRACE();
	      tabCh[i].IOPin = Compare(tabCh[i].IOPin, tabCh[i].RegCTL, tabCh[i].ValRegCMP);
	      cc=1;
	    }
	}
    }
  if (cc==1)
    {
      cl_CC2530_timer::print_info();
    }
  TRACE();
  get_next_cc_event();
}


void
cl_CC2530_timer1::write(class cl_memory_cell *cell, t_mem *val)
{
  cl_CC2530_timer::write(cell, val);

 if (cell == cell_txctl) // correspond to TxCTL register (sfr->read(T1CTL))
   {
     if (((*val & bmDIV)>>2) != 0)
       prescale = 1 << (1 + ((*val & bmDIV)>>1));//2 shifted 2, 4 or 6 times to left
     else 
       prescale = 1;
#ifdef T1INFO
      fprintf(stderr,"Modification of %s control register.\n", id_string);
      fprintf(stderr,
	      "Prescale value: %d Tick Speed: /%d Frequency: %g Hz Crystal: %d Hz\n",
	      prescale, tickspd, freq, CC2530xtal);
#endif
    }

  if (cell == cell_t1cc0h)
    {
      channelID = 0;
      high = true;
    }
  else if (cell == cell_t1cc0l)
    {
      channelID = 0;
      high = false;
    }
  else if (cell == cell_t1cc1h)
    {
      channelID = 1;
      high = true;
    }
  else if (cell == cell_t1cc1l)
    {
      channelID = 1;
      high = false;
    }
  else if (cell == cell_t1cc2h)
    {
      channelID = 2;
      high = true;
    }
  else if (cell == cell_t1cc2l)
    {
      channelID = 2;
      high = false;
    }
  else if (cell == cell_t1cc3h)
    {
      channelID = 3;
      high = true;
    }
  else if (cell == cell_t1cc3l)
    {
      channelID = 3;
      high = false;
    }
  else if (cell == cell_t1cc4h)
    {
      channelID = 4;
      high = true;
    }
  else if (cell == cell_t1cc4l)
    {
      channelID = 4;
      high = false;
    }
  if (high)
    tabCh[channelID].ValRegCMP = (tabCh[channelID].ValRegCMP & 0xFF) + ((*val) << 8);
  else
    tabCh[channelID].ValRegCMP = (tabCh[channelID].ValRegCMP & 0xFF00) + *val;
  TRACE();
  get_next_cc_event();
}


void
cl_CC2530_timer1::TimerTick(int TimerTicks)
{
  //cl_CC2530_timer::tick(TimerTicks);
fprintf(stderr, "T1 count: %d\n",count);
#ifdef TESTING
  if ((sfr->read(T1CCTL1) & 0x04) == 0)
    {
      tickcount += TimerTicks;
#ifdef T1INFO
      fprintf(stderr, "Tickcount: %d\n",tickcount);
#endif
      if((tickcount % 3) == 0)
	{
	  PinEvent = true;
	  if (tabCh[1].IOPin == 0)
	    tabCh[1].IOPin=1;
	  else
	    tabCh[1].IOPin=0;
#ifdef T1INFO
	  fprintf(stderr, "Change of IOPinCH1: %d\n",tabCh[1].IOPin);
#endif
	}
    }
#endif

  TRACE();
  switch (mode)
    {
    case 0: cl_CC2530_timer::do_Stop(TimerTicks); break;
    case 1: cl_CC2530_timer::do_FreeRunningMode(TimerTicks); break;
    case 2: cl_CC2530_timer::do_ModuloMode(TimerTicks); break;
    case 3: cl_CC2530_timer::do_UpDownMode(TimerTicks); break;
    }
}


void
cl_CC2530_timer1::get_next_cc_event()
{
  NextCmpEvent = 0xFFFF;
  for (int i=0; i<ChMax; i++)
    {
      int valRegCTL;
      TRACE();
      if (i<3)
	{
	  valRegCTL = (sfr->read(tabCh[i].RegCTL)) & 0x4;
	  if ((valRegCTL) != 0)
	    {
#ifdef T1INFO
	      fprintf(stderr, "Channel %d: Compare enabled? %s\n", i, (valRegCTL != 0)?"1":"0");
#endif
	  if ((mode == 3) && (up_down == 1))
	    cmpEventIn = count - tabCh[i].ValRegCMP;
	  else
	    cmpEventIn = tabCh[i].ValRegCMP - count;
	      if (((cmpEventIn > 0) && (cmpEventIn < NextCmpEvent)
		   || ((NextCmpEvent == -1) && (cmpEventIn != 0))))
		NextCmpEvent = cmpEventIn;
#ifdef T1INFO
	      fprintf(stderr, "Channel %d: Compare event in %d Timer ticks... %d\n", i, cmpEventIn, NextCmpEvent);
#endif
	    }
	}
      else
	{
	  TRACE();
	  valRegCTL = (xram->read(tabCh[i].RegCTL)) & 0x4;
	  if (valRegCTL != 0)
	    {
#ifdef T1INFO
	      fprintf(stderr, "Channel %d: Compare enabled? %d\n", i,(valRegCTL != 0)?"1":"0");
#endif
	      cmpEventIn = tabCh[i].ValRegCMP - count;
	      if ((cmpEventIn > 0) && (cmpEventIn < NextCmpEvent))
		NextCmpEvent = cmpEventIn;
#ifdef T1INFO
	      fprintf(stderr, "Channel %d: Compare event in %d Timer ticks...\n", i, cmpEventIn);
#endif
	    }
	}
    }
#ifdef T1INFO
  fprintf(stderr, "Next compare event in %d Timer ticks...\n", NextCmpEvent);
#endif
}


/* End of s51.src/CC2530timer1.cc */
