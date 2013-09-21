#include "CC2530timer3cl.h"
#include <assert.h>
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"

#ifdef T3INFO
#define DEBUG
#endif
#ifdef DEBUG
#define TRACE() \
fprintf(stderr, "%s:%d in %s()\n", __FILE__, __LINE__, __FUNCTION__)
#else
#define TRACE()
#endif


cl_CC2530_timer3::cl_CC2530_timer3(class cl_uc *auc, int aid, char *aid_string):
  cl_CC2530_timer<char>(auc, aid, aid_string)
{
  //TRACE();
  make_partner(HW_CC2530_DMA, 1);
  addr_tl  = T3CNT;
  sfr= uc->address_space(MEM_SFR_ID);
  ChMax=2;
  init();
}


int
cl_CC2530_timer3::init(void)
{
  //TRACE();
  assert(sfr);
  TR=0;
  up_down=0;//0 => count up, 1=> count down
  tickspd=1;
  tickcount=0;
  TimerTicks=0;
  prescale=1;
  ticks=0;
  decal = 1;
  freq=CC2530xtal;
  systemTicks=0;
  OVFMaskMask = 0x08;
  OVFIFMask = 0x08;
  IrconFlag = 0x08;
  count =0;
  modes[0]= "Free running mode";
  modes[1]= "    Down Mode    ";
  modes[2]= "   Modulo mode   ";
  modes[3]= "  Up/down Mode   ";
  tabCh[0].ValRegCMP=sfr->read(T3CC0);

  register_cell(sfr, TIMIF, &cell_txstat, wtd_restore_write);
  register_cell(sfr, T3CTL, &cell_txctl, wtd_restore_write); 
  register_cell(sfr, T3CC0, &cell_t3cc0, wtd_restore_write);
  register_cell(sfr, T3CC1, &cell_t3cc1, wtd_restore_write);
  register_cell(sfr, T3CTL, &cell_OvfMaskReg, wtd_restore_write);
  register_cell(sfr, TIMIF, &cell_OvfFlagReg, wtd_restore_write);
  register_cell(sfr, TIMIF, &cell_FlagReg, wtd_restore_write);

  cell_tl = NULL;
  use_cell(sfr, addr_tl, &cell_tl, wtd_restore);
  assert(cell_tl);
  cell_th = NULL;

  tabCh[0].IOPin     = bool(0);
  tabCh[0].ExIOPin   = bool(0);
  tabCh[0].RegCTL    = t_addr(T3CCTL0);
  tabCh[0].RegCMPL   = t_addr(T3CC0);
  tabCh[0].RegCMPH   = t_addr(NULL);
  tabCh[0].ValRegCMP = sfr->read(T3CC0);

  tabCh[1].IOPin     = bool(0);
  tabCh[1].ExIOPin   = bool(0);
  tabCh[1].RegCTL    = t_addr(T3CCTL1);
  tabCh[1].RegCMPL   = t_addr(T3CC1);
  tabCh[1].RegCMPH   = t_addr(NULL);
  tabCh[1].ValRegCMP = sfr->read(T3CC1);

  return(0);
}

void
cl_CC2530_timer3::added_to_uc(void)
{
  //overflow interrupt
  uc->it_sources->add(new cl_it_src(IEN1, bmT3IE, IRCON, bmT3IF, 0x005b, true,
				    "timer #3 interrupt", 4));
}

void
cl_CC2530_timer3::write(class cl_memory_cell *cell, t_mem *val)
{
  //TRACE();
  cl_CC2530_timer<char>::write(cell, val);
  //TRACE();
  if (cell == cell_txctl)
    {
      //TRACE();
      TR=*val & 0x10;
      if ((*val & 0x04) == 1)
	reset();
      prescale = 1 << (*val>>5);
      freq= (CC2530xtal/tickspd);
#ifdef T3INFO
      fprintf(stderr,"Modification of %s control register.\n", id_string);
      fprintf(stderr,
	      "Prescale value: %d Tick Speed: /%d Frequency: %g Hz Crystal: %d Hz\n",
	      prescale, tickspd, freq, CC2530xtal);
#endif
    }
  if (cell == cell_t3cc0)
    {
      //TRACE();
      tabCh[0].ValRegCMP =*val;
#ifdef T3INFO
      fprintf(stderr, "Modif of cmp reg on channel 0: 0x%04x\n",tabCh[0].ValRegCMP);
#endif
      get_next_cc_event();
    }
  else if (cell == cell_t3cc1)
    {
      //TRACE();
      tabCh[1].ValRegCMP =*val;
#ifdef T3INFO
      fprintf(stderr, "Modif of cmp reg on channel 1: 0x%04x\n",tabCh[1].ValRegCMP);
#endif
      get_next_cc_event();
    }
}

void
cl_CC2530_timer3::reset(void)
{
  cell_tl->write(0);
  ticks=0;
}

void
cl_CC2530_timer3::CaptureCompare(void)
{
  cc = 0;
  TRACE();
  for (int i=0; i<ChMax; i++)
    {
      if ((sfr->read(tabCh[i].RegCTL) & bmTimerMode) == 0)//capt enabled
	{
	  captureMode = sfr->read(tabCh[i].RegCTL) & bmCaptMode;
	  capt = Capture(tabCh[i].IOPin, tabCh[i].ExIOPin, captureMode);
	  if (capt == true)
	    {
	      sfr->write(tabCh[i].RegCMPL, sfr->read(T1CNTL));//fixme
#ifdef T3INFO
	      fprintf(stderr,"\nCount: 0x%04x\n", count);
	      fprintf(stderr,"\nCapture: in %s of value: 0x%04x\n\n", id_string, 
		      tabCh[i].ValRegCMP);
#endif
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
	      if (i == 0)
		inform_partners(EV_T3_CH0, 0);
	      else
		inform_partners(EV_T3_CH1, 0);

	      cc=1;
#ifdef T3INFO
	      fprintf(stderr, "Channel %d in compare mode...\n", i);
	      fprintf(stderr, "Count: 0x%04x\n", count);
	      fprintf(stderr, "Compare reg val: 0x%04x\n", tabCh[i].ValRegCMP);
	      fprintf(stderr, "Channel 0 Compare reg val: 0x%04x\n", tabCh[0].ValRegCMP);
#endif
	    }
	}
    }
  if (cc==1)
    {
      cl_CC2530_timer<char>::print_info();
    }
  get_next_cc_event();
}


void
cl_CC2530_timer3::TimerTick(int TimerTicks)
{
  // TRACE();
  //cl_CC2530_timer<char>::tick(TimerTicks);
  //TRACE();

  if (TR != 0)
    {

      switch (mode)
	{
	case 0: cl_CC2530_timer<char>::do_FreeRunningMode(TimerTicks); break;
	case 1: cl_CC2530_timer<char>::do_DownMode(TimerTicks); break;
	case 2: cl_CC2530_timer<char>::do_ModuloMode(TimerTicks); break;
	case 3: cl_CC2530_timer<char>::do_UpDownMode(TimerTicks); break;
	}
    }
}


void
cl_CC2530_timer3::get_next_cc_event()
{
  NextCmpEvent = 0xFF;
  for (int i=0; i<2; i++)
    {
      int valRegCTL;
      //      TRACE();	
      valRegCTL = (sfr->read(tabCh[i].RegCTL)) & 0x4;
      if ((valRegCTL) != 0)
	{
#ifdef T3INFO
	  fprintf(stderr, "Channel %d: Compare enabled? %s\n", i, 
		  (valRegCTL != 0)?"1":"0");
#endif
	  if ((mode == 1) || ((mode == 3) && (up_down == 1)))
	    cmpEventIn = count - tabCh[i].ValRegCMP;
	  else
	    cmpEventIn = tabCh[i].ValRegCMP - count;
	  if ( ((cmpEventIn > 0) && (cmpEventIn < NextCmpEvent))
	       || ((NextCmpEvent == -1) && (cmpEventIn != 0)) )
	    NextCmpEvent = cmpEventIn;
#ifdef T3INFO
	  fprintf(stderr, "Channel %d: Compare event in %d Timer ticks...\n", i, cmpEventIn);
#endif
	}
    }
#ifdef T3INFO
  fprintf(stderr, "Next compare event in %d Timer ticks...\n", NextCmpEvent);
#endif
}

/* End of s51.src/CC2530timer3.cc */
