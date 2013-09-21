#include <math.h>
#include <assert.h>
#include "CC2530usartcl.h"
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"

#undef USARTINFO
#ifdef USARTINFO
#define DEBUG
#endif
#ifdef DEBUG
#define TRACE() \
fprintf(stderr, "%s:%d in %s()\n", __FILE__, __LINE__, __FUNCTION__)
#else
#define TRACE()
#endif

cl_CC2530_usart::cl_CC2530_usart(class cl_uc *auc, int aid, char *aid_string):
  cl_hw(auc, HW_CC2530_USART, aid, aid_string)
{
  make_partner(HW_CC2530_DMA, 1);
  UsartID = aid;
  if (UsartID == 0)
    {
      UXCSR = U0CSR;
      UXDBUF = U0DBUF;
      UXUCR = U0UCR;
      UXBAUD = U0BAUD;
      bmURXxIF = bmURX0IF;
      bmUTXxIF = bmUTX0IF;
      bmURXxIE = bmURX0IE;
      bmUTXxIE = bmUTX0IE;
      addressRxISR = 0x13;
      addressTxISR = 0x3B;
      messageRx = "USART 0 RX";
      messageTx = "USART 0 TX";
    }
  else
    {
      UXCSR = U1CSR;
      UXDBUF = U1DBUF;
      UXUCR = U1UCR;
      UXBAUD = U1BAUD;
      bmURXxIF = bmURX1IF;
      bmUTXxIF = bmUTX1IF;
      bmURXxIE = bmURX1IE;
      bmUTXxIE = bmUTX1IE;
      addressRxISR = 0x1B;
      addressTxISR = 0x73;
      messageRx = "USART 1 RX";
      messageTx = "USART 1 TX";
    }
  init();

}

int
cl_CC2530_usart::init(void)
{
  sfr= uc->address_space(MEM_SFR_ID);
  bitRxCnt = 0;
  if (UsartID == 0)
    {
      register_cell(sfr, U0CSR, &cell_uXcsr, wtd_restore_write);
      register_cell(sfr, U0DBUF, &cell_uXdbuf, wtd_restore_write);
      register_cell(sfr, U0UCR, &cell_uXucr, wtd_restore_write);
      register_cell(sfr, U0GCR, &cell_uXgcr, wtd_restore_write);
      register_cell(sfr, U0BAUD, &cell_uXbaud, wtd_restore_write);
    }
  else
    {
      register_cell(sfr, U1CSR, &cell_uXcsr, wtd_restore_write);
      register_cell(sfr, U1DBUF, &cell_uXdbuf, wtd_restore_write);
      register_cell(sfr, U1UCR, &cell_uXucr, wtd_restore_write);
      register_cell(sfr, U1GCR, &cell_uXgcr, wtd_restore_write);
      register_cell(sfr, U1BAUD, &cell_uXbaud, wtd_restore_write);
    }
  register_cell(sfr, TCON, &cell_tcon, wtd_restore_write);
  register_cell(sfr, IRCON2, &cell_ircon2, wtd_restore_write);

  testTab[0]=0;
  testTab[1]=1;
  testTab[2]=0;
  testTab[3]=1;
  testTab[4]=0;
  testTab[5]=1;
  testTab[6]=0;
  testTab[7]=1;
  testTab[8]=0;
  testTab[9]=1;
  testTab[10]=1;//0:start bit, 10101010:data, 1:Parity, 1:Stop

  reset();

  return(0);
}

void 
cl_CC2530_usart::reset(void)
{
  stopBitLevel = 1;
  startBitLevel = 0;
  TXD = stopBitLevel;
  RXD = stopBitLevel;
  BaudFactor = 1.0/16;

  parity = false;
  mode = 0;
  slave = 0;
  TickCountForBaud = 0;
  RX = 0;
  TX = 0;
  bitRxCnt = 0;
  bitTxCnt = 0;
  BitNumber = 0;
  StopBits = 0;
  queue = 0;
  baud_m = 0;
  baud_e = 0;
  UsartID = 0;
  BaudFactor = 0;
  BaudRate = 0;
}

void
cl_CC2530_usart::added_to_uc(void)
{
  uc->it_sources->add(new cl_it_src(IEN0, bmURXxIE, TCON, bmURXxIF, addressRxISR, 
				    true, messageRx, 4));
  uc->it_sources->add(new cl_it_src(IEN2, bmUTXxIE, IRCON2, bmUTXxIF, addressTxISR, 
				    true, messageTx, 4));
}

int
cl_CC2530_usart::tick(int cycles)
{
  while (cycles--)
    {
      //TRACE();
      assert(sfr);
      TickCountForBaud++;
      BaudRate = BaudFactor * CC2530xtal;

      mode = cell_uXcsr->get() & bmMode;//=0x80 if uart mode, 0 if SPI mode.
#ifdef USARTINFO
      fprintf(stderr, "Tickcount: %d\tGoal: %g\n", TickCountForBaud, 1.0/BaudFactor);
#endif
      if (!((mode == 0) && (slave == 1)))
	{
	  if (TickCountForBaud >= (1/BaudFactor))
	    {
	      BaudTick();
	      TickCountForBaud = 0;
	    }
	}
    }
  return(0);
}

void 
cl_CC2530_usart::SCK(void)
{
  TRACE();
  //slave->ExtSCK();
}

void 
cl_CC2530_usart::ExtSCK(void)
{
  if ((mode == 0) && (slave == 1))
    BaudTick();
}

void 
cl_CC2530_usart::BaudTick(void)
{
  TRACE();
  if (mode == 0)//SPI mode
    {
      if ((cell_uXcsr->get() & bmActive) == 1)//usart active
	{
	  if (slave == 0)//Master mode
	    {
	      SCK();//Generate serial clk
	      Shift_out(regForTx, MOSI);
	      Shift_in(regForRx, MISO);
	    }
	  else //slave mode
	    {
	      if (SSN == 0)
		{
		  Shift_out(regForTx, MISO);
		  Shift_in(regForRx, MOSI);
		}
	      else 
		{
		//Output High Impedance on MISO
		}
	    }
	}

    }
  else//uart mode
    {
      if ((cell_uXcsr->get() & bmActive) == 1)//usart active
	{
	  if (RX == 1)//active because rx
	    Shift_in(regForRx, RXD);
	  if (TX == 1) //active for tx
	    Shift_out(regForTx, TXD);
	}
    }
}

bool
cl_CC2530_usart::Shift_in(t_mem& Rxreg, bool& IPin)
{
  IPin=testTab[0];
  if ((bitRxCnt == 0)&&(IPin != startBitLevel))//waiting for start bit (0)
    return(0);
  bitRxCnt++;//counts number of bits received 
  IPin=(testTab[bitRxCnt - 1] != 0);
#ifdef USARTINFO
  fprintf(stderr, "RX bits count: %d\n", bitRxCnt);
  fprintf(stderr, "IPIN: %s\n", IPin?"1":"0");
#endif
  if ((bitRxCnt == 10) && (BitNumber == 9))//Parity bit
    RxParity = IPin;
  else if ((bitRxCnt >= 2)&&(bitRxCnt <= 9))//Data before parity and stop bits
    {
      if ((cell_uXgcr && bmOrder) == 0)//LSB first
	{
	  if (IPin)//Reception: if receives bit 1
	    {
	      Rxreg += 0x80;//1 goes to rx buffer (left)
	      cell_uXdbuf->set(Rxreg);
	    }
	  Rxreg>>=1;//right shift to make room for next rx bit
	  Rxreg &= 0xFF;
	}
      else //MSB first
	{
	  Rxreg<<=1;//left shift to make room for next rx bit
	  if (IPin)//Reception: if receives bit 1
	    {
	      Rxreg += 1;//1 goes to rx buffer (right)
	      cell_uXdbuf->set(Rxreg);
	    }
#ifdef USARTINFO
	  fprintf(stderr, "RxReg: 0x%02x\n", Rxreg);
#endif
	  Rxreg &= 0xFF;
	}
    }
  if (bitRxCnt >= (BitNumber + 1))//bitNumber = 8 or 9 (+1 for start bit)
    {
      if (BitNumber == 9)//Parity enabled
	{
	  if (bitRxCnt == 10)
	    {
	      RxParity = IPin;
	      if (RxParity != parity_check(Rxreg))
		{
		  cell_uXcsr->set_bit1(bmErr);//Parity error
#ifdef USARTINFO
		  fprintf(stderr, "Parity Error!\n");
#endif
		}
	      else
		{
#ifdef USARTINFO
		fprintf(stderr, "Parity OK!\n");
#endif
		}
	    }
	  if ((bitRxCnt >= 11) && (bitRxCnt < (11 + StopBits)))
	    {
	      if (IPin != stopBitLevel)
		{
		  cell_uXcsr->set_bit1(bmFe);//Framing error
#ifdef USARTINFO
		  fprintf(stderr, "Framing Error!\n");
#endif
		}
	    }
	}
      else
	{
	  if ((bitRxCnt >= 10) && (bitRxCnt < (10 + StopBits)))
	    {
	      if (IPin != stopBitLevel)
		cell_uXcsr->set_bit1(bmFe);//Framing error
	    }
	}
      if (bitRxCnt == BitNumber + StopBits + 1)//(+1 for start bit)
	{
	  //rx end
	  cell_uXcsr->set_bit1(bmRx_byte); //signals end rx
	  //generate receive interrupt
	  cell_tcon->set_bit1(bmURXxIF);
	  if (UsartID == 0)
	    inform_partners(EV_URX0, 0);
	  else
	    inform_partners(EV_URX1, 0);
	  cell_uXcsr->set_bit0(bmActive);//no longer active
	  RX = 0;
	  bitRxCnt = 0;
	}
    }
}

void
cl_CC2530_usart::Shift_out(t_mem& Txreg, bool& OPin)
{
  bitTxCnt++;
#ifdef USARTINFO
  fprintf(stderr, "TX bits count: %d\n", bitTxCnt);
  fprintf(stderr, "TxReg: %d\n", Txreg);
#endif
  if (bitTxCnt <= 1)
    OPin = startBitLevel;
  else if ((bitTxCnt > 1) && (bitTxCnt < 10))
    {
      TRACE();
      if ((cell_uXgcr && bmOrder) == 0)//LSB first
	{
	  TRACE();
	  OPin = ((Txreg & 0x01) == 1);
	  Txreg>>=1;
	  Txreg &= 0xFF;
	}
      else //MSB first 
	{
	  TRACE();
#ifdef USARTINFO
	  fprintf(stderr, "TxReg's MSB: %d\n", (Txreg & 0x80)>>7);
#endif
	  OPin = ((Txreg & 0x80) != 0);
	  Txreg<<=1;
	  Txreg &= 0xFF;
	}
    }
  else if (bitTxCnt >= 10)
    {
      TRACE();
      if ((BitNumber == 9) && (bitTxCnt == 10))
	{
	  TRACE();
#ifdef USARTINFO
	  fprintf(stderr, "ParityTx!\n");
#endif
	  OPin = TxParity;
	}
      else 
	{
	  TRACE();
	  OPin = stopBitLevel;
#ifdef USARTINFO
	  fprintf(stderr, "STOPTx! StopBits:%d\n", StopBits);
#endif
	}
      if (bitTxCnt >= (BitNumber + StopBits + 1))
	{
	  TRACE();
	  TX = 0;
	  bitTxCnt = 0;
	  cell_uXcsr->set_bit1(bmTx_byte);//signals end of tx
	  //generate end of tx interrupt request
	  cell_ircon2->set_bit1(bmUTXxIF);
	  if (UsartID == 0)
	    inform_partners(EV_UTX0, 0);
	  else
	    inform_partners(EV_UTX1, 0);
	  if (queue == 1)
	    {
	      TX = 1;
	      Txreg = cell_uXdbuf->get();	
	      bitTxCnt = 0;
	      queue = 0;
	      if (BitNumber == 9)//Parity enabled
		parity = parity_check(Txreg);
	    }
	  else
	    {
	      cell_uXcsr->set_bit0(bmActive);//no longer active
	    }
	}
    }
#ifdef USARTINFO
  fprintf(stderr, "TXpin: %s\n", OPin?"1":"0");
#endif
}

bool
cl_CC2530_usart::parity_check(t_mem byte)
{
   t_mem check = byte & 0x01;//Isolate LSB
#ifdef USARTINFO
   fprintf(stderr, "\n\nParity CHECK !!!\n");
   fprintf(stderr, "BYTEtoCheck: 0x%02x\tFirst Check: %d\n", byte, check);
#endif
  for (int i =0; i<7; i++)
    {
      byte >>= 1;
      check ^= (byte & 0x01);
#ifdef USARTINFO
      fprintf(stderr, "Itération: %d\tByte: 0x%02x\tCheck: %d\n", i, byte, check);
#endif
    }
  if ((cell_uXucr->get() & bmD9) == 0)//Odd parity
    {
#ifdef USARTINFO
      fprintf(stderr, "ODD Parity !!!\n\n");
#endif
      if (check == 0)
	return(true);
      else
	return(false);
    }
  else//even parity
    {
#ifdef USARTINFO
      fprintf(stderr, "EVEN Parity !!!\n\n");
#endif
      if (check == 0)
	return(false);
      else
	return(true);
    }

}

void 
cl_CC2530_usart::write(class cl_memory_cell *cell, t_mem *val)
{
  //TRACE();
  if (cell == cell_uXdbuf)
    {
      // TRACE();
      if (TX == 1)//already tx
	queue = 1;
      else
	{
	  //  TRACE();
	  TX = 1;
	  regForTx = *val;	
	  cell_uXcsr->set_bit1(bmActive);//active
	  bitTxCnt=0;
	  if (BitNumber == 9)//Parity enabled
	    {
	    TxParity = parity_check(regForTx);
#ifdef USARTINFO
	    fprintf(stderr, "Parity to TX: %s\n\n", TxParity?"1":"0");
#endif
	    }
	}
    }

  if (cell == cell_uXbaud)
    {
      baud_m = *val;
      BaudFactor = (256 + baud_m)*pow(2, baud_e)/(pow(2,28));
    }

  if (cell == cell_uXgcr)
    {
      baud_e = *val*bmBaud_e;
      BaudFactor = (256 + baud_m)*pow(2, baud_e)/(pow(2,28));
    }

 if (cell == cell_uXcsr)
    {
      if ((*val & bmRe) != 0)
	{
	  RX=1;
	  cell_uXcsr->set_bit1(bmActive);
	}
      if ((*val & bmSlave) != 0)
	slave = true;
    }

 if (cell == cell_uXucr)
    {
      if ((*val & (bmBit9 & bmParity<<1)) == 0)
	BitNumber = 8; 
      else 
	BitNumber = 9;

      switch(*val & (bmStart | bmStop))
	{
	case 0: 
	  fprintf(stderr, "Error: Make sure configuration of the UART stop-bit level is different from the start-bit level.\n");
	  break;
	case 1: startBitLevel = 1; stopBitLevel = 0; break;
	case 2: startBitLevel = 0; stopBitLevel = 1; break;
	case 3: 
	  fprintf(stderr, "Error: Make sure configuration of the UART stop-bit level is different from the start-bit level.\n");
	  break;
	default: break;
	}

      if ((*val & bmSPB) == 0)
	StopBits = 1; 
      else 
	StopBits = 2;

      if ((*val & bmFlush) == 1)
	{
	  cell_uXcsr->set_bit0(bmActive); 
	  TX = 0;
	  RX = 0;
	  cell_uXdbuf->set(0);
	  regForTx=0;
	  regForRx=0;
	}

    }
}

t_mem 
cl_CC2530_usart::read(class cl_memory_cell *cell)
{
  if (cell == cell_uXcsr)
    {
      cell_uXcsr->set_bit0(bmErr);//clear parity error bit 
      cell_uXcsr->set_bit0(bmFe);//clear Framing error bit
    }
  return (cell->get()); 
}

void
cl_CC2530_usart::print_info(class cl_console *con)
{
  print_info();
}


void
cl_CC2530_usart::print_info()
{

  char *modes[]= { "UART mode",
		   "SPI Master Mode",
		   "SPI Slave Mode" };
  int mod ;
  if (mode == 1)
    mod = 0;
  else if (mode == 0)
    {
      if (slave == 0)
	mod = 1;
      else
	mod = 2;
    }

  fprintf(stderr,"\n*****************  %s[%d] ", id_string, id);
  fprintf(stderr," %s******************\n", modes[mod]);
  fprintf(stderr,"%s Parity: %s\n",((cell_uXucr->get() & bmD9) == 0)?"Odd":"Even", 
	  (BitNumber == 9)?"enabled":"disabled");
  fprintf(stderr,"BaudRate: %g bits/s\tCC2530 Crystal: %d Hz", BaudRate, CC2530xtal);
  fprintf(stderr,"\n*********************************");
  fprintf(stderr,"****************************************\n\n");
}
