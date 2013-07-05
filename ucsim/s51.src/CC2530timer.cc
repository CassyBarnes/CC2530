#include "CC2530timercl.h"
#include <assert.h>
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"

#define DEBUG
#ifdef DEBUG
#define TRACE()								\
  fprintf(stderr, "%s:%d in %s()\n", __FILE__, __LINE__, __FUNCTION__)
#else
#define TRACE()
#endif

#ifndef CC2530xtal
#define CC2530xtal 32000000
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
  sfr= uc->address_space(MEM_SFR_ID);
  xram= uc->address_space(MEM_XRAM_ID);
  init();
}


template<class T>
int
cl_CC2530_timer<T>::init(void)
{
  ////TRACE();
  fprintf(stderr, "CC2530xtal at %d Hz\n", CC2530xtal);
  register_cell(sfr, CLKCONCMD, &cell_clkconcmd, wtd_restore_write);
  register_cell(sfr, IRCON, &cell_ircon, wtd_restore_write);
  PinEvent = false;
  return(0);
}

template<class T>
int
cl_CC2530_timer<T>::tick(int cycles)
{
  TimerTicks=0;
  for (int i = 0; i<cycles; i++)
    {
      systemTicks++;
      if (((int)systemTicks % prescale) == 0)
	TimerTicks++;
    }
  ////TRACE();
  fprintf(stderr, "\n************* %s *************\n", id_string);
  fprintf(stderr, "tick! %g ticks since reset/clkconcmd modif... %d cycles. Time elapsed: %g s\n", systemTicks, cycles, get_rtime());
  fprintf(stderr, "Tick Frequency: CPU Freq (%d MHz) / %d\n", 32/tickspd, prescale);
  fprintf(stderr, "Mode: %s\n\n", modes[mode]);
  if (TimerTicks != 0)
    TimerTick(TimerTicks);
  fprintf(stderr, "Count: %d\n",count);
  return(resGO);
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
  if ((ctrl & bmCmpMode) != 0)//compare mode
    {
      fprintf(stderr, "Compare case: %d Timer mode: %d\n", (ctrl & bmWTDonCMP)>>3, mode);
      switch((ctrl & bmWTDonCMP)>>3)
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
      get_next_cc_event();
    }
  else if (cell == cell_clkconcmd)
    {
      tickspd = 1 << ((*val & bmTickSpd) >> 3);
      MemElapsedTime = get_rtime();
      MemSystemTicks = systemTicks;
      systemTicks=0;
      freq= CC2530xtal/(tickspd);
      fprintf(stderr,"switch value: %d in %s: tickspeed / %d\n",
	      (*val & bmTickSpd) >> 3,
	      __FUNCTION__,
	      tickspd);
      fprintf(stderr,"Modification of CLKCONCMD.\n");
      fprintf(stderr,
	      "Prescale value: %d Tick Speed: /%d Frequency: %g Hz Crystal:%d Hz\n"
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
  TRACE();

  //While exec of cycles-- !=0, repeat add(1) cycles time
  while (cycles--)
    {
      NextCmpEvent--;
      fprintf(stderr, "Next compare event in %d Timer ticks...\n", NextCmpEvent);
      //TRACE();
      count++;
      if (count==0)
	{
	  if ((cell_OvfMaskReg->get() & OVFMaskMask) != 0)
	    {
	      cell_OvfFlagReg->set_bit1(OVFIFMask);
	      cell_ircon->set_bit1(IrconFlag);
	    }
	  overflow();
	}
      refresh_sfr(count);
      if ((NextCmpEvent == 0) || (PinEvent))
	CaptureCompare();
      PinEvent = false;
    }
  return(0);
}

template<class T>
int
cl_CC2530_timer<T>::do_ModuloMode(int cycles)//Mode 2: Modulo count from 0 to TxCC0 value
{
  TRACE();

  while (cycles--)
    {
      NextCmpEvent--;
      fprintf(stderr, "Next compare event in %d Timer ticks...\n", NextCmpEvent);
      if (count == tabCh[0].ValRegCMP)
	{
	  count=0;
	  if ((cell_OvfMaskReg->get() & OVFMaskMask) != 0)
	    {
	      cell_OvfFlagReg->set_bit1(OVFIFMask);
	      cell_ircon->set_bit1(IrconFlag);
	    }
	  overflow();
	  get_next_cc_event();
	} 
      else
	{
	  count++;
	  if (count == 0)//Case where TxCC0 > Maximum value of timer
	    {
	      if ((cell_OvfMaskReg->get() & OVFMaskMask) != 0)
		{
		  cell_OvfFlagReg->set_bit1(OVFIFMask);
		  cell_ircon->set_bit1(IrconFlag);
		}
	      overflow();
	      get_next_cc_event();
	    } 
	}

      refresh_sfr(count);
      if ((NextCmpEvent == 0) || (PinEvent))
	CaptureCompare();
      PinEvent = false;
    }
  return(0);
}

template<class T>
int
cl_CC2530_timer<T>::do_UpDownMode(int cycles)//mode 3: up/down to TxCC0
{

  while (cycles--)
    {
      NextCmpEvent--;
      fprintf(stderr, "Next compare event in %d Timer ticks...\n", NextCmpEvent);
      up_down_changed = false;
      ////TRACE();
      //Count up 
      if (!up_down)
	{
	  count++;
	  if (count==0)
	    {
	      if ((cell_OvfMaskReg->get() & OVFMaskMask) != 0)
		{
		  cell_OvfFlagReg->set_bit1(OVFIFMask);
		  cell_ircon->set_bit1(IrconFlag);
		}
	      overflow();
	    }
	  if (count == tabCh[0].ValRegCMP)
	    {
	      up_down=1;
	      up_down_changed = true;
	    }
	}
      //Count down
      else
	{	 
	  count--;
	  if (count == 0)
	    {
	      if ((cell_OvfMaskReg->get() & OVFMaskMask) != 0)
		{
		  cell_OvfFlagReg->set_bit1(OVFIFMask);
		  cell_ircon->set_bit1(IrconFlag);
		}
	      overflow();
	      up_down=0;
	      up_down_changed = true;
	    }
	}
      refresh_sfr(count);
      if ((NextCmpEvent == 0) || (PinEvent))
	CaptureCompare();
      if (up_down_changed)
	get_next_cc_event();
      PinEvent = false;
    }
  return(0);
}



template<class T>
int
cl_CC2530_timer<T>::do_DownMode(int cycles)//mode 4: down from TxCC0
{
  while (cycles--)
    {
      NextCmpEvent--;
      fprintf(stderr, "Next compare event in %d Timer ticks...\n", NextCmpEvent);
      if (count == 0)
	{
	  count=tabCh[0].ValRegCMP;
	  if ((NextCmpEvent == 0) || (PinEvent))
	    CaptureCompare();
	  else
	    get_next_cc_event();
	}
      else
	{
	  count--;
	}
      if (count == 0)
	{
	  if ((cell_OvfMaskReg->get() & OVFMaskMask) != 0)
	    {
	      cell_OvfFlagReg->set_bit1(OVFIFMask);
	      cell_ircon->set_bit1(IrconFlag);
	    }
	  overflow();
	} 
      refresh_sfr(count);
      if ((NextCmpEvent == 0) || (PinEvent))
	CaptureCompare();
      PinEvent = false;
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
  fprintf(stderr,"Timer Frequency: %g Hz\tCC2530 Crystal: %d Hz", freq, CC2530xtal);
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
