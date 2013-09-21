#include "CC2530WDTcl.h"
#include <assert.h>
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"

#define bmINT 0x03
#define bmMODE 0x0C
#define bmWDTIE 0x20
#define bmWDTIF 0x20

cl_CC2530_WDT::cl_CC2530_WDT(class cl_uc *auc, int aid, char *aid_string):
  cl_hw(auc, HW_TIMER, aid, aid_string)
{
  sfr= uc->address_space(MEM_SFR_ID);
  freq = 32768;
  CC2530freq = 32000000;
  count = 0;//16 bits
  init();
}

int
cl_CC2530_WDT::init(void)
{
 register_cell(sfr, WDCTL, &cell_wdctl, wtd_restore_write);
 register_cell(sfr, CLKCONCMD, &cell_clkconcmd, wtd_restore_write);
 register_cell(sfr, IRCON2, &cell_ircon2, wtd_restore_write);
}

ulong
cl_CC2530_WDT::get_rtime(void)
{
  return(MemElapsedTime + systemTicks/freq);
}

void
cl_CC2530_WDT::added_to_uc(void)
{
  uc->it_sources->add(new cl_it_src(IEN2, bmWDTIE, IRCON2, bmWDTIF, 0x008b, true,
				    "WDT overflow", 4));
}

int
cl_CC2530_WDT::tick(int cycles)
{
  TimerTicks=0;
  for (int i = 0; i<cycles; i++)
    {
      systemTicks++;
      if (systemTicks ==  TickINT)
	{
	  TimerTicks++;
	  systemTicks = 0;
	}
    }
  if (TimerTicks != 0)
    TimerTick(TimerTicks);
  return(resGO);
}

void
cl_CC2530_WDT::TimerTick(int TimerTicks)
{
  if (enabled != 0)
    {
      switch (mode)
	{
	case 0: break;
	case 1: break;
	case 2: cl_CC2530_WDT::do_WDTMode(TimerTicks); break;
	case 3: cl_CC2530_WDT::do_TimerMode(TimerTicks); break;
	}
    }
}

void
cl_CC2530_WDT::reset(void)
{
  enabled = false;
}

void
cl_CC2530_WDT::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_wdctl)
    {
      switch((*val & bmMODE)>>2) 
	{
	case 0:
	  if (mode != 2)
	    mode = 0;//IDLE mode
	  break;
	case 1:
	  if (mode != 2)
	  break;
	case 2:
	  enabled = true;
	  break;
	case 3:
	  enabled = true;
	  mode = 3;//timer mode
	  break;
	}
      if (mode == 0)
	{
	  switch(*val & bmINT) 
	    {
	    case 0: interval = 32768; break;
	    case 1: interval = 8192; break;
	    case 2: interval = 512; break;
	    case 3: interval = 64; break;
	    }
	}
    }
  else if (cell == cell_clkconcmd)
    {
      switch(*val & 0x07)
	{ 
	case 0: clkspd= 1; break;
	case 1: clkspd= 2; break;
	case 2: clkspd= 4; break;
	case 3: clkspd= 8; break;
	case 4: clkspd= 16; break;
	case 5: clkspd= 32; break;
	case 6: clkspd= 64; break;
	case 7: clkspd= 128; break;
	default: clkspd=1; break;
	}
      MemElapsedTime = get_rtime();
      MemSystemTicks = systemTicks;
      TickINT = 32000000/(clkspd*32768);//Tick every ... SystemTicks (@32MHz)
      systemTicks = 0;
      freq = CC2530xtal/(clkspd);
    }
}

int
cl_CC2530_WDT::do_TimerMode(int cycles)//Mode 3: Timer counts from 0 to FFFF
{
  //TRACE();

  //While exec of cycles-- !=0, repeat add(1) cycles time
  while (cycles--)
    {
      //TRACE();
      count++;
      if (count==0)
	{
	  cell_ircon2->set_bit1(bmWDTIF);//WD timer overflow flag
	  overflow();
	}
      if (count == interval)
	{
	cell_ircon2->set_bit1(bmWDTIF);
	count == 0;
	reset();//SYSTEM RESET
	}
    }
  return(0);
}

int
cl_CC2530_WDT::overflow(void)
{

}

int
cl_CC2530_WDT::do_WDTMode(int cycles)//Mode 3: Timer counts from 0 to FFFF
{
  while (cycles--)
    {
      //TRACE();
      count++;
      if (count == interval)
	{
	count == 0;
	reset();//SYSTEM RESET
	}
    }
  return(0);
}
