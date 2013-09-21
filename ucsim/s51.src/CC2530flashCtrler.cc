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
  flashbank4= uc->address_space(MEM_FLASHBANK4_ID);
  flashbank5= uc->address_space(MEM_FLASHBANK5_ID);
  flashbank6= uc->address_space(MEM_FLASHBANK6_ID);
  flashbank7= uc->address_space(MEM_FLASHBANK7_ID);
  flash = flashbank0;

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
  reset();
  return(0);
}

void 
cl_CC2530_flash_ctrler::reset(void)
{

  for (int i = 0; i<128; i++)
    {
      PageTab[i].StartAddr = i<<11;/* i*2048;(page size) */
      PageTab[i].NumberOfWrites = 0;
      PageTab[i].lockbit = false;
      for (int j = 0; j<PAGE_SIZE; j++)
	PageTab[i].WordTab[j].NumberOfWrites = 0;
    }

  writing = false;
  regFull = false;
  reg = 0;
  address = 0;
  PageNum = 0;
  WordNum = 0;
  TXNum = 0;
  dest = 0;
  val = 0;

  freq = 32000000;
  TickCount = 0;
  timerEnabled = false;

}

int
cl_CC2530_flash_ctrler::tick(int cycles)
{
  if (timerEnabled)
    TickCount++;
  return(resGO);
}

double
cl_CC2530_flash_ctrler::timer(void)
{
  return (TickCount/freq);
}

void
cl_CC2530_flash_ctrler::erasePage(int pageNumber)
{
  switch ((pageNumber & 0x70) >> 4)
    {
    case 0:flash = flashbank0; break;
    case 1:flash = flashbank1; break;
    case 2:flash = flashbank2; break;
    case 3:flash = flashbank3; break;
    case 4:flash = flashbank4; break;
    case 5:flash = flashbank5; break;
    case 6:flash = flashbank6; break;
    case 7:flash = flashbank7; break;
    }

  cell_fctl->set_bit1(bmBUSY);
  /*fprintf(stderr, "Erasing page 0x%04x! Start @ is 0x%04x\n", pageNumber, PageTab[pageNumber].StartAddr);*/
   int StartAddrInFlashbank = PageTab[pageNumber].StartAddr % FLASHBANK_SIZE;
  for (int i = 0; i < PAGE_SIZE; i++)//erasing each word of page
    {
      flash->write(StartAddrInFlashbank + i, 0xFF);
      PageTab[pageNumber].WordTab[i].NumberOfWrites = 0;
    }
  PageTab[pageNumber].NumberOfWrites = 0;
  cell_fctl->set_bit0(bmBUSY);
}

void
cl_CC2530_flash_ctrler::ChipErase(void)
{
  for (int i = 0; i< 128/*PageTab[].size()*/; i++)//Erases all pages
    {
      erasePage(i);
    }
}

void
cl_CC2530_flash_ctrler::flashWrite(void)
{
  TRACE();
  PageNum = (address >> 9) & 0x7F;//7 MSB of address correspond to the page number
  WordNum = address & 0x01FF;//9 LSB correspond to the word number in the page
  //if (!PageTab[PageNum].lockbit)//WARNING: right now the program to be executed by ucsim is stocked where the lockbits should be...
  //{
  //counting the number of times the page and word were written
      PageTab[PageNum].NumberOfWrites++;
      PageTab[PageNum].WordTab[WordNum].NumberOfWrites++;

      //each page can't be written more than 1024 times between erases
      if (PageTab[PageNum].NumberOfWrites > 1024)
	{
	  fprintf(stderr, "ERROR: Page %d was written too many times!\n", PageNum);
	}
      //each word can't be written more than 8 times between erases
      if (PageTab[PageNum].WordTab[WordNum].NumberOfWrites > 8)
	{
	  fprintf(stderr, "ERROR: Word %d in page %d was written too many times!\n", 
		  WordNum, PageNum);
	}

      int AddrInFlashbank = (address & 0x1FFF) <<2;//@ of word's 1st Byte
      //13 LSB of faddr are word address, flashbank number 
      //already selected when address is written to faddr registers, 1 word is 
      //4 bytes, address of first byte of the word in flashbank is word address * 4

      //writing the 4 bytes from the register to the word selected  in flash
      for (int i = 0; i<4; i++)
	{
	  //address of first byte of word, + i to write to next bytes of word
	  dest = AddrInFlashbank +i;
	  //shift reg value to isolate the 4 bytes
	  val = ((reg>>(8*i)) & 0xFF);
	  //Write in selected flashbank byte from register to corresponding byte @ 
	  flash->write(dest,val);
	  fprintf(stderr, "Flash Write of 0x%02x to address 0x%04x in %s!\n", val, dest, flash->get_name());
	}
      //case of lockbits 
      if ((PageNum == 0) && (WordNum < 4))
	{
	  for (int i = 0; i<32; i++)	    
	    PageTab[(3-WordNum)*32 + i].lockbit = (((reg>>i) & 1) == 0);	    
	}

      //reset of register
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
	  PageNum = (cell_faddrh->get() >> 1) & 0x7F;
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
      address = (*val<<8) + cell_faddrl->get();
      switch((address & 0xE000)>>14)
	{
	case 0:flash = flashbank0; break;
	case 1:flash = flashbank1; break;
	case 2:flash = flashbank2; break;
	case 3:flash = flashbank3; break;
	case 4:flash = flashbank4; break;
	case 5:flash = flashbank5; break;
	case 6:flash = flashbank6; break;
	case 7:flash = flashbank7; break;
	}
    }
  if (cell == cell_faddrl)
    {
      TRACE();
      address = (cell_faddrh->get()<<8) + *val;
      switch((address & 0xE000)>>14)
	{
	case 0:flash = flashbank0; break;
	case 1:flash = flashbank1; break;
	case 2:flash = flashbank2; break;
	case 3:flash = flashbank3; break;
	case 4:flash = flashbank4; break;
	case 5:flash = flashbank5; break;
	case 6:flash = flashbank6; break;
	case 7:flash = flashbank7; break;
	}
    }
  if (cell == cell_fwdata)
    {
      if (regFull)
	fprintf(stderr, "Wait until FCTL.FULL goes low to write to FWDATA!\n");
      else
	{//if writing enabled copy of fwdata in the register
	  if (writing)
	    {
	      TRACE();
	      if (TXNum == 0)
		{
		  reg = 0;
		  timerEnabled = true;
		}
	      reg = reg | (*val<<(8*TXNum));
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
