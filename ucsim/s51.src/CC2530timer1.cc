#include "CC2530timer1cl.h"
#include "regs51.h"
#include "types51.h"


cl_CC2530_timer1::cl_CC2530_timer1(class cl_uc *auc, int aid, char *aid_string):
  cl_hw(auc, HW_TIMER, aid, aid_string)
{
  /*  mask_TR  = bmTR1;//Run control*/
  mask_M0  = bmM0;//M0 and M1 used to select mode
  mask_M1  = bmM1;
  mask_TF  = bmOVFIF;//Interrupt mask for T1STAT
  addr_tl  = T1CNTL;
  addr_th  = T1CNTH;
}

void
cl_CC2530_timer1::tickspeed(void)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  switch(sfr->read(CLKCONCMD) & 0x07)
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
  fprintf(stderr,"switch value: %d tickspeed: %d %d\n",sfr->read(CLKCONCMD) & 0x07,tickspd, sfr->read(CLKCONCMD));
}

void
cl_CC2530_timer1::Capture(int i)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  t_mem captureMode = sfr->read(T1CCTL0) & 0x03;

  switch(i)
    { 
    case 0: 
      switch(captureMode)
	{ 
	case 0: break;//No capture
	case 1: captureRisingEdge0=1; captureFallEdge0=0; break;//Capt on rising edge
	case 2: captureRisingEdge0=0; captureFallEdge0=1; break;//Capt on falling edge
	case 3: captureRisingEdge0=1; captureFallEdge0=1; break;//Capt on all edges
	default: break;
	}
      break;
    case 1: 
      switch(captureMode)
	{ 
	case 0: break;//No capture
	case 1: captureRisingEdge1=1; captureFallEdge1=0; break;//Capt on rising edge
	case 2: captureRisingEdge1=0; captureFallEdge1=1; break;//Capt on falling edge
	case 3: captureRisingEdge1=1; captureFallEdge1=1; break;//Capt on all edges
	default: break;
	}
      break;
    case 2: 
      switch(captureMode)
	{ 
	case 0: break;//No capture
	case 1: captureRisingEdge2=1; captureFallEdge2=0; break;//Capt on rising edge
	case 2: captureRisingEdge2=0; captureFallEdge2=1; break;//Capt on falling edge
	case 3: captureRisingEdge2=1; captureFallEdge2=1; break;//Capt on all edges
	default: break;
	}
      break;
    case 3: 
      switch(captureMode)
	{ 
	case 0: break;//No capture
	case 1: captureRisingEdge3=1; captureFallEdge3=0; break;//Capt on rising edge
	case 2: captureRisingEdge3=0; captureFallEdge3=1; break;//Capt on falling edge
	case 3: captureRisingEdge3=1; captureFallEdge3=1; break;//Capt on all edges
	default: break;
	}
      break;
    case 4: 
      switch(captureMode)
	{ 
	case 0: break;//No capture
	case 1: captureRisingEdge4=1; captureFallEdge4=0; break;//Capt on rising edge
	case 2: captureRisingEdge4=0; captureFallEdge4=1; break;//Capt on falling edge
	case 3: captureRisingEdge4=1; captureFallEdge4=1; break;//Capt on all edges
	default: break;
	}
      break;
    default: break;
    }
}

void
cl_CC2530_timer1::Compare(int i)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  switch(i)
    { 
    case 0: IOPinChn = IOPinCh0; break;
    case 1: IOPinChn = IOPinCh1; break;
    case 2: IOPinChn = IOPinCh2; break;
    case 3: IOPinChn = IOPinCh3; break;
    case 4: IOPinChn = IOPinCh4; break;
    default: break;
    }
  if (sfr->read(T1CCTL0) & 0x04 != 0)//capture mode
    {
      switch(sfr->read(T1CCTL0) & 0x38)
	{ 
	case 0:	//set output on compare
	  if ((T1CC0L== T1CNTL)&&(T1CC0H == T1CNTH)) 
	    IOPinChn = 1; 
	  break;
	case 1: //Clear output on compare	  
	  if ((T1CC0L== T1CNTL)&&(T1CC0H == T1CNTH))
	    IOPinChn = 0; 
	  break;
	case 2: //Toggle output on compare
	  if ((T1CC0L== T1CNTL)&&(T1CC0H == T1CNTH))
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
	      if ((T1CC0L <= T1CNTL) && (T1CC0H == T1CNTH) || (T1CC0H < T1CNTH))
//Count above threshold 
		IOPinChn = 1;
	      else
		IOPinChn = 0;
	    }
	  else
	    {
	      if ((T1CC0L== T1CNTL) && (T1CC0H == T1CNTH))
		IOPinChn = 1;
	      if ((T1CNTL == 0) && (T1CNTH == 0))
		IOPinChn = 0;
	    }
	  break;
	case 4: tickspd= 16; 
	  if (mode == 3)//up/down
	    {
	      if ((T1CC0L <= T1CNTL) && (T1CC0H == T1CNTH) || (T1CC0H < T1CNTH))
//Count above threshold 
		IOPinChn = 0;
	      else
		IOPinChn = 1;
	    }
	  else
	    {
	      if ((T1CC0L== T1CNTL)&&(T1CC0H == T1CNTH))
		IOPinChn = 0;
	      if ((T1CNTL == 0) && (T1CNTH == 0))
		IOPinChn = 1;
	    }
	  break;//Clear output on compare-up, set on 0
	case 5: break;//Reserved
	case 6: break;//Reserved
	case 7: IOPinChn = 0; break;//Initialize output pin. CMP[2:0] is not changed.
	default: break;
	}
    }
  switch(i)
    { 
    case 0: IOPinCh0 = IOPinChn; break;
    case 1: IOPinCh1 = IOPinChn; break;
    case 2: IOPinCh2 = IOPinChn; break;
    case 3: IOPinCh3 = IOPinChn; break;
    case 4: IOPinCh4 = IOPinChn; break;
    default: break;
    }
}

void
cl_CC2530_timer1::prescaler(void)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  switch((sfr->read(T1CTL) & bmDIV)>>2)
    { 
    case 0: prescale= 1; break;
    case 1: prescale= 8; break;
    case 2: prescale= 32; break;
    case 3: prescale= 128; break;
    default: prescale=1; break;
    }
  freq= CC2530xtal/(prescale*tickspd);
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
  return((double)ticks/freq);
}

int
cl_CC2530_timer1::init(void)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  CC2530xtal=32000000;
  fprintf(stderr, "CC2530xtal init at %g Hz\n", CC2530xtal);
  if (sfr)
    {
      //:id=aid
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
      use_cell(sfr, addr_tl, &cell_tl, wtd_restore);
      use_cell(sfr, addr_th, &cell_th, wtd_restore);



    }
  return(0);
}

void
cl_CC2530_timer1::added_to_uc(void)
{
  //overflow interrupt
    uc->it_sources->add(new cl_it_src(bmT1IE, IEN1, bmOVFIF, 0x001b, true,
				      "timer #1", 4));
}

/*t_mem
cl_CC2530_timer1::read(class cl_cell *cell)
{
  return(cell->get());
}*/

void
cl_CC2530_timer1::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_t1ctl)
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
      tickspeed();
      prescaler();
      fprintf(stderr,"Modification of T1CTL.\n");
      fprintf(stderr,"Prescale value: %d\tSystem clk division: %d\tFrequency: %g %g\n"
	      ,prescale, tickspd, freq, CC2530xtal);
    }
  else if (cell == cell_clkconcmd)
    {
      tickspeed();
      prescaler();
      fprintf(stderr,"Modification of CLKCONCMD.\n");
      fprintf(stderr,"Prescale value: %d\tSystem clk division: %d\tFrequency: %g %g\n"
	      ,prescale, tickspd, freq, CC2530xtal);
    }
}


int
cl_CC2530_timer1::tick(int cycles)
{
  switch (mode)
    {
    case 0: do_mode0(cycles); break;
    case 1: do_mode1(cycles); break;
    case 2: do_mode2(cycles); break;
    case 3: do_mode3(cycles); break;
    }
  ticks+=cycles;
  return(resGO);
}

int
cl_CC2530_timer1::do_mode0(int cycles)
{
  return(0);//timer stopped
}

int
cl_CC2530_timer1::do_mode1(int cycles)//Mode 1: free-running from 0 to FFFF
{
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
    }
  return(0);
}

int
cl_CC2530_timer1::do_mode2(int cycles)//Mode 2: Modulo count from 0 to T1CC0 value
{
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
      if ((cell_tl->get() == T1CC0L) && (cell_th->get() == T1CC0H))
	{
	  cell_tl->set(0);
	  cell_th->set(0);
	}
    }
  return(0);
}

int
cl_CC2530_timer1::do_mode3(int cycles)//mode 3: up/down to T1CC0
{
  bool up_down=0;//0 => count up, 1=> count down
  while (cycles--)
    {
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
	  if ((cell_tl->get() == T1CC0L) && (cell_th->get() == T1CC0H))
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
    }
  return(0);
}

void
cl_CC2530_timer1::overflow(void)
{
  inform_partners(EV_OVERFLOW, 0);
}

//Useful??
void
cl_CC2530_timer1::happen(class cl_hw *where, enum hw_event he, void *params)
{
  struct ev_port_changed *ep= (struct ev_port_changed *)params;

  /* if (where->cathegory == HW_PORT &&
      he == EV_PORT_CHANGED &&
      ep->id == 3)
    {
      t_mem p3n= ep->new_pins & ep->new_value;
      t_mem p3o= ep->pins & ep->prev_value;
      if ((p3n & mask_T) &&
	  !(p3o & mask_T))
	T_edge++;
      INT= p3n & mask_INT;
      //printf("timer%d p%dchanged (%02x,%02x->%02x,%02x) INT=%d(%02x) edge=%d(%02x)\n",id,where->id,ep->prev_value,ep->pins,ep->new_value,ep->new_pins,INT,mask_INT,T_edge,mask_T);
      }*/
  fprintf(stderr,"Timer 1 happen...\n");
}

void
cl_CC2530_timer1::print_info(class cl_console *con)
{
  char *modes[]= { "Timer stopped", "Free running mode", "Modulo mode", "Up/down Mode" };
  int on;
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);

  con->dd_printf("%s[%d] 0x%04x", id_string, id,
		 256*cell_th->get()+cell_tl->get());
  con->dd_printf(" %s", modes[mode]);
    on= TR;
  con->dd_printf(" %s", on?"ON":"OFF");
  con->dd_printf(" irq=%c", (cell_t1stat->get()&mask_TF)?'1':'0');
  con->dd_printf(" %s", sfr?"?":((sfr->get(IEN1)&bmT1IE)?"en":"dis"));
  // con->dd_printf(" prio=%d", uc->it_priority(bmPT0));
  con->dd_printf("\n");
}


/* End of s51.src/CC2530timer1.cc */
