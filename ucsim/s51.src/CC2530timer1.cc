#include "CC2530timer1cl.h"
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


cl_CC2530_timer1::cl_CC2530_timer1(class cl_uc *auc, int aid, char *aid_string):
  cl_hw(auc, HW_TIMER, aid, aid_string)
{
  volatile int callnbr = 0;
  TRACE();
#ifdef DEBUG
  callnbr++;
  fprintf(stderr, "Called %d times\n\n", callnbr);
#endif
  mask_M0  = bmM0;//M0 and M1 used to select mode
  mask_M1  = bmM1;
  mask_TF  = bmOVFIF;//Interrupt mask for T1STAT
  addr_tl  = T1CNTL;
  addr_th  = T1CNTH;
  sfr= uc->address_space(MEM_SFR_ID);
  xram= uc->address_space(MEM_XRAM_ID);
  init();
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
	      sfr->write(tabCh[i].RegCMPH, sfr->read(T1CNTH));
	      sfr->write(tabCh[i].RegCMPL, sfr->read(T1CNTL));
	      fprintf(stderr,"\nCount: 0x%04x\n",256*cell_th->get()+cell_tl->get());
	      fprintf(stderr,"\nCapture: in %s of value: 0x%04x\n\n", id_string, 
		      256*sfr->read(tabCh[i].RegCMPH)+sfr->read(tabCh[i].RegCMPL));
	      int flag=1<<i;
	      sfr->write(T1STAT, flag);
	    }
	}
      else//compare mode
	{
	  if (((cell_tl->get() == sfr->read(T1CC0L)) 
	       && (cell_th->get() == sfr->read(T1CC0H)))
	      ||((cell_tl->get() == sfr->read(tabCh[i].RegCMPL)) 
		 && (cell_th->get() == sfr->read(tabCh[i].RegCMPH)))
	      ||((cell_tl->get() == 0) && (cell_th->get() == 0)))
	    {
	      TRACE();
	      tabCh[i].IOPin=Compare(tabCh[i].IOPin, tabCh[i].RegCTL, tabCh[i].RegCMPH, tabCh[i].RegCMPL);
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
	      xram->write(tabCh[i].RegCMPH, sfr->read(T1CNTH));
	      xram->write(tabCh[i].RegCMPL, sfr->read(T1CNTL));
	      int flag=1<<i;
	      sfr->write(T1STAT, flag);
	    }
	}
      else
	{
	  if (((cell_tl->get() == sfr->read(T1CC0L)) 
	       && (cell_th->get() == sfr->read(T1CC0H)))
	      ||((cell_tl->get() == xram->read(tabCh[i].RegCMPL)) 
		 && (cell_th->get() == xram->read(tabCh[i].RegCMPH)))
	      ||((cell_tl->get() == 0) && (cell_th->get() == 0)))
	    {
	      TRACE();
	      tabCh[i].IOPin=Compare(tabCh[i].IOPin, tabCh[i].RegCTL, tabCh[i].RegCMPH, tabCh[i].RegCMPL);
	      cc=1;
	    }
	}
    }
  if (cc==1)
    {
      print_info();
    }
}

bool
cl_CC2530_timer1::Capture(bool& IOPin, bool& ExIOPin, int captureMode)
{
  bool capture =false;
  if (IOPin != ExIOPin)//transition
    {
      if (IOPin ==1)//risingEdge
	{
	  if (captureMode==1 || captureMode==3)
	    {
	      fprintf(stderr, "Rising Edge of IOPin 1 detected\n");
	      capture = true;//capture
	    }
	}
      else 
	{
	  if (captureMode==2 || captureMode==3)
	    {
	      fprintf(stderr, "Falling Edge of IOPin 1 detected\n");
	      capture = true;//capture
	    }
	}
      ExIOPin=IOPin;
    }
  return (capture);
}

int
cl_CC2530_timer1::Compare(int IOPinChn, t_addr ctrlReg, t_addr T1CCnH, t_addr T1CCnL)
{
  TRACE();
  if (ctrlReg == T1CCTL3 || ctrlReg == T1CCTL4)
    ctrl=xram->read(ctrlReg);
  else
    ctrl=sfr->read(ctrlReg);
      fprintf(stderr, "Compare mode? %d\n", ctrl & 0x04);
      if ((ctrl & 0x04) != 0)//compare mode
    {
      fprintf(stderr, "Compare case: %d Timer mode: %d\n", (ctrl & 0x38)>>3, mode);
      switch((ctrl & 0x38)>>3)
	{ 
	case 0:	//set output on compare
	  if ((sfr->read(T1CC0L)== cell_tl->get())
	       &&(sfr->read(T1CC0H) == cell_th->get())) 
	    IOPinChn = 1; 
	  break;
	case 1: //Clear output on compare	  
	  if ((sfr->read(T1CC0L)== cell_tl->get())
	      &&(sfr->read(T1CC0H) == cell_th->get()))
	    IOPinChn = 0; 
	  break;
	case 2: //Toggle output on compare
	  if ((sfr->read(T1CC0L)== cell_tl->get())
	      &&(sfr->read(T1CC0H) == cell_th->get()))
	    {
	      if (IOPinChn == 0)
		IOPinChn = 1;
	      else
		IOPinChn = 0;
	    }
	  break;
	  //Set output on compare-up, clear on 0 or clear on cmp down (up/down mode)
	case 3:
	  if (mode == 3)//up/down
	    {
	      if ((sfr->read(T1CCnL) == cell_tl->get()) 
		   && (sfr->read(T1CCnH) == cell_th->get())) 
		{
		  //Count at threshold (toggle output)
		  if (up_down == 0)//Counting up
		    IOPinChn = 1;
		  else
		    IOPinChn = 0;
		}
	      else if (((sfr->read(T1CCnL) < cell_tl->get()) 
			&& (sfr->read(T1CCnH) == cell_th->get()))
		       ||(sfr->read(T1CCnH) < cell_th->get()))
		{
		  //count above threshold
		    IOPinChn = 1;
		}
	      else
		{
		  IOPinChn = 0;
		}
	    }
	  else
	    {
	      if ((sfr->read(T1CCnL)== cell_tl->get()) 
		  && (sfr->read(T1CCnH) == cell_th->get()))
		IOPinChn = 1;
	      if ((cell_tl->get() == 0) && (cell_th->get() == 0))
		IOPinChn = 0;
	    }
	  break;
	case 4://Clear output on compare-up, set on 0
	  if (mode == 3)//up/down
	    {
	      if ((sfr->read(T1CCnL) == cell_tl->get()) 
		   && (sfr->read(T1CCnH) == cell_th->get())) 
		{
		  //Count at threshold (toggle output)
		  if (up_down == 0)//counting up
		    IOPinChn = 0;
		  else
		    IOPinChn = 1;
		}
	      else if (((sfr->read(T1CCnL) < cell_tl->get()) 
			&& (sfr->read(T1CCnH) == cell_th->get()))
		       ||(sfr->read(T1CCnH) < cell_th->get()))
		{
		  //count above threshold
		    IOPinChn = 0;
		}
	      else
		{
		IOPinChn = 1;
		//print_info();
		}
	    }
	  else
	    {
	      if ((sfr->read(T1CCnL)== cell_tl->get())
		  &&(sfr->read(T1CCnH) == cell_th->get()))
		IOPinChn = 0;
	      if ((cell_tl->get() == 0) && (cell_th->get() == 0))
		IOPinChn = 1;
	    }
	  break;
	case 5: //Clear when equal T1CC0, set when equal T1CCn
	  if (T1CC0L == T1CCnL)
	    {
	      break;
	    }
	  else if (mode == 2)//Modulo mode -> case 5 <=> case 3
	    {
	      if ((sfr->read(T1CCnL) == cell_tl->get()) 
		  && (sfr->read(T1CCnH) == cell_th->get()))
		IOPinChn = 1;
	      if ((cell_tl->get() == 0) && (cell_th->get() == 0))
		IOPinChn = 0;
	    }
	  else
	    {
	      if ((sfr->read(T1CC0L)== cell_tl->get())
		  &&(sfr->read(T1CC0H) == cell_th->get()))
		IOPinChn = 0;
	      if ((sfr->read(T1CCnL)== cell_tl->get())
		  &&(sfr->read(T1CCnH) == cell_th->get()))
		IOPinChn = 1;
	    }
	  break;
	case 6: //Set when equal T1CC0, clear when equal T1CCn
	  if (T1CC0L == T1CCnL)
	    {
	      break;
	    }
	  else if (mode == 2)//Modulo mode-> case 6 <=> case 4
	    {
	      if ((sfr->read(T1CCnL)== cell_tl->get()) 
		  && (sfr->read(T1CCnH) == cell_th->get()))
		IOPinChn = 0;
	      if ((cell_tl->get() == 0) && (cell_th->get() == 0))
		IOPinChn = 1;
	    }
	  else
	    {
	      if ((sfr->read(T1CC0L) == cell_tl->get())
		  &&(sfr->read(T1CC0H) == cell_th->get()))
		IOPinChn = 1;
	      if ((sfr->read(T1CCnL) == cell_tl->get())
		  &&(sfr->read(T1CCnH) == cell_th->get()))
		IOPinChn = 0;
	    }
	  break;
	case 7: IOPinChn = 0; break;//Initialize output pin. CMP[2:0] is not changed.
	default: break;
	}
    }
      return(IOPinChn);
}


void
cl_CC2530_timer1::reset(void)
{
  cell_tl->write(0);
  cell_th->write(0);
  ticks=0;
}

double
cl_CC2530_timer1::get_rtime(void)
{
  return(ticks/freq);
}

int
cl_CC2530_timer1::init(void)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  CC2530xtal=32000000;
  fprintf(stderr, "CC2530xtal init at %g Hz\n", CC2530xtal);

  assert(sfr);
  up_down=0;//0 => count up, 1=> count down
  tickspd=1;
  tickcount=0;
  prescale=1;
  ticks=0;
  freq=CC2530xtal;

  if (id == 1)
    {
      register_cell(sfr, T1STAT, &cell_t1stat, wtd_restore_write);
      register_cell(sfr, T1CTL, &cell_t1ctl, wtd_restore_write);
      register_cell(sfr, CLKCONCMD, &cell_clkconcmd, wtd_restore_write);
    }
  else
    {
      fprintf(stderr, "Error: Timer1 id must be 1\n");
    }

  cell_tl = NULL;
  use_cell(sfr, addr_tl, &cell_tl, wtd_restore);
  assert(cell_tl);
  use_cell(sfr, addr_th, &cell_th, wtd_restore);

  tabCh[0]={0, 0, 0, 0, T1CC0L, T1CC0H, T1CCTL0};
  tabCh[1]={0, 0, 0, 0, T1CC1L, T1CC1H, T1CCTL1};
  tabCh[2]={0, 0, 0, 0, T1CC2L, T1CC2H, T1CCTL2};
  tabCh[3]={0, 0, 0, 0, T1CC3L, T1CC3H, T1CCTL3};
  tabCh[4]={0, 0, 0, 0, T1CC4L, T1CC4H, T1CCTL4};
  
  return(0);
}

void
cl_CC2530_timer1::added_to_uc(void)
{
  //overflow interrupt
  uc->it_sources->add(new cl_it_src(bmT1IE, T1STAT, bmOVFIF, 0x001b, true,
				    "timer #1 overflow", 4));
  uc->it_sources->add(new cl_it_src(bmT1IE, T1STAT, bmCH0IF, 0x001b, true,
				    "timer #1 Channel 0 interrupt", 4));
  uc->it_sources->add(new cl_it_src(bmT1IE, T1STAT, bmCH1IF, 0x001b, true,
				    "timer #1 Channel 1 interrupt", 4));
  uc->it_sources->add(new cl_it_src(bmT1IE, T1STAT, bmCH2IF, 0x001b, true,
				    "timer #1 Channel 2 interrupt", 4));
  uc->it_sources->add(new cl_it_src(bmT1IE, T1STAT, bmCH3IF, 0x001b, true,
				    "timer #1 Channel 3 interrupt", 4));
  uc->it_sources->add(new cl_it_src(bmT1IE, T1STAT, bmCH4IF, 0x001b, true,
				    "timer #1 Channel 4 interrupt", 4));
}


void
cl_CC2530_timer1::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_t1ctl) // correspond to T1CTL register (sfr->read(T1CTL))
    {
      t_mem md= *val & (mask_M0|mask_M1);
      TR= *val & !(mask_M0|mask_M1);//tr=>run
      if (md == mask_M0)
	mode= 1;
      else if (md == mask_M1)
	mode= 2;
      else if (md == (mask_M0|mask_M1))
	mode= 3;
      else
	mode= 0;
      //tickspeed();
      //prescaler();  // sfr->read(T1CTL)
      switch((*val & bmDIV)>>2)
	{ 
	case 0: prescale= 1; break;
	case 1: prescale= 8; break;
	case 2: prescale= 32; break;
	case 3: prescale= 128; break;
	default: prescale=1; break;
	}
      freq= CC2530xtal/(prescale*tickspd);
      fprintf(stderr,"Modification of T1CTL.\n");
      fprintf(stderr,
      "Prescale value: %d System clk division: %d Frequency: %g Hz Crystal: %g Hz\n",
	      prescale, tickspd, freq, CC2530xtal);
    }
  else if (cell == cell_clkconcmd)
    {
      switch(*val & 0x07)
	{ 
	case 0: tickspd= 1; break;
	case 1: tickspd= 2; break;
	case 2: tickspd= 4; break;
	case 3: tickspd= 8; break;
	case 4: tickspd= 16; break;
	case 5: tickspd= 32; break;
	case 6: tickspd= 64; break;
	case 7: tickspd= 128; break;
	default: tickspd=1; break;
	}

      freq= CC2530xtal/(prescale*tickspd);
      fprintf(stderr,"switch value: %d in %s: tickspeed x %d\n",
	      *val & 0x07,
	      __FUNCTION__,
	      tickspd);
      fprintf(stderr,"Modification of CLKCONCMD.\n");
      fprintf(stderr,
      "Prescale value: %d System clk division: %d Frequency: %g Hz Crystal:%g Hz\n"
	      ,prescale, tickspd, freq, CC2530xtal);
    }
}


int
cl_CC2530_timer1::tick(int cycles)
{

  fprintf(stderr, "Old ticks...  %g\n", ticks);

  ticks+=(double)cycles;

  fprintf(stderr, "tick! %g ticks... %d cycles. Time elapsed: %g s\n", ticks, cycles, get_rtime());

  fprintf(stderr, "Mode: %d\n", mode);

#ifdef DEBUG
  if ((sfr->read(T1CCTL1) & 0x04) == 0)
    {
      tickcount += cycles;
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

  switch (mode)
    {
    case 0: do_mode0(cycles); break;
    case 1: do_mode1(cycles); break;
    case 2: do_mode2(cycles); break;
    case 3: do_mode3(cycles); break;
    }

  return(resGO);
}

int
cl_CC2530_timer1::do_mode0(int cycles)
{
  TRACE();
  return(0);//timer stopped
}

int
cl_CC2530_timer1::do_mode1(int cycles)//Mode 1: free-running from 0 to FFFF
{
  TRACE();
  if (!TR)//run control
    return(0);

  //While exec of cycles-- !=0, repeat add(1) cycles time
  while (cycles--)
    {
      //Do add(1) to low cell, if 0 do add(1) to high cell, if 0 overflow 
      if (!cell_tl->add(1))//if tl==0 => overflow tl
	{
	  if (!cell_th->add(1))
	    {
	      cell_t1stat->set_bit1(mask_TF);//TF is timer overflow flag
	      overflow();
	    }
	}
      CaptureCompare();
    }
  return(0);
}

int
cl_CC2530_timer1::do_mode2(int cycles)//Mode 2: Modulo count from 0 to T1CC0 value
{
  TRACE();
  if (!TR)
    return(0);

  while (cycles--)
    {
      if (!cell_tl->add(1))//if tl==0 after add 1 => overflow tl
	{
	  if (!cell_th->add(1))
	    {
	      cell_t1stat->set_bit1(mask_TF);//TF is timer overflow flag
	      overflow();
	    }
	}
      if ((cell_tl->get() == sfr->read(T1CC0L)) 
	  && (cell_th->get() == sfr->read(T1CC0H)))
	{
	  cell_tl->set(0);
	  cell_th->set(0);
	}
      CaptureCompare();
    }

  return(0);
}

int
cl_CC2530_timer1::do_mode3(int cycles)//mode 3: up/down to T1CC0
{

  while (cycles--)
    {
      TRACE();
      //Count up 
      if (!up_down)
	{
	  if (!cell_tl->add(1))
	    {
	      if (!cell_th->add(1))
		{
		  cell_t1stat->set_bit1(mask_TF);
		  overflow();
		}
	    }
	  if ((cell_tl->get() == sfr->read(T1CC0L)) 
	      && (cell_th->get() == sfr->read(T1CC0H)))
	    up_down=1;
	}
      //Count down
      else
	{	
	  if (cell_tl->add(-1) == 0xFF)
	    {
	      cell_th->add(-1);
	    }
	  if ((cell_tl->get() == 0) && (cell_th->get() == 0))
	    {
	      cell_t1stat->set_bit1(mask_TF);
	      overflow();
	      up_down=0;
	    }
	}
      CaptureCompare();
    }
  return(0);
}

void
cl_CC2530_timer1::overflow(void)
{
  inform_partners(EV_OVERFLOW, 0);
  fprintf(stderr,"Timer 1 overflow !\n");
  print_info();
}

//Useful??
void
cl_CC2530_timer1::happen(class cl_hw *where, enum hw_event he, void *params)
{
  struct ev_port_changed *ep= (struct ev_port_changed *)params;
  fprintf(stderr,"Timer 1 %s !\n", __FUNCTION__);
}

void
cl_CC2530_timer1::print_info(class cl_console *con)
{
  print_info();
  /*  char *modes[]= { "Timer stopped", "Free running mode", "Modulo mode", "Up/down Mode" };
  int on;

  con->dd_printf("%s[%d] 0x%04x", id_string, id,
		 256*cell_th->get()+cell_tl->get());
  con->dd_printf(" %s\n", modes[mode]);
  con->dd_printf("Prescale value: %d\tSystem clk division: %d\n", prescale, tickspd);
  con->dd_printf("Frequency: %g\tCC2530 Crystal: %g\n", freq, CC2530xtal);
  con->dd_printf("Time elapsed: %g\n", get_rtime());
  con->dd_printf("Channel 3 Output: %d", tabCh[3].IOPin);
  con->dd_printf("\n");*/
}

void
cl_CC2530_timer1::print_info()
{
  char *modes[]= { "Timer stopped", "Free running mode", "Modulo mode", "Up/down Mode" };
  int on;

  fprintf(stderr,"\n*************%s[%d] Count: 0x%04x", id_string, id,
		 256*cell_th->get()+cell_tl->get());
  fprintf(stderr," %s*************\n", modes[mode]);
  fprintf(stderr,"Prescale value: %d\t\tSystem clk division: %d\n", 
	  prescale, tickspd);
  fprintf(stderr,"Timer Frequency: %g Hz\tCC2530 Crystal: %g Hz", freq, CC2530xtal);
  fprintf(stderr,"\nTime elapsed: %g s\n", get_rtime());
  fprintf(stderr,"Timer 1 IOPins:\t");
  for (int i=0; i<5; i++)
    {
      fprintf(stderr,"Channel %d: %d\t", i, tabCh[i].IOPin);
      if (i==2)
	fprintf(stderr,"\n");	
    }
  fprintf(stderr,"\n*********************************");
  fprintf(stderr,"***********************************\n\n");
}


/* End of s51.src/CC2530timer1.cc */
