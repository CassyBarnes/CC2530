/*
 * Simulator of microcontrollers (interruptcl.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#ifndef INTERRUPTCL_HEADER
#define INTERRUPTCL_HEADER

#include "stypes.h"
#include "pobjcl.h"
#include "uccl.h"

#include "newcmdcl.h"


class cl_interrupt: public cl_hw
{
public:
  class cl_address_space *sfr;
  bool was_reti, bit_IT0, bit_IT1, bit_INT0, bit_INT1;
  class cl_memory_cell *cell_tcon;
public:
  cl_interrupt(class cl_uc *auc);
  virtual int init(void);

  virtual void added_to_uc(void);

  //virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);

  //virtual void mem_cell_changed(class cl_m *mem, t_addr addr);

  virtual int tick(int cycles);
  virtual void reset(void);
  virtual void happen(class cl_hw *where, enum hw_event he, void *params);

  virtual void print_info(class cl_console *con);
};

class cl_radio: public cl_hw
{
public:
  class cl_address_space *sfr;
  class cl_memory_cell *cell_RFIRQF1;
  class cl_memory_cell *cell_U0CSR;
  class cl_memory_cell *cell_P0IFG;
  class cl_memory_cell *cell_P1IFG;
  class cl_memory_cell *cell_P2IFG;
  class cl_memory_cell *cell_PICTL;
  class cl_memory_cell *cell_P1IEN;
  class cl_memory_cell *cell_P0INP;
  class cl_memory_cell *cell_DPS;
  class cl_memory_cell *cell_MPAGE;
  class cl_memory_cell *cell_T2CTRL;
  class cl_memory_cell *cell_ST0;
  class cl_memory_cell *cell_ST1;
  class cl_memory_cell *cell_ST2;
  class cl_memory_cell *cell_IEN2;
  class cl_memory_cell *cell_S1CON;
  class cl_memory_cell *cell_T2EVTCFG;
  class cl_memory_cell *cell_SLEEPSTA;
  class cl_memory_cell *cell_CLKCONSTA;
  class cl_memory_cell *cell_FMAP;
  class cl_memory_cell *cell_T2IRQF;
  class cl_memory_cell *cell_T2M0;
  class cl_memory_cell *cell_T2M1;
  class cl_memory_cell *cell_T2MOVF0;
  class cl_memory_cell *cell_T2MOVF1;
  class cl_memory_cell *cell_T2MOVF2;
  class cl_memory_cell *cell_T2IRQM;
  class cl_memory_cell *cell_IEN1;
  class cl_memory_cell *cell_P0IEN;
  class cl_memory_cell *cell_P2IEN;
  class cl_memory_cell *cell_STLOAD;
  class cl_memory_cell *cell_PMUX;
  class cl_memory_cell *cell_T1STAT;
  class cl_memory_cell *cell_ENCDI;
  class cl_memory_cell *cell_ENCDO;
  class cl_memory_cell *cell_ENCCS;
  class cl_memory_cell *cell_ADCCON1;
  class cl_memory_cell *cell_ADCCON2;
  class cl_memory_cell *cell_ADCCON3;
  class cl_memory_cell *cell_IP1;
  class cl_memory_cell *cell_ADCL;
  class cl_memory_cell *cell_ADCH;
  class cl_memory_cell *cell_RNDL;
  class cl_memory_cell *cell_RNDH;
  class cl_memory_cell *cell_SLEEPCMD;
  class cl_memory_cell *cell_RFERRF;
  class cl_memory_cell *cell_IRCON;
  class cl_memory_cell *cell_U0DBUF;
  class cl_memory_cell *cell_U0BAUD;
  class cl_memory_cell *cell_T2MSEL;
  class cl_memory_cell *cell_U0UCR;
  class cl_memory_cell *cell_U0GCR;
  class cl_memory_cell *cell_CLKCONCMD;
  class cl_memory_cell *cell_WDCTL;
  class cl_memory_cell *cell_T3CNT;
  class cl_memory_cell *cell_T3CTL;
  class cl_memory_cell *cell_T3CCTL0;
  class cl_memory_cell *cell_T3CC0;
  class cl_memory_cell *cell_T3CCTL1;
  class cl_memory_cell *cell_T3CC1;
  class cl_memory_cell *cell_DMAIRQ;
  class cl_memory_cell *cell_DMA1CFGL;
  class cl_memory_cell *cell_DMA1CFGH;
  class cl_memory_cell *cell_DMA0CFGL;
  class cl_memory_cell *cell_DMA0CFGH;
  class cl_memory_cell *cell_DMAARM;
  class cl_memory_cell *cell_DMAREQ;
  class cl_memory_cell *cell_TIMIF;
  class cl_memory_cell *cell_RFD;
  class cl_memory_cell *cell_T1CC0L;
  class cl_memory_cell *cell_T1CC0H;
  class cl_memory_cell *cell_T1CC1L;
  class cl_memory_cell *cell_T1CC1H;
  class cl_memory_cell *cell_T1CC2L;
  class cl_memory_cell *cell_T1CC2H;
  class cl_memory_cell *cell_RFST;
  class cl_memory_cell *cell_T1CNTL;
  class cl_memory_cell *cell_T1CNTH;
  class cl_memory_cell *cell_T1CTL;
  class cl_memory_cell *cell_T1CCTL0;
  class cl_memory_cell *cell_T1CCTL1;
  class cl_memory_cell *cell_T1CCTL2;
  class cl_memory_cell *cell_IRCON2;
  class cl_memory_cell *cell_RFIRQF0;
  class cl_memory_cell *cell_T4CNT;
  class cl_memory_cell *cell_T4CTL;
  class cl_memory_cell *cell_T4CCTL0;
  class cl_memory_cell *cell_T4CC0;
  class cl_memory_cell *cell_T4CCTL1;
  class cl_memory_cell *cell_T4CC1;
  class cl_memory_cell *cell_PERCFG;
  class cl_memory_cell *cell_APCFG;
  class cl_memory_cell *cell_P0SEL;
  class cl_memory_cell *cell_P1SEL;
  class cl_memory_cell *cell_P2SEL;
  class cl_memory_cell *cell_P1INP;
  class cl_memory_cell *cell_P2INP;
  class cl_memory_cell *cell_U1CSR;
  class cl_memory_cell *cell_U1DBUF;
  class cl_memory_cell *cell_U1BAUD;
  class cl_memory_cell *cell_U1UCR;
  class cl_memory_cell *cell_U1GCR;
  class cl_memory_cell *cell_P0DIR;
  class cl_memory_cell *cell_P1DIR;

public:
  cl_radio(class cl_uc *auc);
  virtual int init(void);

  // virtual void added_to_uc(void);

  //virtual t_mem read(class cl_memory_cell *cell);
  //virtual void write(class cl_memory_cell *cell, t_mem *val);
  //virtual t_mem read(class cl_memory_cell *cell);

  //virtual void mem_cell_changed(class cl_m *mem, t_addr addr);

  virtual int tick(int cycles);
  virtual void reset(void);
  virtual void happen(class cl_hw *where, enum hw_event he, void *params);

  virtual void print_info(class cl_console *con);
};

#endif

/* End of s51.src/interruptcl.h */
