#include <algorithm>    // std::min//Bug while compiling
#include <assert.h>
using namespace std;
#include "CC2530DMAcl.h"
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"


#define DEBUG
#define DMAINFO
#ifdef DMAINFO
#define DEBUG
#endif

#ifdef DEBUG
#define TRACE() \
 fprintf(stderr, "%s:%d in %s()\n", __FILE__, __LINE__, __FUNCTION__)
#define TRACE_STR(str, ...)						\
 fprintf(stderr, "%s:%d in %s()\n" str, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#else
#define TRACE()
#define TRACE_STR(str, ...)
#endif


#define bmDMAIF 1
#define TESTING
//#define min(a,b) (a<=b?a:b)

cl_CC2530_dma::cl_CC2530_dma(class cl_uc *auc, int aid, char *aid_string):
  cl_hw(auc, HW_CC2530_DMA, aid, aid_string)
{
  sfr= uc->address_space(MEM_SFR_ID);
  xram= uc->address_space(MEM_XRAM_ID);

  register_cell(sfr, DMAARM, &cell_dmaarm, wtd_restore_write);
  register_cell(sfr, DMAREQ, &cell_dmareq, wtd_restore_write);
  register_cell(sfr, DMAIRQ, &cell_dmairq, wtd_restore_write);
  register_cell(sfr, IRCON, &cell_ircon, wtd_restore_write);

  init();
}

int
cl_CC2530_dma::init()
{
  ObjInit = false;
  // struct DMAchannel tabDMACh[5];
  for (int i = 0; i<5; i++)
    channel_init(i);


#ifdef TESTING
  TRACE();
  t_mem config0[] = {0x80, 3, 0, 0, 0x40, 0x10, 0xAA, 0xA0};
  t_mem config1[] = {0x80, 0x40, 0, 0x50, 0x40, 0x15, 0x47, 0xF0};
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

  // Check Configuration addresses in SFR bank (track any change of SFR reg)
  config_channel_addresses();

  for (int i = 0; i<5; i++)
    {
      t_addr CFGaddr = tabDMACh[i].ConfigAddress;
      tabDMACh[i].armed = 0;
      tabDMACh[i].TransferCount = 0;

      ObjInit = true;
      LoadConfig(i, CFGaddr);
      ObjInit = false;
    }
#endif

  return(0);
}

void
cl_CC2530_dma::reset(void)
{
  ObjInit = false;
  for (int i = 0; i<5; i++)
    channel_init(i);
}

void cl_CC2530_dma::channel_init(int i)
{
  tabDMACh[i].source = t_addr(0);
  tabDMACh[i].destination = t_addr(0);
  tabDMACh[i].ConfigAddress = t_addr(0);
  tabDMACh[i].s = t_addr(0);
  tabDMACh[i].d = t_addr(0);
  tabDMACh[i].ITmask = false;
  tabDMACh[i].armed = false;
  tabDMACh[i].single = false;
  tabDMACh[i].repeated = false;
  tabDMACh[i].triggered = false;
  tabDMACh[i].TransferCount = int(0);
  tabDMACh[i].VLEN = int(0);
  tabDMACh[i].LEN = int(0);
  tabDMACh[i].Priority = int(0);
  tabDMACh[i].delta = int(0);
  tabDMACh[i].TriggerEvent = int(32);
  tabDMACh[i].SRCincrement = int(0);
  tabDMACh[i].DESTincrement = int(0);
  tabDMACh[i].SRCinc = int(0);
  tabDMACh[i].DESTinc = int(0);
  tabDMACh[i].TxMode = int(0);
  tabDMACh[i].Byte_Word_tx = int(0);
  tabDMACh[i].M8 = int(0);
  tabDMACh[i].DmaChState = DMAChannelIdle;
}


void cl_CC2530_dma::config_channel_addresses()
{
  // 5 channels and only 2 configuration addresses:
  // - 1st configuration address is in DMA0CFGH:
  // tabDMACh[0].ConfigAddress = (sfr->read(DMA0CFGH)<<8) + sfr->read(DMA0CFGL);
  tabDMACh[0].ConfigAddress = 0x80;
  // - 2nd configuration address is in DMA1CFGH for the remaining 4 channels:
  t_addr CFGaddr1 = (sfr->read(DMA1CFGH)<<8) + sfr->read(DMA1CFGL);
  tabDMACh[1].ConfigAddress = 0x90;
  for (int i = 2; i<5; i++)
    {
      tabDMACh[i].ConfigAddress = CFGaddr1 + 32*(i-1);
    }
}

int
cl_CC2530_dma::tick(int cycles)
{
  TRACE();
#ifdef DMAINFO
  fprintf(stderr, "\n************* %s *************\n", id_string);
  fprintf(stderr, "tick! \n");
#endif

  // Check Configuration addresses in SFR bank (track any change of SFR reg)
  config_channel_addresses();
  for (int i = 0; i < 5; i++)
    {

      //TRACE_STR("Old State of Ch %d: %d\n", i, tabDMACh[i].DmaChState);
      StateTrans(i);

      //fprintf(stderr, "New State of Ch %d: %d\n", i, tabDMACh[i].DmaChState);
    }
  return(resGO);
}

void
cl_CC2530_dma::happen(class cl_hw *where, enum hw_event he, void *params)
{
  int Trigger;
  switch(he)
    {
    case EV_T1_CH0:    Trigger = 2;   TRACE(); break;
    case EV_T1_CH1:    Trigger = 3;   break;
    case EV_T1_CH2:    Trigger = 4;   break;
    case EV_T2_EVENT1: Trigger = 5;   break;
    case EV_T2_EVENT2: Trigger = 6;   break;
    case EV_T3_CH0:    Trigger = 7;   break;
    case EV_T3_CH1:    Trigger = 8;   break;
    case EV_T4_CH0:    Trigger = 9;   break;
    case EV_T4_CH1:    Trigger = 10;  break;
    case EV_ST:        Trigger = 11;  break;
    case EV_IOC_0:     Trigger = 12;  break;
    case EV_IOC_1:     Trigger = 13;  break;
    case EV_URX0:      Trigger = 14;  break;
    case EV_UTX0:      Trigger = 15;  break;
    case EV_URX1:      Trigger = 16;  break;
    case EV_UTX1:      Trigger = 17;  break;
    case EV_FLASH:     Trigger = 18;  break;
    case EV_RADIO:     Trigger = 19;  break;
    case EV_DBG_BW:    Trigger = 31;  break;
    default: break;
    }

  for (int i = 0; i< 5; i++)
    {
      if (tabDMACh[i].TriggerEvent == Trigger)
	{
	tabDMACh[i].triggered = true;
	fprintf(stderr, "Event %d triggered Channel %d of the DMA\n", Trigger, i);
	}
    }
}


void
cl_CC2530_dma::transfer(int i)
{
  TRACE();

    // 1. Transfer One Byte or Word When channel is granted access:
    // 1.1. First byte
    xram->write(tabDMACh[i].d, xram->read(tabDMACh[i].s));
    fprintf(stderr, "DMA transfer on channel %d: Moving 0x%02x from 0x%04x to 0x%04x\n", i, xram->read(tabDMACh[i].s), tabDMACh[i].s, tabDMACh[i].d);
    // 1.2. Shall we transmit a second byte ?
    if (tabDMACh[i].Byte_Word_tx == 1) // Word tx
      {
	TRACE();
	if (tabDMACh[i].DESTinc >= 0)
	  {
	    xram->write((tabDMACh[i].d)+1, xram->read((tabDMACh[i].s)+1));
	    fprintf(stderr, "DMA transfer on channel %d: Moving 0x%02x from 0x%04x to 0x%04x\n", i, xram->read(tabDMACh[i].s + 1), tabDMACh[i].s + 1, tabDMACh[i].d + 1);
	  }
	else
	  {
	    xram->write((tabDMACh[i].d)-1, xram->read((tabDMACh[i].s)-1));
	    fprintf(stderr, "DMA transfer on channel %d: Moving 0x%02x from 0x%04x to 0x%04x\n", i, xram->read(tabDMACh[i].s-1), tabDMACh[i].s-1, tabDMACh[i].d-1);	
	  }
      }

    // 2. Update address counters:
    tabDMACh[i].s +=  tabDMACh[i].SRCinc;//*Byte_Word_tx;
    tabDMACh[i].d +=  tabDMACh[i].DESTinc;//*Byte_Word_tx;
    tabDMACh[i].TransferCount--;
}



void
cl_CC2530_dma::LoadConfig(int i, t_addr CFGaddr)
{
  channel_init(i);
  TRACE();
  tabDMACh[i].source      = (xram->read(CFGaddr)<<8) + xram->read(CFGaddr + 1);
  tabDMACh[i].destination = (xram->read(CFGaddr + 2)<<8) + xram->read(CFGaddr + 3);
  tabDMACh[i].VLEN        = (xram->read(CFGaddr + 4) & 0xE0)>>5;
  tabDMACh[i].LEN         = ((xram->read(CFGaddr + 4) & 0x1F)<< 8) + xram->read(CFGaddr + 5);
  tabDMACh[i].Byte_Word_tx  = ((xram->read(CFGaddr + 6)) & 0x80)>>7;
  tabDMACh[i].TxMode        = ((xram->read(CFGaddr + 6)) & 0x60)>>5;
  tabDMACh[i].TriggerEvent  = ((xram->read(CFGaddr + 6)) & 0x1F);
  tabDMACh[i].SRCincrement  = ((xram->read(CFGaddr + 7)) & 0xC0)>>6;
  tabDMACh[i].DESTincrement = ((xram->read(CFGaddr + 7)) & 0x30)>>4;
  tabDMACh[i].ITmask        = (((xram->read(CFGaddr + 7)) & 0x08)>>3)!=0;
  tabDMACh[i].M8            = ((xram->read(CFGaddr + 7)) & 0x04)>>2;
  tabDMACh[i].Priority      = (xram->read(CFGaddr + 7)) & 0x03;


  if (!ObjInit)
    {
      //TRACE_STR("Transfer init!!! \n");

      tabDMACh[i].s = tabDMACh[i].source;
      tabDMACh[i].d = tabDMACh[i].destination;

      switch(tabDMACh[i].VLEN)
	{
	case 1: tabDMACh[i].delta = 1; break;
	case 2: tabDMACh[i].delta = 0; break;
	case 3: tabDMACh[i].delta = 2; break;
	case 4: tabDMACh[i].delta = 3; break;
	default: tabDMACh[i].delta = int(0); break;
	}

      switch(tabDMACh[i].VLEN)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	  // TODO: handle the case where Byte_Word_tx == 1 (length is
	  // then encoded on 2 bytes):
	  tabDMACh[i].TransferCount = min(tabDMACh[i].LEN,
					  int(xram->read(tabDMACh[i].s)) +
					  tabDMACh[i].delta);
	  break;
	case 0: case 7: tabDMACh[i].TransferCount = tabDMACh[i].LEN; break;
	case 5: case 6:
	  tabDMACh[i].TransferCount = min(tabDMACh[i].LEN,
					  int(xram->read(tabDMACh[i].s)));
	  break;
	default:
	  assert(false);
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

#ifdef DMAINFO
      fprintf(stderr, "Transfering on channel %d: 0x%04x %s, Maximum length is: 0x%04x \n",i,
	      tabDMACh[i].TransferCount,
	      (tabDMACh[i].Byte_Word_tx == 0)?"Bytes":"Words",
	      tabDMACh[i].LEN);
#endif
      tabDMACh[i].single   = (tabDMACh[i].TxMode & 1) == 0;
      tabDMACh[i].repeated = (tabDMACh[i].TxMode & 2) == 2;
#ifdef DMAINFO
      fprintf(stderr, "Repeated : %s\n",(tabDMACh[i].repeated)?"yes":"no");
#endif
      tabDMACh[i].init = true;
    }
}

void
cl_CC2530_dma::StateTrans(int i)
{
  switch(tabDMACh[i].DmaChState)
    {
    case DMAChannelIdle:
      if (tabDMACh[i].armed)
	tabDMACh[i].DmaChState = LoadDMAChannelConfig;
      break;

    case LoadDMAChannelConfig: 
      // Read configuration of channel from memory:
      LoadConfig(i, tabDMACh[i].ConfigAddress);
      // Move to next state:
      tabDMACh[i].DmaChState = DMAChannelArmed; 
      break;

    case DMAChannelArmed:
      // Wait for event to trigger transfer:
      if (tabDMACh[i].triggered) {
	tabDMACh[i].triggered = false; // Ok, taken in account, clear the bit...
	tabDMACh[i].DmaChState = TransferOneByteOrWord;
      }
      break;

    case TransferOneByteOrWord:
      // 1. Transfer 1 or 2 bytes:
      transfer(i);

      // 2. Whenever the transfer count has been reached:
      if (tabDMACh[i].TransferCount <= 0) {
	//  TRACE_STR("End of transfer!!! \n");

	  // 2.1. Set interrupt flag
	  cell_dmairq->set_bit1(1<<i);
	  if (tabDMACh[i].ITmask)
	    cell_ircon->set_bit1(bmDMAIF);

	// 2.3 If channel is programmed to repeat the transfer, reload config and go:
	if (tabDMACh[i].repeated)
	  tabDMACh[i].DmaChState = LoadDMAChannelConfig;
	else
	  // 2.4 Or disam and move to IDLE state
	  tabDMACh[i].DmaChState = DisarmChannel;
	break;
      }

      // 4. Otherwise, for non-block DMA, wait for an event to resume transfer (1 or 2 byte at a time):
      if (tabDMACh[i].single              &&
	  !tabDMACh[i].triggered) // No triggering event raised.
	{
	  tabDMACh[i].DmaChState = DMAChannelArmed; // Wait for event.
	  break;
	}

      // 5. By default, stay in current state
      break;

    case DisarmChannel: 
      tabDMACh[i].armed = false;
      tabDMACh[i].DmaChState = DMAChannelIdle;
      break;

    default:
      assert(!"Unknown state");
      break;
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
	      tabDMACh[i].triggered = true; // Software triggers the channel.
	    }
	}
    }

  if (cell == cell_dmaarm)
    {
      for (int i = 0; i < 5; i++)
	tabDMACh[i].armed = ((*val & (1<<i)) != 0);

      if ((*val & (1 << 7)) != 0) // ABORT command (bit 7)
	{
	  cell_dmaarm->set(0);
	  // Disarm every channel
	  for (int i = 0; i < 5; i++)
	    tabDMACh[i].armed = false;
	}
    }

  if (cell == cell_dmairq)
    {
      for (int i = 0; i < 5; i++)
	{
	  if ((*val & 1<<i)!=0)
	    tabDMACh[i].ITmask = true;
	}
    }
}

#undef DEBUG
