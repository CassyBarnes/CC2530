#include "CC2530timercl.h"
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

template<class T>
cl_CC2530_timer<T>::cl_CC2530_timer(class cl_uc *auc, int aid, char *aid_string):
  cl_hw(auc, HW_TIMER, aid, aid_string)
{
  volatile int callnbr = 0;
#ifdef DEBUG
  callnbr++;
  fprintf(stderr, "Called %d times\n\n", callnbr);
#endif
  mask_M0  = bmM0;//M0 and M1 used to select mode
  mask_M1  = bmM1;
  mask_TF  = bmOVFIF;//Interrupt mask for T1STAT
  //addr_tl  = T1CNTL;
  //addr_th  = T1CNTH;
  sfr= uc->address_space(MEM_SFR_ID);
  xram= uc->address_space(MEM_XRAM_ID);
  init();
}


template<class T>
int
cl_CC2530_timer<T>::init(void)
{
  ////TRACE();
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  CC2530xtal=32000000;
  fprintf(stderr, "CC2530xtal init at %g Hz\n", CC2530xtal);
  register_cell(sfr, CLKCONCMD, &cell_clkconcmd, wtd_restore_write);
  return(0);
}

template<class T>
int
cl_CC2530_timer<T>::tick(int cycles)
{
  ////TRACE();
  fprintf(stderr, "%s\n", id_string);
  fprintf(stderr, "tick! %g ticks... %d cycles. Time elapsed: %g s\n", systemTicks, cycles, get_rtime());
  fprintf(stderr, "Mode: %d\n", mode);
}

template<class T>
double
cl_CC2530_timer<T>::get_rtime(void)
{
  return(MemElapsedTime + systemTicks/freq);
}

template<class T>
void
cl_CC2530_timer<T>::reset(void)
{
  cell_tl->write(0);
  if (cell_th != NULL)
    cell_th->write(0);
  ticks=0;
}

template<class T>
void
cl_CC2530_timer<T>::added_to_uc(void)
{
  //overflow interrupt
  uc->it_sources->add(new cl_it_src(bmT1IE, TIMIF, bmOVFIF, 0x004b, true,
				    "timer #1 overflow", 4));
}

template<class T>
void
cl_CC2530_timer<T>::CaptureCompare(void)
{
  cc=0;
  ////TRACE();
  if (ChMax>3)
    {
      ////TRACE();
      for (int i=0; i<3; i++)
	{
	  //Possible capture/compare cases
	  if ((sfr->read(tabCh[i].RegCTL) & 0x04) == 0)//capt enabled
	    {
	      captureMode = sfr->read(tabCh[i].RegCTL) & 0x03;
	      capt=Capture(tabCh[i].IOPin, tabCh[i].ExIOPin, captureMode);
	      if (capt == true)
		{
		  if (cell_th != NULL)
		    sfr->write(tabCh[i].RegCMPH, count>>8);
		  sfr->write(tabCh[i].RegCMPL, count & 0xFF);

		  fprintf(stderr,"\nCount: 0x%04x\n",count);
		  fprintf(stderr,"\nCapture: in %s of value: 0x%04x\n\n", 
			  id_string, tabCh[i].ValRegCMP);

		  int flag=1<<i;
		  sfr->write(T1STAT, flag);
		}
	    }
	  else//compare mode
	    {
	      fprintf(stderr, "Channel %d in compare mode...\n", i);
	      fprintf(stderr, "Count: 0x%04x\n", count);
	      fprintf(stderr, "Compare reg val: 0x%04x\n", tabCh[i].ValRegCMP);
	      fprintf(stderr, "Channel 0 Compare reg val: 0x%04x\n", tabCh[0].ValRegCMP);
	      if ((count == tabCh[0].ValRegCMP)
		  ||(count == tabCh[i].ValRegCMP)
		  ||(count == 0))
		{
		  ////TRACE();
		  tabCh[i].IOPin=Compare(tabCh[i].IOPin, tabCh[i].RegCTL, tabCh[i].ValRegCMP);
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
		  xram->write(tabCh[i].RegCMPH, count>>8);
		  xram->write(tabCh[i].RegCMPL, count & 0xFF);
		  int flag=1<<i;
		  sfr->write(T1STAT, flag);
		}
	    }
	  else
	    {
	      if ((count == tabCh[0].ValRegCMP)
		  ||(count == tabCh[i].ValRegCMP)
		  ||(count == 0))
		{
		  ////TRACE();
		  tabCh[i].IOPin=Compare(tabCh[i].IOPin, tabCh[i].RegCTL, tabCh[i].ValRegCMP);
		  cc=1;
		}
	    }
	}
    }
  else
    {
      for (int i=0; i<ChMax; i++)
	{
	  if ((sfr->read(tabCh[i].RegCTL) & 0x04) == 0)//capt enabled
	    {
	      captureMode = sfr->read(tabCh[i].RegCTL) & 0x03;
	      capt=Capture(tabCh[i].IOPin, tabCh[i].ExIOPin, captureMode);
	      if (capt == true)
		{
		  sfr->write(tabCh[i].RegCMPH, sfr->read(T1CNTH));
		  sfr->write(tabCh[i].RegCMPL, sfr->read(T1CNTL));
		  fprintf(stderr,"\nCount: 0x%04x\n", count);
		  fprintf(stderr,"\nCapture: in %s of value: 0x%04x\n\n", id_string, 
			  tabCh[i].ValRegCMP);
		  int flag=1<<i;
		  sfr->write(T1STAT, flag);
		}
	    }
	  else//compare mode
	    {
	      if ((count == tabCh[0].ValRegCMP)
		  ||(count == tabCh[i].ValRegCMP)
		  ||(count == 0))
		{
		  ////TRACE();
		  tabCh[i].IOPin=Compare(tabCh[i].IOPin, tabCh[i].RegCTL, tabCh[i].ValRegCMP);
		  cc=1;
		}
	    }
	}
    }
  if (cc==1)
    {
      print_info();
    }
}

template<class T>
bool
cl_CC2530_timer<T>::Capture(bool& IOPin, bool& ExIOPin, int captureMode)
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

template<class T>
bool
cl_CC2530_timer<T>::Compare(bool IOPinChn, t_addr ctrlReg, T TxCCn)
{
  ////TRACE();
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
	  if (count == tabCh[0].ValRegCMP) 
	    IOPinChn = 1; 
	  break;
	case 1: //Clear output on compare	  
	  if (count == tabCh[0].ValRegCMP) 
	    IOPinChn = 0; 
	  break;
	case 2: //Toggle output on compare
	  if (count == tabCh[0].ValRegCMP) 
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
	      if (count == TxCCn) 
		{
		  //Count at threshold (toggle output)
		  if (up_down == 0)//Counting up
		    IOPinChn = 1;
		  else
		    IOPinChn = 0;
		}
	      else if (count > TxCCn)
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
	      if (count == TxCCn)
		IOPinChn = 1;
	      if (count == 0)
		IOPinChn = 0;
	    }
	  break;
	case 4://Clear output on compare-up, set on 0
	  if (mode == 3)//up/down
	    {
	      if (count == TxCCn) 
		{
		  //Count at threshold (toggle output)
		  if (up_down == 0)//counting up
		    IOPinChn = 0;
		  else
		    IOPinChn = 1;
		}
	      else if (count > TxCCn)
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
	      if (count == TxCCn)
		IOPinChn = 0;
	      if (count == 0)
		IOPinChn = 1;
	    }
	  break;
	case 5: //Clear when equal TxCC0, set when equal TxCCn
	  if (ctrlReg == T1CCTL0)
	    {
	      break;
	    }
	  else if (mode == 2)//Modulo mode -> case 5 <=> case 3
	    {
	      if (count == TxCCn)
		IOPinChn = 1;
	      if (count == 0)
		IOPinChn = 0;
	    }
	  else
	    {
	      if (count == tabCh[0].ValRegCMP)
		IOPinChn = 0;
	      if (count == TxCCn)
		IOPinChn = 1;
	    }
	  break;
	case 6: //Set when equal TxCC0, clear when equal TxCCn
	  if (ctrlReg == T1CCTL0)
	    {
	      break;
	    }
	  else if (mode == 2)//Modulo mode-> case 6 <=> case 4
	    {
	      if (count == TxCCn)
		IOPinChn = 0;
	      if (count == 0)
		IOPinChn = 1;
	    }
	  else
	    {
	      if (count == tabCh[0].ValRegCMP)
		IOPinChn = 1;
	      if (count == TxCCn)
		IOPinChn = 0;
	    }
	  break;
	case 7: IOPinChn = 0; break;//Initialize output pin. CMP[2:0] is not changed.
	default: break;
	}
    }
      return(IOPinChn);
}


template<class T>
void
cl_CC2530_timer<T>::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_txctl) // correspond to TxCTL register (sfr->read(T1CTL))
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
      MemElapsedTime = get_rtime();
      MemSystemTicks = systemTicks;
      systemTicks=0;
      freq= CC2530xtal/(tickspd);
      fprintf(stderr,"switch value: %d in %s: tickspeed / %d\n",
	      *val & 0x07,
	      __FUNCTION__,
	      tickspd);
      fprintf(stderr,"Modification of CLKCONCMD.\n");
      fprintf(stderr,
	      "Prescale value: %d System clk division: %d Frequency: %g Hz Crystal:%g Hz\n"
	      ,prescale, tickspd, freq, CC2530xtal);
    }
}

template<class T>
int
cl_CC2530_timer<T>::do_Stop(int cycles)
{
  ////TRACE();
  return(0);//timer stopped
}

template<class T>
int
cl_CC2530_timer<T>::do_FreeRunningMode(int cycles)//Mode 1: free-running from 0 to FFFF
{
  ////TRACE();

  //While exec of cycles-- !=0, repeat add(1) cycles time
  while (cycles--)
    {
      ////TRACE();
      count++;
      if (count==0)
	{
	  cell_txstat->set_bit1(mask_TF);//TF is timer overflow flag
	  overflow();
	}
      refresh_sfr(count);
      CaptureCompare();
    }
  return(0);
}

template<class T>
int
cl_CC2530_timer<T>::do_ModuloMode(int cycles)//Mode 2: Modulo count from 0 to TxCC0 value
{
  ////TRACE();

  while (cycles--)
    {
      if (count == tabCh[0].ValRegCMP)
	{
	  count=0;
	  cell_txstat->set_bit1(mask_TF);//TF is timer overflow flag
	  overflow();
	} 
      else
	{
	  count++;
	  if (count == 0)//Case where TxCC0 > Maximum value of timer
	    {
	      cell_txstat->set_bit1(mask_TF);//TF is timer overflow flag
	      overflow();
	    } 
	}

      refresh_sfr(count);
      CaptureCompare();
    }
  return(0);
}

template<class T>
int
cl_CC2530_timer<T>::do_UpDownMode(int cycles)//mode 3: up/down to TxCC0
{

  while (cycles--)
    {
      ////TRACE();
      //Count up 
      if (!up_down)
	{
	  count++;
	  if (count==0)
	    {
	      cell_txstat->set_bit1(mask_TF);//TF is timer overflow flag
	      overflow();
	    }
	  if (count == tabCh[0].ValRegCMP)
	    up_down=1;
	}
      //Count down
      else
	{	 
	  count--;
	  if (count == 0)
	    {
	      cell_txstat->set_bit1(mask_TF);
	      overflow();
	      up_down=0;
	    }
	}
      refresh_sfr(count);
      CaptureCompare();
    }
  return(0);
}



template<class T>
int
cl_CC2530_timer<T>::do_DownMode(int cycles)//mode 4: down from TxCC0
{
  while (cycles--)
    {
      if (count == 0)
	{
	  count=tabCh[0].ValRegCMP;
	}
      else
	{
	  count--;
	}
      if (count == 0)
	{
	  cell_txstat->set_bit1(mask_TF);
	  overflow();
	} 
      refresh_sfr(count);
      CaptureCompare();
    }
}

template<class T>
void
cl_CC2530_timer<T>::overflow(void)
{
  inform_partners(EV_OVERFLOW, 0);
  fprintf(stderr,"%s overflow !\n", id_string);
  print_info();
}

template<class T>
void
cl_CC2530_timer<T>::refresh_sfr(char count)
{
  cell_tl->set(count & 0xFF); 
}

template<class T>
void
cl_CC2530_timer<T>::refresh_sfr(short int count)
{
  cell_tl->set(count & 0xFF); 
  cell_th->set(count>>8);
}

template<class T>
void
cl_CC2530_timer<T>::print_info(class cl_console *con)
{
  print_info();
}

template<class T>
void
cl_CC2530_timer<T>::print_info()
{

  fprintf(stderr,"\n***********  %s[%d] Count: 0x%04x", id_string, id,
		 count);
  fprintf(stderr," %s*************\n", modes[mode]);
  fprintf(stderr,"Prescale value: %d\t\tSystem clk division: %d\n", 
	  prescale, tickspd);
  fprintf(stderr,"Timer Frequency: %g Hz\tCC2530 Crystal: %g Hz", freq, CC2530xtal);
  fprintf(stderr,"\nTime elapsed: %g s\n", get_rtime());
  fprintf(stderr,"%s IOPins:\t", id_string);
  for (int i=0; i<ChMax; i++)
    {
      fprintf(stderr,"Channel %d: %d\t", i, tabCh[i].IOPin);
      if (i==2)
	fprintf(stderr,"\n");	
    }
  fprintf(stderr,"\n*********************************");
  fprintf(stderr,"****************************************\n\n");
}

template class cl_CC2530_timer<short int>;
template class cl_CC2530_timer<char>;
