#include "CC2530DMAcl.h"
#include <assert.h>
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"
//#include <algorithm>    // std::min

#define DEBUG
#ifdef DEBUG
#define TRACE() \
fprintf(stderr, "%s:%d in %s()\n", __FILE__, __LINE__, __FUNCTION__)
#else
#define TRACE()
#endif

#define TESTING
#define min(a,b) (a<=b?a:b)

cl_CC2530_dma::cl_CC2530_dma(class cl_uc *auc, int aid, char *aid_string):
  cl_hw(auc, HW_CC2530_DMA, aid, aid_string)
{
  sfr= uc->address_space(MEM_SFR_ID);
  xram= uc->address_space(MEM_XRAM_ID);
  init();
}

int
cl_CC2530_dma::init()
{
  register_cell(sfr, DMAARM, &cell_dmaarm, wtd_restore_write);
  register_cell(sfr, DMAREQ, &cell_dmareq, wtd_restore_write);
  register_cell(sfr, DMAIRQ, &cell_dmairq, wtd_restore_write);

#ifdef TESTING
  TRACE();
  t_mem config0[] = {0x80, 2, 0, 0, 0x40, 0x20, 0xA2, 0xA0};
  t_mem config1[] = {0x80, 0x40, 0, 0x50, 0x40, 0x20, 0x47, 0xF0};
  t_mem config2[] = {0, 0, 0, 0, 0x40, 0x20, 0x20, 0x50};

  for (int j = 0; j<8; j++)
    {
      xram->write(0x80 + j, config0[j]);
      //if (j==6)
      //fprintf(stderr, "Write of trigger at @ 0x%04x\n", 0x80 + j);
    }
  for (int i = 1; i<5; i++)
    {
      for (int j = 0; j<8; j++)
	{
	  if (i == 1)
	    {
	      xram->write(0x90 + j, config1[j]); 
	      //if (j==6)
	      //fprintf(stderr, "Write of trigger at @ 0x%04x\n", 0x90 + j);
	    }
	  else
	    {
	      xram->write(0x90 + 32*(i-1) + j, config2[j]);
	      //if (j==6)
		//fprintf(stderr, "Write of trigger at @ 0x%04x\n", 0x90 + 32*(i - 1) + j);
	    }
	}
    }
  sfr->write(DMA0CFGH, 0);
  sfr->write(DMA0CFGL, 0x80);
  sfr->write(DMA1CFGH, 0);
  sfr->write(DMA1CFGL, 0x90);
  //fprintf(stderr,"Checking @DMA0CFGL 0x%04x\n", sfr->read(DMA0CFGL));

#endif

  tabDMACh[0].ConfigAddress = (sfr->read(DMA0CFGH)<<8) + sfr->read(DMA0CFGL); 
  //fprintf(stderr, "Channel 0 config @ 0x%04x\n", tabDMACh[0].ConfigAddress);
  for (int i = 1; i<5; i++)
    {
      tabDMACh[i].ConfigAddress = (sfr->read(DMA1CFGH)<<8) + sfr->read(DMA1CFGL) 
	+ 32*(i-1); 
      // fprintf(stderr, "Channel %d config @ 0x%04x\n", i, tabDMACh[i].ConfigAddress);
    }

  for (int i = 0; i<5; i++)
    {
      t_addr CFGaddr = tabDMACh[i].ConfigAddress;
      tabDMACh[i].armed = tabDMACh[i].TransferCount = 0;
      ObjInit = true;
      LoadConfig(i, CFGaddr);
      ObjInit = false;
    }
}

int
cl_CC2530_dma::tick(int cycles)
{
  TRACE(); 
  fprintf(stderr, "\n************* %s *************\n", id_string);
  fprintf(stderr, "tick! \n");
  flagsReg = sfr->read(0x8E);
  fprintf(stderr, "Event flag: 0x%04x\n", flagsReg);
  for (int i = 1; i<5; i++)
    fprintf(stderr, "Channel %d max len  0x%04x\n", i, tabDMACh[i].LEN);
  if (flagsReg != 0)
    {

      fprintf(stderr, "Flagsreg: 0x%04x \n", flagsReg);

      tabDMACh[0].ConfigAddress = (sfr->read(DMA0CFGH)<<8) + sfr->read(DMA0CFGL); 
      for (int i = 1; i<5; i++)
	{
	  tabDMACh[i].ConfigAddress = (sfr->read(DMA1CFGH)<<8) + sfr->read(DMA1CFGL) 
	    + 32*(i-1); 
	  
	}

      transferTest();
      flagsReg = 0;
      sfr->write(0x8E, 0);
    }
  return(resGO);
}

void
cl_CC2530_dma::transferTest(void)
{
  for (int i = 0; i < 5; i++)
    {
      TRACE();
      t_addr CFGaddr = tabDMACh[i].ConfigAddress;
      tabDMACh[i].armed = ((sfr->read(DMAARM) & (1<<i)) != 0);
      if (tabDMACh[i].armed)
	{
  fprintf(stderr, "Trigger: 0x%04x Other: %s 0x%04x 0x%04x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x  \n",tabDMACh[i].TriggerEvent, tabDMACh[i].armed?"1":"0",tabDMACh[i].source, tabDMACh[i].destination, tabDMACh[i].VLEN, tabDMACh[i].LEN, tabDMACh[i].Byte_Word_tx, tabDMACh[i].TxMode, tabDMACh[i].SRCincrement,  tabDMACh[i].DESTincrement);
	  TRACE();
	  LoadConfig(i, CFGaddr);
	  tabDMACh[i].TriggerEvent = ((xram->read(CFGaddr + 6)) & 0x1F);
	  fprintf(stderr, 
		  "Flagsreg: 0x%04x Trigger for channel %d: 0x%04x from @ 0x%04x\n", 
		  flagsReg, i, 
		  tabDMACh[i].TriggerEvent,
		  CFGaddr + 6);
	  if (((1<<tabDMACh[i].TriggerEvent) & flagsReg) != 0)
	    {
	      TRACE();
	      transfer(i);
	    }
	}
    }
}

void
cl_CC2530_dma::transferTest(int i)
{
  TRACE();
  t_addr CFGaddr = tabDMACh[i].ConfigAddress;
  fprintf(stderr, "Trigger: 0x%04x Other: %s 0x%04x 0x%04x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x  \n",tabDMACh[i].TriggerEvent, tabDMACh[i].armed?"1":"0",tabDMACh[i].source, tabDMACh[i].destination, tabDMACh[i].VLEN, tabDMACh[i].LEN, tabDMACh[i].Byte_Word_tx, tabDMACh[i].TxMode, tabDMACh[i].SRCincrement,  tabDMACh[i].DESTincrement);
  if (tabDMACh[i].armed)
    {
      tabDMACh[i].TriggerEvent = ((xram->read(CFGaddr + 6)) & 0x1F);
      LoadConfig(i, CFGaddr);
      fprintf(stderr, "Flagsreg: 0x%04x Trigger: 0x%04x \n", flagsReg, 
	      tabDMACh[i].TriggerEvent);
      if (Req || (((1<<tabDMACh[i].TriggerEvent) & flagsReg) != 0))
	{
	  transfer(i);
	}
    }    
}

void
cl_CC2530_dma::transfer(int i)
{
  TRACE();
  if (tabDMACh[i].single)
    {
      TRACE();
      DMAwrite(i);
      tabDMACh[i].s += tabDMACh[i].SRCinc;//*Byte_Word_tx;
      tabDMACh[i].d += tabDMACh[i].DESTinc;//*Byte_Word_tx;
      tabDMACh[i].TransferCount--;
    }
  else
    {
      TRACE();
      while(DMAwrite(i))
	{
	  tabDMACh[i].s +=  tabDMACh[i].SRCinc;//*Byte_Word_tx;
	  tabDMACh[i].d +=  tabDMACh[i].DESTinc;//*Byte_Word_tx;
	  tabDMACh[i].TransferCount--;
	}
    }
}


bool
cl_CC2530_dma::DMAwrite(int ChannelID)
{
  TRACE();

  if (tabDMACh[ChannelID].TransferCount == 0)
    {
      //To add: Notify CPU
      fprintf(stderr, "End of transfer!!! \n");
      if (tabDMACh[ChannelID].repeated)
	tabDMACh[ChannelID].armed = 1;
      else
	{
	  tabDMACh[ChannelID].armed = 0;
	  cell_dmaarm->set_bit0(1<<ChannelID);
	}
      tabDMACh[ChannelID].TxEnd = true;
      tabDMACh[ChannelID].init = false;
      tabDMACh[ChannelID].idle = true;
      flagsReg = 0;
      sfr->write(0x8E, 0);
      return(false);
    }
  xram->write(tabDMACh[ChannelID].d, xram->read(tabDMACh[ChannelID].s));
  //fprintf(stderr, "Write of 0x%02x (@ 0x%04x) to @ 0x%04x ! \n", xram->read(s), s, d);
  if (tabDMACh[ChannelID].Byte_Word_tx == 1)//Word tx
    {
      TRACE();
      xram->write((tabDMACh[ChannelID].d)+1, xram->read((tabDMACh[ChannelID].s)+1));
    }
  return(true);
}


void
cl_CC2530_dma::LoadConfig(int i, t_addr CFGaddr)
{
  tabDMACh[i].source = (xram->read(CFGaddr)<<8) 
    + xram->read(CFGaddr + 1);
  tabDMACh[i].destination = (xram->read(CFGaddr + 2)<<8) 
    + xram->read(CFGaddr + 3);
  tabDMACh[i].VLEN = (xram->read(CFGaddr + 4) & 0xE0)>>5;
  tabDMACh[i].LEN = ((xram->read(CFGaddr + 4) & 0x1F)<< 8)
    +xram->read(CFGaddr + 5);
  tabDMACh[i].Byte_Word_tx = ((xram->read(CFGaddr + 6)) & 0x80)>>7;
  tabDMACh[i].TxMode = ((xram->read(CFGaddr + 6)) & 0x60)>>5;
  tabDMACh[i].SRCincrement = ((xram->read(CFGaddr + 7)) & 0xC0)>>6;
  tabDMACh[i].DESTincrement = ((xram->read(CFGaddr + 7))& 0x30)>>4;
  tabDMACh[i].ITmask = ((xram->read(CFGaddr + 7)) & 0x08)>>3;
  tabDMACh[i].M8 = ((xram->read(CFGaddr + 7)) & 0x04)>>2;
  tabDMACh[i].Priority = (xram->read(CFGaddr + 7)) & 0x03;

  if (!tabDMACh[i].init && !ObjInit)
    {
      fprintf(stderr, "Transfer init!!! \n");
      tabDMACh[i].s = tabDMACh[i].source;
      tabDMACh[i].d = tabDMACh[i].destination;
      tabDMACh[i].TxEnd = false;

      switch(tabDMACh[i].VLEN)
	{
	case 1: tabDMACh[i].delta = 1; break;
	case 2: tabDMACh[i].delta = 0; break;
	case 3: tabDMACh[i].delta = 2; break;
	case 4: tabDMACh[i].delta = 3; break;
	}

      tabDMACh[i].TransferCount = min(tabDMACh[i].LEN, xram->read(tabDMACh[i].s) + tabDMACh[i].delta);
  
      switch(tabDMACh[i].VLEN)
	{
	case 0: case 7: tabDMACh[i].TransferCount = tabDMACh[i].LEN; break;
	case 5: case 6: tabDMACh[i].TransferCount = min(tabDMACh[i].LEN,xram->read(tabDMACh[i].s));
	  break;
	}

      switch(tabDMACh[i].SRCincrement)
	{
	case 0: tabDMACh[i].SRCinc = 0; break;
	case 1: tabDMACh[i].SRCinc = 1; break;
	case 2: tabDMACh[i].SRCinc = 2; break;
	case 3: tabDMACh[i].SRCinc = -1; break;
	}
      switch(tabDMACh[i].DESTincrement)
	{
	case 0: tabDMACh[i].DESTinc = 0; break;
	case 1: tabDMACh[i].DESTinc = 1; break;
	case 2: tabDMACh[i].DESTinc = 2; break;
	case 3: tabDMACh[i].DESTinc = -1; break;
	}
 
      fprintf(stderr, "Transfering : 0x%04x %s, Maximum length is: 0x%04x \n", 
	      tabDMACh[i].TransferCount, 
	      (tabDMACh[i].Byte_Word_tx == 0)?"Bytes":"Words", 
	      tabDMACh[i].LEN);
      tabDMACh[i].single =(tabDMACh[i].TxMode & 1) == 0;
      tabDMACh[i].repeated =(tabDMACh[i].TxMode & 2) == 2;
      fprintf(stderr, "Repeated : %s\n",(tabDMACh[i].repeated)?"yes":"no");
      tabDMACh[i].init = true;
    }
}

void 
cl_CC2530_dma::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_dmareq) 
    {
      TRACE();
      for (int i = 0; i < 5; i++)
	{
	  if ((*val & (1<<i)) != 0)
	    {
	      TRACE();
	      Req = true;
	      transferTest(i);
	      Req = false;
	    }
	}
    }
  if (cell == cell_dmaarm) 
    {
      for (int i = 0; i < 5; i++)
	tabDMACh[i].armed = (*val & (1<<i) != 0);
      if ((*val & 0x80) != 0)//abort
	{
	  cell_dmaarm->set(0);
	  for (int i = 0; i < 5; i++)
	    tabDMACh[i].armed = false;
	}
    }
  if (cell == cell_dmairq) 
    {
    }
}
