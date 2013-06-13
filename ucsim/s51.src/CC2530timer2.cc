#include "CC2530timer2cl.h"
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


cl_CC2530_timer2::cl_CC2530_timer2(class cl_uc *auc, int aid, char *aid_string):
  cl_hw(auc, HW_TIMER, aid, aid_string)
{
  sfr= uc->address_space(MEM_SFR_ID);
  xram= uc->address_space(MEM_XRAM_ID);
  init();
}

int
cl_CC2530_timer2::init(void)
{
  TRACE();
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  CC2530xtal=32000000;
  fprintf(stderr, "CC2530xtal init at %g Hz\n", CC2530xtal);

  assert(sfr);
  TimerTicks=0;
  ticks=0;
  freq=CC2530xtal;
  count=0;
  mode = 0;
  modes[0]= "     Up Mode     ";
  modes[1]= "   Delta Mode    ";

  //cell_t2m0->set(count & 0xFF);
  //cell_t2m1->set(count >> 8);

  register_cell(sfr, T2CTRL, &cell_t2ctrl, wtd_restore_write);
  register_cell(sfr, T2EVTCFG, &cell_t2evtcfg, wtd_restore_write);
  register_cell(sfr, T2MSEL, &cell_t2msel, wtd_restore_write);
  register_cell(sfr, T2M0, &cell_t2m0, wtd_restore_write);
  register_cell(sfr, T2M1, &cell_t2m1, wtd_restore_write);
  register_cell(sfr, IRCON, &cell_ircon, wtd_restore_write);
  register_cell(sfr, T2IRQF, &cell_t2irqf, wtd_restore_write);
  register_cell(sfr, T2MOVF0, &cell_t2movf0, wtd_restore_write);
  register_cell(sfr, T2MOVF1, &cell_t2movf1, wtd_restore_write);
  register_cell(sfr, T2MOVF2, &cell_t2movf2, wtd_restore_write);
  return(0);
}

int
cl_CC2530_timer2::tick(int cycles)
{
  event = false;
  TimerTicks += cycles;

  TRACE();
  fprintf(stderr, "%s\n", id_string);
  fprintf(stderr, "tick! %g ticks... %d cycles. Time elapsed: %g s\n", TimerTicks, cycles, get_rtime());
  fprintf(stderr, "Mode: %s\n", modes[mode]);
  //if (run == 1)
    {
      TRACE();
      switch (mode)
	{
	case 0: do_UpMode(cycles); break;
	case 1: do_DeltaMode(cycles); break;
	}
    }
}

double
cl_CC2530_timer2::get_rtime(void)
{
  return(TimerTicks/freq);
}

void
cl_CC2530_timer2::added_to_uc(void)
{
  //interrupt
  uc->it_sources->add(new cl_it_src(IEN1, bmT2IE, IRCON, bmT2IF, 0x004b, true,
				    "timer #2 overflow", 4));
}

void
cl_CC2530_timer2::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_t2ctrl)
    {
      if (*val & 0x01 == 0)
	run = 0;
      else
	run = 1;
    }
  else if ((cell == cell_t2m0) || (cell== cell_t2m1))
    {
      if ((run == 0) && (((cell_t2msel->get()) & 0x07) == 0))
	{
	  if (cell == cell_t2m1)
	    count = *val*256 + cell_t2m0->get();
	}
      else
	{
	  if (run == 1)
	    {
	      if (cell == cell_t2m1)
		{
		  count= *val*256 + cell_t2m0->get();
		  mode = 1;
		}
	    }
	}
    }
  int t2movfsel = ((cell_t2msel->get()) >> 4) & 0x07;
  if (cell == cell_t2movf0) 
    {
      if (t2movfsel == 0)
	OVF0 = *val;
      else if (t2movfsel == 0x03)
	t2_OVFcmp1_0 = *val;
      else if (t2movfsel == 0x04)
	t2_OVFcmp2_0 = *val;
    }
  if (cell == cell_t2movf1) 
    {
      if (t2movfsel == 0)
	OVF1 = *val;
      else if (t2movfsel == 0x03)
	t2_OVFcmp1_1 = *val;
      else if (t2movfsel == 0x04)
	t2_OVFcmp2_1 = *val;
    }
  if (cell == cell_t2movf2) 
    {
      if (t2movfsel == 0)
	OVF2 = *val;
      else if (t2movfsel == 0x03)
	t2_OVFcmp1_2 = *val;
      else if (t2movfsel == 0x04)
	t2_OVFcmp2_2 = *val;
    }

  /*if (cell == cell_t2msel)
    {
      switch (*val & 0x07) 
	{
	case 0:
	      cell_t2m0->set(count & 0xFF);
	      cell_t2m1->set(count >> 8);*/
}

/*t_mem
cl_CC2530_timer2::read(class cl_memory_cell *cell)
{
  t_mem* d = 0;
  if ((cell == cell_t2m0) || (cell== cell_t2m1))
    {
      switch ((cell_t2msel->get()) & 0x07)
	{
	case 0: 
	  TRACE();
	  if (cell == cell_t2m0)
	    {
	      d = count & 0xFF;
	      fprintf(stderr, "reading t2m0, value should be %d.\n", count & 0xFF);
	      cell_t2m0->set(count & 0xFF);
	      //return(1);
	      //return(count & 0xFF);
	    }
	  else
	    d = count>>8;
	  return(*d);
	  break;
	case 1:
	  TRACE();
	  //t2_cap = count;
	  if (cell == cell_t2m0)
	    d = t2_cap & 0xFF;
	  else
	    d = t2_cap>>8;
	  return(*d);
	  break;
	case 2:  break;
	case 3:  break;
	case 4:  break;
	default: break;
	}
    }
  int t2movfsel = ((cell_t2msel->get()) >> 4) & 0x07;
  if (cell == cell_t2movf0) 
    {
      if (t2movfsel == 0)
	return(OVF0);
      if (t2movfsel == 1)
	return(OVFcap0);
    }
  if (cell == cell_t2movf1) 
    {
      if (t2movfsel == 0)
	return(OVF1);
      if (t2movfsel == 1)
	return(OVFcap1);
    }
  if (cell == cell_t2movf2) 
    {
      if (t2movfsel == 0)
	return(OVF2);
      if (t2movfsel == 1)
	return(OVFcap2);
    }
    }*/

int
cl_CC2530_timer2::do_UpMode(int cycles)
{
  TRACE();
  while (cycles--)
    {
      TRACE();
      count++;
      if (count==0)
	{
	  overflow();
	}
      CountCompare();
    }
  return(0);
}

int
cl_CC2530_timer2::do_DeltaMode(int cycles)//mode 4: down from TxCC0
{
  while (cycles--)
    {
      if (count == 0)
	{
	  mode = 0;
	}
      else
	{
	  count--;
	}
      CountCompare();
    }
}

void
cl_CC2530_timer2::overflow(void)
{
  cell_t2irqf->set_bit1(0x01);
  if ((sfr->read(T2IRQM) & 0x01) == 1)
    cell_ircon->set_bit1(bmT2IF);

  inform_partners(EV_OVERFLOW, 0);
  fprintf(stderr,"%s overflow !\n", id_string);
  print_info();
  OVF0++;
  if (OVF0 == 0)
    {
      OVF1++;
      if (OVF1== 0)
	OVF2++;
    }

  if ((OVF2 == OVFper2) && (OVF1 == OVFper1) && (OVF0 == OVFper0))
    {
      //OVF period event
      OVF2 = 0;
      OVF1 = 0;
      OVF0 = 0;
      cell_t2irqf->set_bit1(0x08);
      if ((sfr->read(T2IRQM) & 0x08) == 1)
	cell_ircon->set_bit1(bmT2IF);//Interrupt request
      event = true;
    }

  if ((OVF2 == t2_OVFcmp1_2)&&(OVF1 == t2_OVFcmp1_1) && (OVF0 == t2_OVFcmp1_0))
    {
      cell_t2irqf->set_bit1(0x10);
      if ((sfr->read(T2IRQM) & 0x10) == 1)
	cell_ircon->set_bit1(bmT2IF);//Interrupt request
      event = true;
    }

  if ((OVF2 == t2_OVFcmp2_2)&&(OVF1 == t2_OVFcmp2_1) && (OVF0 == t2_OVFcmp2_0))
    {
      cell_t2irqf->set_bit1(0x20);
      if ((sfr->read(T2IRQM) & 0x20) == 1)
	cell_ircon->set_bit1(bmT2IF);//Interrupt request
      event = true;
    }
}

void
cl_CC2530_timer2::CountCompare(void)
{
  if (count == t2_cmp1)
    {
      cell_t2irqf->set_bit1(0x02);
      if ((sfr->read(T2IRQM) & 0x02) == 1)
	cell_ircon->set_bit1(bmT2IF);
      event = true;
    }
  if (count == t2_cmp2)
    {
      cell_t2irqf->set_bit1(0x04);
      if ((sfr->read(T2IRQM) & 0x04) == 1)
	cell_ircon->set_bit1(bmT2IF);
      event = true;
    }

  //events...
  if (event)
    {
      int i = cell_t2evtcfg->get() & 0x07;
      T2_EVENT1 = (cell_t2irqf->get() & i) != 0;
      int j = ((cell_t2evtcfg->get()) >> 4) & 0x07;
      T2_EVENT2 = (cell_t2irqf->get() & j) != 0;
    }
  else
    {
      cell_t2irqf->set(0);
      T2_EVENT1 = 0;
      T2_EVENT2 = 0;
    }
}

void
cl_CC2530_timer2::print_info(class cl_console *con)
{
  print_info();
}

void
cl_CC2530_timer2::print_info()
{
  fprintf(stderr,"\n***********  %s[%d] Count: 0x%04x", id_string, id,
		 count);
  fprintf(stderr," %s*************\n", modes[mode]);
  fprintf(stderr,"Timer Frequency: %g Hz\tCC2530 Crystal: %g Hz", freq, CC2530xtal);
  fprintf(stderr,"\nTime elapsed: %g s\n", get_rtime());

  fprintf(stderr,"\n*********************************");
  fprintf(stderr,"****************************************\n\n");
}
