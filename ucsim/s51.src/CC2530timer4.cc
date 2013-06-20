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
  freq=CC2530xtal;
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
  //overflow interrupt
  uc->it_sources->add(new cl_it_src(IEN1, bmT4IE, TIMIF, bmT4OVFIF, 0x0063, true,
				    "timer #1 overflow", 4));
  uc->it_sources->add(new cl_it_src(IEN1, bmT4IE, TIMIF, bmT4CH0IF, 0x0063, true,
				    "timer #1 Channel 0 interrupt", 4));
  uc->it_sources->add(new cl_it_src(IEN1, bmT4IE, TIMIF, bmT4CH1IF, 0x0063, true,
				    "timer #1 Channel 1 interrupt", 4));
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
      switch(*val>>5)
	{ 
	case 0: prescale= 1; break;
	case 1: prescale= 2; break;
	case 2: prescale= 4; break;
	case 3: prescale= 8; break;
	case 4: prescale= 16; break;
	case 5: prescale= 32; break;
	case 6: prescale= 64; break;
	case 7: prescale= 128; break;
	default: prescale=1; break;
	}
      freq= CC2530xtal/tickspd;
      fprintf(stderr,"Modification of %s control register.\n", id_string);
      fprintf(stderr,
	      "Prescale value: %d System clk division: %d Frequency: %g Hz Crystal: %g Hz\n",
	      prescale, tickspd, freq, CC2530xtal);
    }
  if (cell == cell_t4cc0)
    {
      //TRACE();
      tabCh[0].ValRegCMP =*val;
      fprintf(stderr, "Modif of cmp reg on channel 0: 0x%04x\n",tabCh[0].ValRegCMP);
    }
  else if (cell == cell_t4cc1)
    {
      //TRACE();
      tabCh[1].ValRegCMP =*val;
      fprintf(stderr, "Modif of cmp reg on channel 1: 0x%04x\n",tabCh[1].ValRegCMP);
    }
}

void
cl_CC2530_timer4::reset(void)
{
  cell_tl->write(0);
  ticks=0;
}


int
cl_CC2530_timer4::tick(int cycles)
{
  //TRACE();
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
cl_CC2530_timer4::TimerTick(int TimerTicks)
{
  //TRACE();
  cl_CC2530_timer::tick(TimerTicks);
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

/* End of s51.src/CC2530timer4.cc */
