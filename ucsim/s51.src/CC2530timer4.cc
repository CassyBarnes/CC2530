#include "CC2530timer4cl.h"
#include <assert.h>
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


cl_CC2530_timer4::cl_CC2530_timer4(class cl_uc *auc, int aid, char *aid_string):
  cl_CC2530_timer<char>(auc, aid, aid_string)
{
  //TRACE();
  addr_tl  = T4CNT;
  sfr= uc->address_space(MEM_SFR_ID);
  ChMax=2;
  init();
}


int
cl_CC2530_timer4::init(void)
{
  assert(sfr);
  TR=0;
  up_down=0;//0 => count up, 1=> count down
  tickspd=1;
  tickcount=0;
  TimerTicks=0;
  prescale=1;
  ticks=0;
  decal = 4;
  freq=CC2530xtal;
  systemTicks=0;
  OVFMaskMask = 0x08;
  OVFIFMask = 0x10;
  IrconFlag = 0x10;
  count =0;
  modes[0]= "Free running mode";
  modes[1]= "    Down Mode    ";
  modes[2]= "   Modulo mode   ";
  modes[3]= "  Up/down Mode   ";
  tabCh[0].ValRegCMP=sfr->read(T4CC0);

  register_cell(sfr, TIMIF, &cell_txstat, wtd_restore_write);
  register_cell(sfr, T4CTL, &cell_txctl, wtd_restore_write); 
  register_cell(sfr, T4CC0, &cell_t4cc0, wtd_restore_write);
  register_cell(sfr, T4CC1, &cell_t4cc1, wtd_restore_write);
  register_cell(sfr, T4CTL, &cell_OvfMaskReg, wtd_restore_write);
  register_cell(sfr, TIMIF, &cell_OvfFlagReg, wtd_restore_write);
  register_cell(sfr, TIMIF, &cell_FlagReg, wtd_restore_write);

  cell_tl = NULL;
  use_cell(sfr, addr_tl, &cell_tl, wtd_restore);
  assert(cell_tl);
  cell_th = NULL;

  tabCh[0]={0, 0, T4CCTL0, T4CC0, NULL, sfr->read(T4CC0)};
  tabCh[1]={0, 0, T4CCTL1, T4CC1, NULL, sfr->read(T4CC1)};

  return(0);
}

void
cl_CC2530_timer4::added_to_uc(void)
{
  //Timer interrupt
  uc->it_sources->add(new cl_it_src(IEN1, bmT4IE, IRCON, bmT4IF, 0x0063, true,
				    "timer #4 overflow", 4));
}

void
cl_CC2530_timer4::write(class cl_memory_cell *cell, t_mem *val)
{
  //TRACE();
  cl_CC2530_timer::write(cell, val);
  //TRACE();
  if (cell == cell_txctl)
    {
      //TRACE();
	TR=*val & 0x10;
      if (*val & 0x04 == 1)
	reset();
      prescale = 1 << (*val>>5);
      freq= CC2530xtal/tickspd;
      fprintf(stderr,"Modification of %s control register.\n", id_string);
      fprintf(stderr,
	      "Prescale value: %d Tick Speed: /%d Frequency: %g Hz Crystal: %d Hz\n",
	      prescale, tickspd, freq, CC2530xtal);
    }
  if (cell == cell_t4cc0)
    {
      //TRACE();
      tabCh[0].ValRegCMP =*val;
      fprintf(stderr, "Modif of cmp reg on channel 0: 0x%04x\n",tabCh[0].ValRegCMP);
      get_next_cc_event();
    }
  else if (cell == cell_t4cc1)
    {
      //TRACE();
      tabCh[1].ValRegCMP =*val;
      fprintf(stderr, "Modif of cmp reg on channel 1: 0x%04x\n",tabCh[1].ValRegCMP);
      get_next_cc_event();
    }
}

void
cl_CC2530_timer4::reset(void)
{
  cell_tl->write(0);
  ticks=0;
}


void
cl_CC2530_timer4::CaptureCompare(void)
{
  TRACE();
  for (int i=0; i<ChMax; i++)
    {
      if ((sfr->read(tabCh[i].RegCTL) & bmTimerMode) == 0)//capt enabled
	{
	  captureMode = sfr->read(tabCh[i].RegCTL) & bmCaptMode;
	  capt = cl_CC2530_timer::Capture(tabCh[i].IOPin, tabCh[i].ExIOPin, captureMode);
	  if (capt == true)
	    {
	      sfr->write(tabCh[i].RegCMPL, sfr->read(T1CNTL));//fixme
	      fprintf(stderr,"\nCount: 0x%04x\n", count);
	      fprintf(stderr,"\nCapture: in %s of value: 0x%04x\n\n", id_string, 
		      tabCh[i].ValRegCMP);
	      int flag=1<<i;
	      if ((sfr->read(tabCh[i].RegCTL) & 0x40) != 0)
		{
		  cell_FlagReg->set_bit1(flag << decal);
		  cell_ircon->set_bit1(IrconFlag);
		}
	    }
	}
      else//compare mode
	{
	  if ((count == tabCh[0].ValRegCMP)
	      ||(count == tabCh[i].ValRegCMP)
	      ||(count == 0))
	    {
	      ////TRACE();
	      tabCh[i].IOPin = Compare(tabCh[i].IOPin, tabCh[i].RegCTL, tabCh[i].ValRegCMP);
	      flagsReg = sfr->read(0x8E);
	      flagsReg = flagsReg | (0x200 << i);
	      sfr->write(0x8E, flagsReg);
	      cc=1;
	    }
	}
    }
  if (cc==1)
    {
      cl_CC2530_timer::print_info();
    }
  get_next_cc_event();
}

void
cl_CC2530_timer4::TimerTick(int TimerTicks)
{
  TRACE();
  //cl_CC2530_timer::tick(TimerTicks);
  //TRACE();

  if (TR != 0)
    {
      switch (mode)
	{
	case 0: cl_CC2530_timer::do_FreeRunningMode(TimerTicks); break;
	case 1: cl_CC2530_timer::do_DownMode(TimerTicks); break;
	case 2: cl_CC2530_timer::do_ModuloMode(TimerTicks); break;
	case 3: cl_CC2530_timer::do_UpDownMode(TimerTicks); break;
	}
    }
}

void
cl_CC2530_timer4::get_next_cc_event()
{
  NextCmpEvent = 0xFF;
  for (int i=0; i<2; i++)
    {
      int valRegCTL;
      TRACE();	
      valRegCTL = (sfr->read(tabCh[i].RegCTL)) & 0x4;
      if ((valRegCTL) != 0)
	{
	  fprintf(stderr, "Channel %d: Compare enabled? %s\n", i, 
		  (valRegCTL != 0)?"1":"0");
	  if ((mode == 1) || ((mode == 3) && (up_down == 1)))
	    cmpEventIn = count - tabCh[i].ValRegCMP;
	  else
	    cmpEventIn = tabCh[i].ValRegCMP - count;
	  if (((cmpEventIn > 0) && (cmpEventIn < NextCmpEvent)
		   || ((NextCmpEvent == -1) && (cmpEventIn != 0))))
	    NextCmpEvent = cmpEventIn;
	  fprintf(stderr, "Channel %d: Compare event in %d Timer ticks...\n", i, 
		  cmpEventIn);
	}
    }
  fprintf(stderr, "Next compare event in %d Timer ticks...\n", NextCmpEvent);
}

/* End of s51.src/CC2530timer4.cc */
