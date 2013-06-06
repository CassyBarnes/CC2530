#include "CC2530timer1cl.h"
#include <assert.h>
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"

#define TESTING

#define DEBUG
#ifdef DEBUG
#define TRACE() \
fprintf(stderr, "%s:%d in %s()\n", __FILE__, __LINE__, __FUNCTION__)
#else
#define TRACE()
#endif


cl_CC2530_timer1::cl_CC2530_timer1(class cl_uc *auc, int aid, char *aid_string):
  cl_CC2530_timer<short int>(auc, aid, aid_string)
{
  addr_tl  = T1CNTL;
  addr_th  = T1CNTH;
  sfr= uc->address_space(MEM_SFR_ID);
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
  count=0;
  modes[0]= "  Timer stopped  ";
  modes[1]= "Free running mode";
  modes[2]= "   Modulo mode   ";
  modes[3]= "  Up/down Mode   ";
  tabCh[0].ValRegCMP=256*sfr->read(T1CC0H)+sfr->read(T1CC0L);

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

  cell_tl = NULL;
  use_cell(sfr, addr_tl, &cell_tl, wtd_restore);
  assert(cell_tl);
  use_cell(sfr, addr_th, &cell_th, wtd_restore);

  tabCh[0]={0, 0, T1CCTL0, T1CC0L, T1CC0H, 256*sfr->read(T1CC0H)+sfr->read(T1CC0L)};
  tabCh[1]={0, 0, T1CCTL1, T1CC1L, T1CC1H, 256*sfr->read(T1CC1H)+sfr->read(T1CC1L)};
  tabCh[2]={0, 0, T1CCTL2, T1CC2L, T1CC2H, 256*sfr->read(T1CC2H)+sfr->read(T1CC2L)};
  tabCh[3]={0, 0, T1CCTL3, T1CC3L, T1CC3H,256*xram->read(T1CC3H)+xram->read(T1CC3L)};
  tabCh[4]={0, 0, T1CCTL4, T1CC4L, T1CC4H,256*xram->read(T1CC4H)+xram->read(T1CC4L)};
  
  return(0);
}

void
cl_CC2530_timer1::reset(void)
{
  cell_tl->write(0);
  cell_th->write(0);
  ticks=0;
}

void
cl_CC2530_timer1::write(class cl_memory_cell *cell, t_mem *val)
{
  cl_CC2530_timer::write(cell, val);

 if (cell == cell_txctl) // correspond to TxCTL register (sfr->read(T1CTL))
    {
      switch((*val & bmDIV)>>2)
	{ 
	case 0: prescale= 1; break;
	case 1: prescale= 8; break;
	case 2: prescale= 32; break;
	case 3: prescale= 128; break;
	default: prescale=1; break;
	}
      fprintf(stderr,"Modification of %s control register.\n", id_string);
      fprintf(stderr,
	      "Prescale value: %d System clk division: %d Frequency: %g Hz Crystal: %g Hz\n",
	      prescale, tickspd, freq, CC2530xtal);
    }

  if (cell == cell_t1cc0h)
    {
      tabCh[0].ValRegCMP = (tabCh[0].ValRegCMP & 0xFF) + 256*(*val);
    }
  else if (cell == cell_t1cc0l)
    {
      tabCh[0].ValRegCMP = (tabCh[0].ValRegCMP & 0xFF00) + *val;
    }
  else if (cell == cell_t1cc1h)
    {
      tabCh[1].ValRegCMP = (tabCh[1].ValRegCMP & 0xFF) + 256*(*val); 
    }
  else if (cell == cell_t1cc1l)
    {
      tabCh[1].ValRegCMP = (tabCh[1].ValRegCMP & 0xFF00) + *val;
    }
  else if (cell == cell_t1cc2h)
    {
      tabCh[2].ValRegCMP = (tabCh[2].ValRegCMP & 0xFF) + 256*(*val);
    }
  else if (cell == cell_t1cc2l)
    {
      tabCh[2].ValRegCMP = (tabCh[2].ValRegCMP & 0xFF00) + *val;
    }
  else if (cell == cell_t1cc3h)
    {
      tabCh[3].ValRegCMP = (tabCh[3].ValRegCMP & 0xFF) + 256*(*val);
    }
  else if (cell == cell_t1cc3l)
    {
      tabCh[3].ValRegCMP = (tabCh[3].ValRegCMP & 0xFF00) + *val;
    }
  else if (cell == cell_t1cc4h)
    {
      tabCh[4].ValRegCMP = (tabCh[4].ValRegCMP & 0xFF) + 256*(*val);
    }
  else if (cell == cell_t1cc4l)
    {
      tabCh[4].ValRegCMP = (tabCh[4].ValRegCMP & 0xFF00) + *val;
    }

}

int
cl_CC2530_timer1::tick(int cycles)
{
  TimerTicks=0;
  for (int i = 0; i<cycles; i++)
    {
      systemTicks++;
      if (((int)systemTicks % prescale) == 0)
	TimerTicks++;
    }
  if (TimerTicks != 0)
    TimerTick(TimerTicks);
  return(resGO);
}

void
cl_CC2530_timer1::TimerTick(int TimerTicks)
{
  cl_CC2530_timer::tick(TimerTicks);

  #ifdef TESTING
  if ((sfr->read(T1CCTL1) & 0x04) == 0)
    {
      tickcount += TimerTicks;
      fprintf(stderr, "Tickcount: %d\n",tickcount);
      if((tickcount % 3) == 0)
	{
	  if (tabCh[1].IOPin == 0)
	    tabCh[1].IOPin=1;
	  else
	    tabCh[1].IOPin=0;
	  fprintf(stderr, "Change of IOPinCH1: %d\n",tabCh[1].IOPin);
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


/* End of s51.src/CC2530timer1.cc */
