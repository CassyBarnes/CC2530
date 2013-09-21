#include "CC2530sleepTimercl.h"
#include <assert.h>
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"

#define bmLDRDY 0x01
#define bmSTIE 0x20
#define bmSTIF 0x80

cl_CC2530_sleep_timer::cl_CC2530_sleep_timer(class cl_uc *auc, int aid, char *aid_string):cl_hw(auc, HW_TIMER, aid, aid_string)
{
  sfr= uc->address_space(MEM_SFR_ID);
}

int
cl_CC2530_sleep_timer::init(void)
{
  count = 0;
  TimerTicks=0;
  ticks=0;
  freq=CC2530xtal;
  register_cell(sfr, ST0, &cell_st0, wtd_restore_write);
  register_cell(sfr, ST1, &cell_st1, wtd_restore_write);
  register_cell(sfr, ST2, &cell_st2, wtd_restore_write);
  register_cell(sfr, STCV0, &cell_stcv0, wtd_restore_write);
  register_cell(sfr, STCV1, &cell_stcv1, wtd_restore_write);
  register_cell(sfr, STCV2, &cell_stcv2, wtd_restore_write);
  register_cell(sfr, STLOAD, &cell_stload, wtd_restore_write);
  register_cell(sfr, STCC, &cell_stcc, wtd_restore_write);
  register_cell(sfr, CLKCONCMD, &cell_clkconcmd, wtd_restore_write);
  cmp = 0xFFFFFF;
  for (int i=0; i < 2; i++)
    {
    for (int j = 0; i < 7; i++)
      {
	tabPORT[i].pin[j] = 0;
      }
    }
}

void
cl_CC2530_sleep_timer::added_to_uc(void)
{
  //interrupt
  uc->it_sources->add(new cl_it_src(IEN0, bmSTIE, IRCON, bmSTIF, 0x004b, true,
				    "timer #2 overflow", 4));
}

int
cl_CC2530_sleep_timer::tick(int cycles)
{
  TimerTicks=0;
  for (int i = 0; i<cycles; i++)
    {
      systemTicks++;
      if (systemTicks  == 1000)
	{
	  TimerTicks++;
	  systemTicks = 0;
	}
    }
  if (TimerTicks != 0)
    STtick(TimerTicks);
  return(resGO);
}

int
cl_CC2530_sleep_timer::STtick(int cycles)
{
  count += cycles;
  cell_st0->set(count & 0xFF);
  cell_st1->set((count >> 8 ) & 0xFF);
  cell_st2->set((count >> 16) & 0xFF);
  if (count == cmp)
    {
      if ((powerMode == "PM1") || (powerMode == "PM2"))
	{
	  //Wake up device
	}
    }
  if (captureEnabled == 1)
    {
      if (*pinPointer == 1)
	{
	  //capture
	  cell_stcv0->set(count & 0xFF);
	  cell_stcv1->set((count >> 8 ) & 0xFF);
	  cell_stcv2->set((count >> 16) & 0xFF);
	}
    }
}

ulong
cl_CC2530_sleep_timer::get_rtime(void)
{  
  return(MemElapsedTime + systemTicks/freq);
}

void
cl_CC2530_sleep_timer::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_st0)
    {
      if ((cell_stload->get() & bmLDRDY) == 1)
	{
	  cmp = (cell_st2->get() << 16) + (cell_st1->get() << 8) + *val;
	  if (cmp + 5 >= count)
	    fprintf(stderr, "Warning: Compare event might be lost");
	}
    }
  if (cell == cell_stcc)
    {
      PortNum = (*val >> 3) & 0x3;
      PinNum = *val & 0x7;
      if ((PortNum != 3) && !((PortNum == 2) && ((PinNum == 6) || (PinNum ==7))))
	pinPointer = &tabPORT[PortNum].pin[PinNum];
      if (PortNum == 3)
	{
	  if ((sfr->read(IEN0) & bmSTIE) == 0)
	    captureEnabled = 0;
	}
    }
}

void
cl_CC2530_sleep_timer::reset(void)
{
  count = 0;
  cell_st0->set(0);
  cell_st1->set(0);
  cell_st2->set(0);
  cmp = 0xFFFFFF;
}

void
cl_CC2530_sleep_timer::print_info(class cl_console *con)
{
  print_info();
}

void
cl_CC2530_sleep_timer::print_info()
{
  fprintf(stderr,"\n***********  %s[%d] Count: 0x%04x", id_string, id,
		 count);
  fprintf(stderr,"Timer Frequency: %g Hz\tCC2530 Crystal: %g Hz", freq, CC2530xtal);
  fprintf(stderr,"\nTime elapsed: %g s", get_rtime());

  fprintf(stderr,"\n*********************************");
  fprintf(stderr,"****************************************\n\n");
}
