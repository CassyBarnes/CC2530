#include "CC2530flashCtrlercl.h"
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

#define bmBUSY 0x80
#define bmFULL 0x40
#define bmABORT 0x20
#define bmWRITE 0x02
#define bmERASE 0x01

cl_CC2530_flash_ctrler::cl_CC2530_flash_ctrler(class cl_uc *auc, int aid, char *aid_string): cl_hw(auc, HW_CC2530_FLASH_CONTROLLER, aid, aid_string)
{
  sfr= uc->address_space(MEM_SFR_ID);
  xram= uc->address_space(MEM_XRAM_ID);
  flashbank0= uc->address_space(MEM_FLASHBANK0_ID);
  flashbank1= uc->address_space(MEM_FLASHBANK1_ID);
  flashbank2= uc->address_space(MEM_FLASHBANK2_ID);
  flashbank3= uc->address_space(MEM_FLASHBANK3_ID);
  flash = flashbank0;
  freq = 32000000;
  TickCount = 0;
  timerEnabled = false;
  init();
  ChipErase();
}

int
cl_CC2530_flash_ctrler::init()
{
  TRACE();
  register_cell(xram, FADDRH, &cell_faddrh, wtd_restore_write);
  register_cell(xram, FADDRL, &cell_faddrl, wtd_restore_write);
  register_cell(xram, FCTL, &cell_fctl, wtd_restore_write);
  register_cell(xram, FWDATA, &cell_fwdata, wtd_restore_write);
  register_cell(sfr, CLKCONCMD, &cell_clkconcmd, wtd_restore_write);

  for (int i = 0; i<128; i++)
    PageTab[i].StartAddr = i<<10;/* *1024; */
}

int
cl_CC2530_flash_ctrler::tick(int cycles)
{
  if (timerEnabled)
    TickCount++;
}

double
cl_CC2530_flash_ctrler::timer(void)
{
  return (TickCount/freq);
}

void
cl_CC2530_flash_ctrler::erasePage(int pageNumber)
{
  if (pageNumber < 32)
    flash = flashbank0;
  else if (pageNumber < 64)
    {
      PageTab[pageNumber].StartAddr -= 0x8000;
      flash = flashbank1;
    }
  else if (pageNumber < 96)
    {
      PageTab[pageNumber].StartAddr -= 0x10000;
      flash = flashbank2;
    }
  else 
    {
      PageTab[pageNumber].StartAddr -= 0x18000;
      flash = flashbank3;
    }
  cell_fctl->set_bit1(bmBUSY);
  /*fprintf(stderr, "Erasing page 0x%04x! Start @ is 0x%04x\n", pageNumber, PageTab[pageNumber].StartAddr);*/
  for (int i = 0; i< 1024/*PageTab[pageNumber].size*/; i++)
    {
      flash->write(PageTab[pageNumber].StartAddr + i, 0xFF);
      PageTab[pageNumber].WordTab[i].NumberOfWrites = 0;
    }
  PageTab[pageNumber].NumberOfWrites = 0;
  cell_fctl->set_bit0(bmBUSY);
}

void
cl_CC2530_flash_ctrler::ChipErase(void)
{
  for (int i = 0; i< 128/*PageTab[].size()*/; i++)
    {
      erasePage(i);
    }
}

int
cl_CC2530_flash_ctrler::flashWrite(void)
{
  TRACE();
  PageNum = address & 0xFE00;
  WordNum = address & 0x01FF;
  //if (!PageTab[PageNum].lockbit)
  //{
      PageTab[PageNum].NumberOfWrites++;
      PageTab[PageNum].WordTab[WordNum].NumberOfWrites++;

      if (PageTab[PageNum].NumberOfWrites > 1024)
	{
	  fprintf(stderr, "ERROR: Page %d was written too many times!\n", PageNum);
	  return(0);
	}
      if (PageTab[PageNum].WordTab[WordNum].NumberOfWrites > 8)
	{
	  fprintf(stderr, "ERROR: Word %d in page %d was written too many times!\n", 
		  WordNum, PageNum);
	  return(0);
	}
      for (int i = 0; i<4; i++)
	{
	  dest = ((address & 0x3FFF)<<2)+i;
	  val = ((reg>>(8*i)) & 0xFF);
	  flash->write(dest,val);
	  fprintf(stderr, "Flash Write of %d to address %d in %s!\n", val, dest, flash->get_name());
	}

      if ((PageNum == 0) && (WordNum < 4))
	{
	  for (int i = 0; i<32; i++)	    
	    PageTab[(3-WordNum)*32 + i].lockbit = (((reg>>i) & 0x1) == 0);	    
	}

      reg = 0;
      cell_fctl->set_bit0(bmFULL);
      regFull = false;
      //cell_fctl->set_bit0(bmWRITE);
      //writing = false;

      //}
}

void
cl_CC2530_flash_ctrler::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_fctl)
    {
	  fprintf(stderr, "Value written to fctl is 0x%02x!\n", *val);
      if ((*val & bmERASE)!= 0)
	{
	  PageNum = cell_faddrh->get() & 0xFE;
	  fprintf(stderr, "Erasing page 0x%04x!\n", PageNum);
	  erasePage(PageNum);
	}
      if ((*val & bmWRITE)!=0)
	{
	  writing = true;
	}
      else
	  writing = false;
    }
  if (cell == cell_faddrh)
    {
      TRACE();
      address = *val<<8 + cell_faddrl->get();
      switch((address & 0xC000)>>14)
	{
	case 0:flash = flashbank0; break;
	case 1:flash = flashbank1; break;
	case 2:flash = flashbank2; break;
	case 3:flash = flashbank3; break;
	}
    }
  if (cell == cell_faddrl)
    {
      TRACE();
      address = cell_faddrh->get()<<8 + *val;
      switch((address & 0xC000)>>14)
	{
	case 0:flash = flashbank0; break;
	case 1:flash = flashbank1; break;
	case 2:flash = flashbank2; break;
	case 3:flash = flashbank3; break;
	}
    }
  if (cell == cell_fwdata)
    {
      if (regFull)
	fprintf(stderr, "Wait until FCTL.FULL goes low to write to FWDATA!\n");
      else
	{
	  if (writing)
	    {
	      TRACE();
	      if (TXNum == 0)
		timerEnabled = true;
	      reg = reg + (*val<<(8*TXNum));
	      fprintf(stderr, "REGISTER value is 0x%08x!\n", reg);
	      TXNum++;
	      if (TXNum == 4)
		{
		  TRACE();
		  cell_fctl->set_bit1(bmFULL);
		  regFull = true;
		  TXNum = 0;
		  if (timer() > 0.00002)
		    {
		      fprintf(stderr, "Write not finished within 20 µs!\n");	  
		    }
		  timerEnabled = false;
		  TickCount = 0;
		  flashWrite();
		}
	    }
	  else
	    {
	      fprintf(stderr, "Set FCTL.WRITE before writing!\n");
	    }
	}
    }
  if (cell == cell_clkconcmd)
    {
      freq = 32000000 << (*val & 7);
      if ((*val & 7)>1)
	fprintf(stderr, "It is recommended to keep CLKCONSTA.CLKSPD at 000 or 001 while writing to the flash.\n ");

    }
}
