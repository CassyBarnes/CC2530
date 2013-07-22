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

#define T2INFO

#ifndef CC2530xtal
#define CC2530xtal 32000000
#endif

cl_CC2530_timer2::cl_CC2530_timer2(class cl_uc *auc, int aid, char *aid_string):
  cl_hw(auc, HW_MAC_TIMER, aid, aid_string)
{
  make_partner(HW_CC2530_USART, 0);
  make_partner(HW_CC2530_RADIO, 0);
  sfr= uc->address_space(MEM_SFR_ID);
  xram= uc->address_space(MEM_XRAM_ID);
  init();
}

int
cl_CC2530_timer2::init(void)
{
  TRACE();
  //CC2530xtal=32000000;
  fprintf(stderr, "CC2530xtal init at %d Hz\n", CC2530xtal);
  events[0] = "1 (Period)";
  events[1] = "1 (Compare 1)";
  events[2] = "1 (Compare 2)";
  events[3] = "1 (OVF period)";
  events[4] = "1 (OVF Compare 1)";
  events[5] = "1 (OVF Compare 2)";
  events[6] = "0";
  assert(sfr);
  TimerTicks = 0;
  ticks = 0;
  freq = CC2530xtal;
  count = 0;
  mode = 0;
  modes[0] = "   Up Mode   ";
  modes[1] = " Delta Mode  ";

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
  register_cell(sfr, CLKCONCMD, &cell_clkconcmd, wtd_restore_write);
  register_cell(xram, CSPT, &cell_cspt, wtd_restore_write);
  return(0);
}

int
cl_CC2530_timer2::tick(int cycles)
{

  TimerTicks += cycles;
  systemTicks += cycles;

#ifdef T2INFO
  TRACE();
  fprintf(stderr, "************* %s *************\n", id_string);
  fprintf(stderr, "tick! %g ticks... %d cycles. Time elapsed: %g s\n", TimerTicks, cycles, get_rtime());
  fprintf(stderr, "Mode: %s\n", modes[mode]);
#endif

  switch (mode)
    {
    case 0: do_UpMode(cycles); break;
    case 1: do_DeltaMode(cycles); break;
    }
  
  /*  t2movfsel = ((cell_t2msel->get()) >> 4) & 0x07;
  t2msel = cell_t2msel->get() & 0x07;
  switch (t2movfsel)
    {
    case 0: 
      cell_t2movf0->set(OVF0);
      cell_t2movf1->set(OVF1);
      cell_t2movf2->set(OVF2);
      break;
    case 1:
      cell_t2movf0->set(OVFcap0);
      cell_t2movf1->set(OVFcap1);
      cell_t2movf2->set(OVFcap2);
      break;	
    case 2:
      cell_t2movf0->set(OVFper0);
      cell_t2movf1->set(OVFper1);
      cell_t2movf2->set(OVFper2);
      break;	
    case 3:
      cell_t2movf0->set(OVFcmp1_0);
      cell_t2movf1->set(OVFcmp1_1);
      cell_t2movf2->set(OVFcmp1_2);
      break;	
    case 4:
      cell_t2movf0->set(OVFcmp2_0);
      cell_t2movf1->set(OVFcmp2_1);
      cell_t2movf2->set(OVFcmp2_2);
      break;
    default:
      fprintf(stderr, "ERROR in T2MSEL register configuration.\n");
      break;
    }
  int t2msel = cell_t2msel->get() & 0x07;
  switch (t2msel)
    {
    case 0: 
      cell_t2m0->set(count & 0xFF);
      cell_t2m1->set((count>>8) & 0xFF);
      break;
    case 1: 
      cell_t2m0->set(t2_cap & 0xFF);
      cell_t2m1->set((t2_cap>>8) & 0xFF);
      break;
    case 2: 
      cell_t2m0->set(t2_per & 0xFF);
      cell_t2m1->set((t2_per>>8) & 0xFF);
      break;
    case 3: 
      cell_t2m0->set(t2_cmp1 & 0xFF);
      cell_t2m1->set((t2_cmp1>>8) & 0xFF);
      break;
    case 4: 
      cell_t2m0->set(t2_cmp2 & 0xFF);
      cell_t2m1->set((t2_cmp2>>8) & 0xFF);
      break;
    default:
      fprintf(stderr, "ERROR in T2MSEL register configuration.\n");
      break;
      }*/
  fprintf(stderr, "Timer MAC count: 0x%02x.\n", count);
}

double
cl_CC2530_timer2::get_rtime(void)
{
  return(MemElapsedTime + systemTicks/freq);
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
      if ((*val & 0x01) == 0)
	run = 0;
      else
	run = 1;
    }
  if (cell == cell_clkconcmd)
    {
      tickspd= 1<<((*val & bmTickSpd) >> 3);
      MemElapsedTime = get_rtime();
      MemSystemTicks = systemTicks;
      systemTicks=0;
      freq= CC2530xtal/(tickspd);
      fprintf(stderr,"Modification of CLKCONCMD.\n");

    }
  if (cell == cell_t2msel)
    {
      t2movfsel = (*val >> 4) & 0x07;
      t2msel = *val & 0x07;
    }

  if (cell == cell_t2m0)
    {
      TRACE();
      fprintf(stderr,"T2MSEl is %d.\n", t2msel);
      switch (t2msel)
	{
	case 0:
	  count = *val;
	  fprintf(stderr,"MAC count is %d.\n", count);
	  break;
	case 1:
	  t2_cap = *val;
	  break;
	case 2:
	  t2_per = *val;
	  break;
	case 3:
	  t2_cmp1 = *val;
	  fprintf(stderr, "CMP1 value: 0x%04x .\n", t2_cmp1);
	  break;
	case 4:
	  t2_cmp2 = *val;
	  break;
	default:
	  fprintf(stderr, "ERROR in T2MSEL register configuration.\n");
	  break;
	}
    }

  if (cell == cell_t2m1)
    {
      TRACE();
      switch (t2msel)
	{
	case 0:
	  if (run == 0)
	    {
	      count = (*val<< 8) + cell_t2m0->get();
	      fprintf(stderr,"MAC count is %02x.\n", count);
	    }
	  else //run =1
	    {
	      count= (*val<< 8) + cell_t2m0->get();
	      fprintf(stderr,"MAC count is %d.\n", count);
	      mode = 1;
	    }
	  break;
	case 1:
	  t2_cap = (*val<< 8) + cell_t2m0->get();
	  break;
	case 2:
	  t2_per = (*val<< 8) + cell_t2m0->get();
	  break;
	case 3:
	  t2_cmp1 = (*val<< 8) + cell_t2m0->get();
	  fprintf(stderr, "CMP1 value: 0x%04x .\n", t2_cmp1);
	  break;
	case 4:
	  t2_cmp2 = (*val<< 8) + cell_t2m0->get();
	  break;
	default:
	  fprintf(stderr, "ERROR in T2MSEL register configuration.\n");
	  break;
	}
    }
 
  if (cell == cell_t2movf0) 
    {
      switch(t2movfsel)
	{
	case 0: OVF0 = *val; break;
	case 1: OVFcap0 = *val; break;
	case 2: OVFper0 = *val; break;
	case 3: OVFcmp1_0 = *val; break;
	case 4: OVFcmp2_0 = *val; break;
	default: fprintf(stderr, "ERROR in T2MOVFSEL reg configuration.\n"); break;
	}
    }

  if (cell == cell_t2movf1) 
    {
      switch(t2movfsel)
	{
	case 0: OVF1 = *val; break;
	case 1: OVFcap1 = *val; break;
	case 2: OVFper1 = *val; break;
	case 3: OVFcmp1_1 = *val; break;
	case 4: OVFcmp2_1 = *val; break;
	default: fprintf(stderr, "ERROR in T2MOVFSEL reg configuration.\n"); break;
	}
    }

  if (cell == cell_t2movf2) 
    {
      switch(t2movfsel)
	{
	case 0: OVF2 = *val; break;
	case 1: OVFcap2 = *val; break;
	case 2: OVFper2 = *val; break;
	case 3: OVFcmp1_2 = *val; break;
	case 4: OVFcmp2_2 = *val; break;
	default: fprintf(stderr, "ERROR in T2MOVFSEL reg configuration.\n"); break;
	}
    }
}

t_mem
cl_CC2530_timer2::read(class cl_memory_cell *cell)
{
  t_mem d = 0;
  if (cell == cell_t2m0)
    {
      switch (t2msel)
	{
	case 0: 
	  d = count & 0xFF;
	  fprintf(stderr, "reading t2m0, value should be %d.\n", count & 0xFF);
	  return(d);
	case 1:
	  d = t2_cap & 0xFF;
	  return(d);
	  break;
	case 2: 
	  d = t2_per & 0xFF;
	  return(d);
	  break;
	case 3:
	  fprintf(stderr, "reading t2m0, cmp1 value should be %d.\n", t2_cmp1 & 0xFF);
	  d = t2_cmp1 & 0xFF;
	  return(d);
	  break;
	case 4:  
	  d = t2_cmp2 & 0xFF;
	  return(d);
	  break;
	default: break;
	}	
    }

  if (cell== cell_t2m1)
    {
      switch (t2msel)
	{
	case 0: 
	  d = (count>>8) & 0xFF;
	  return(d);
	  break;
	case 1:
	  d = (t2_cap>>8) & 0xFF;
	  return(d);
	  break;
	case 2:  
	  d = (t2_per>>8) & 0xFF;
	  return(d);
	  break;
	case 3: 
	  d = (t2_cmp1>>8) & 0xFF;
	  return(d);
	  break;
	case 4:  
	  d = (t2_cmp2>>8) & 0xFF;
	  return(d);
	  break;
	default: break;
	}
    }

  if (cell == cell_t2movf0) 
    {
      switch (t2movfsel)
	{
	case 0: 
	  return(OVF0);
	  break;
	case 1:
	  return(OVFcap0);
	  break;
	case 2:  
	  return(OVFper0);
	  break;
	case 3: 
	  return(OVFcmp1_0);
	  break;
	case 4:  
	  return(OVFcmp2_0);
	  break;
	default: break;
	}
    }
  if (cell == cell_t2movf1) 
    {
      switch (t2movfsel)
	{
	case 0: 
	  return(OVF1);
	  break;
	case 1:
	  return(OVFcap1);
	  break;
	case 2:  
	  return(OVFper1);
	  break;
	case 3: 
	  return(OVFcmp1_1);
	  break;
	case 4:  
	  return(OVFcmp2_1);
	  break;
	default: break;
	}
    }

  if (cell == cell_t2movf2) 
    {
      switch (t2movfsel)
	{
	case 0: 
	  return(OVF2);
	  break;
	case 1:
	  return(OVFcap2);
	  break;
	case 2:  
	  return(OVFper2);
	  break;
	case 3: 
	  return(OVFcmp1_2);
	  break;
	case 4:  
	  return(OVFcmp2_2);
	  break;
	default: break;
	}
    }
}

int
cl_CC2530_timer2::do_UpMode(int cycles)
{
  while (cycles--)
    {
      event = false;
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
      event = false;
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
  inform_partners(EV_OVERFLOW, 0);
  fprintf(stderr,"%s overflow !\n", id_string);
  int cspt = cell_cspt->get();
  if (cspt != 0xFF)
    cell_cspt->set(cspt - 1);
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

  if ((OVF2 == OVFcmp1_2)&&(OVF1 == OVFcmp1_1) && (OVF0 == OVFcmp1_0))
    {
      cell_t2irqf->set_bit1(0x10);
      if ((sfr->read(T2IRQM) & 0x10) == 1)
	cell_ircon->set_bit1(bmT2IF);//Interrupt request
      event = true;
    }

  if ((OVF2 == OVFcmp2_2)&&(OVF1 == OVFcmp2_1) && (OVF0 == OVFcmp2_0))
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
  T2_EVENT1 = false;
  T2_EVENT2 = false;
  if (count == t2_cmp1)
    {
      cell_t2irqf->set_bit1(0x02);
      if ((sfr->read(T2IRQM) & 0x02) == 1)
	cell_ircon->set_bit1(bmT2IF);
      event = true;
      fprintf(stderr, "COMPARE 1 EVENT! Count: %d\n", count);
    }
  if (count == t2_cmp2)
    {
      cell_t2irqf->set_bit1(0x04);
      if ((sfr->read(T2IRQM) & 0x04) == 1)
	cell_ircon->set_bit1(bmT2IF);
      event = true;
    }
  if ((count == t2_per) && (t2_per != 0))
    {
      fprintf(stderr, "PERIOD EVENT! Count: %d\n", count);
      count=0;
      overflow();
      cell_t2irqf->set_bit1(0x01);
      if ((sfr->read(T2IRQM) & 0x01) == 1)
	cell_ircon->set_bit1(bmT2IF);
      event = true;
    }

  //events...
  if (event)
    {
      evtcfg1 = cell_t2evtcfg->get() & 0x07;
      bm_evtcfg1 = 1;
      for (int i; i<evtcfg1; i++)
	bm_evtcfg1 <<= 1;
      T2_EVENT1 = (cell_t2irqf->get() & bm_evtcfg1) != 0;//true if watched event has happened 
      evtcfg2 = ((cell_t2evtcfg->get()) >> 4) & 0x07;
      bm_evtcfg2 = 1;
      for (int i; i<evtcfg2; i++)
	bm_evtcfg2 <<= 1;
      T2_EVENT2 = (cell_t2irqf->get() & bm_evtcfg2) != 0;
    }
  else
    {
      cell_t2irqf->set(0);
      T2_EVENT1 = 0;
      T2_EVENT2 = 0;
    }
  fprintf(stderr, "MAC TIMER OUTPUTS: EVENT1: %s\tEVENT2: %s\n",T2_EVENT1?events[evtcfg1]:events[6], T2_EVENT2?events[evtcfg2]:events[6]);
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
  fprintf(stderr,"Timer Frequency: %g Hz\tCC2530 Crystal: %d Hz", freq, CC2530xtal);
  fprintf(stderr,"\nTime elapsed: %g s", get_rtime());

  fprintf(stderr,"\n*********************************");
  fprintf(stderr,"****************************************\n\n");
}
