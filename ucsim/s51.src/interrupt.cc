//#define DBG_RADIO
/*
 * Simulator of microcontrollers (interrupt.cc)
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
#include <assert.h>
// prj
#include "utils.h"

// sim
#include "itsrccl.h"

// local
#include "interruptcl.h"
#include "regs51.h"
//#include "uc51cl.h"
#include "types51.h"


cl_interrupt::cl_interrupt(class cl_uc *auc):
  cl_hw(auc, HW_INTERRUPT, 0, "irq")
{
  was_reti= DD_FALSE;
}

int
cl_interrupt::init(void)
{
  sfr= uc->address_space(MEM_SFR_ID);
  if (sfr)
    {
      //sfr->register_hw(IE, this, 0);
      register_cell(sfr, IE, 0, wtd_restore);
      register_cell(sfr, TCON, &cell_tcon, wtd_restore_write);
      bit_INT0= sfr->read(P3) & bm_INT0;
      bit_INT1= sfr->read(P3) & bm_INT1;
    }
  return(0);
}

void
cl_interrupt::added_to_uc(void)
{
  uc->it_sources->add(new cl_it_src(bmEX0, TCON, bmIE0, 0x0003, true,
				    "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, TCON, bmIE1, 0x0013, true,
				    "external #1", 3));
}

void
cl_interrupt::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_tcon)
    {
      bit_IT0= *val & bmIT0;
      bit_IT1= *val & bmIT1;
    }
  else
    // IE register
    was_reti= DD_TRUE;
}

/*void
cl_interrupt::mem_cell_changed(class cl_m *mem, t_addr addr)
{
}*/

int
cl_interrupt::tick(int cycles)
{
  if (!bit_IT0 && !bit_INT0)
    cell_tcon->set_bit1(bmIE0);
  if (!bit_IT1 && !bit_INT1)
    cell_tcon->set_bit1(bmIE1);
  return(resGO);
}

void
cl_interrupt::reset(void)
{
  was_reti= DD_FALSE;
}

void
cl_interrupt::happen(class cl_hw *where, enum hw_event he, void *params)
{
  struct ev_port_changed *ep= (struct ev_port_changed *)params;

  if (where->cathegory == HW_PORT &&
      he == EV_PORT_CHANGED &&
      ep->id == 3)
    {
      t_mem p3n= ep->new_pins & ep->new_value;
      t_mem p3o= ep->pins & ep->prev_value;
      if (bit_IT0 &&
	  !(p3n & bm_INT0) &&
	  (p3o & bm_INT0))
	cell_tcon->set_bit1(bmIE0);
      if (bit_IT1 &&
	  !(p3n & bm_INT1) &&
	  (p3o & bm_INT1))
	cell_tcon->set_bit1(bmIE1);
      bit_INT0= p3n & bm_INT0;
      bit_INT1= p3n & bm_INT1;
    }
}


void
cl_interrupt::print_info(class cl_console *con)
{
  int ie= sfr->get(IE);
  int i;

  con->dd_printf("Interrupts are %s. Interrupt sources:\n",
		 (ie&bmEA)?"enabled":"disabled");
  con->dd_printf("  Handler  En  Pr Req Act Name\n");
  for (i= 0; i < uc->it_sources->count; i++)
    {
      class cl_it_src *is= (class cl_it_src *)(uc->it_sources->at(i));
      con->dd_printf("  0x%06x", is->addr);
      con->dd_printf(" %-3s", (ie&(is->ie_mask))?"en":"dis");
      con->dd_printf(" %2d", uc->it_priority(is->ie_mask));
      con->dd_printf(" %-3s",
		     (sfr->get(is->src_reg)&(is->src_mask))?
		     "YES":"no");
      con->dd_printf(" %-3s", (is->active)?"act":"no");
      con->dd_printf(" %s", object_name(is));
      con->dd_printf("\n");
    }
  con->dd_printf("Active interrupt service(s):\n");
  con->dd_printf("  Pr Handler  PC       Source\n");
  for (i= 0; i < uc->it_levels->count; i++)
    {
      class it_level *il= (class it_level *)(uc->it_levels->at(i));
      if (il->level >= 0)
	{
	  con->dd_printf("  %2d", il->level);
	  con->dd_printf(" 0x%06x", il->addr);
	  con->dd_printf(" 0x%06x", il->PC);
	  con->dd_printf(" %s", (il->source)?(object_name(il->source)):
			 "nothing");
	  con->dd_printf("\n");
	}
    }
}




// Radio (CC2330 specific)
cl_radio::cl_radio(class cl_uc *auc):
  cl_hw(auc, HW_CC2530_RADIO, 0, "radio")
{
#ifdef DBG_RADIO
  fprintf(stderr, "%s:%d CC2530 radio %s\n", __FILE__, __LINE__, __FUNCTION__);
#endif
}

int
cl_radio::init(void)
{
#ifdef DBG_RADIO
  fprintf(stderr, "%s:%d CC2530 radio %s\n", __FILE__, __LINE__, __FUNCTION__);
#endif

  sfr= uc->address_space(MEM_SFR_ID);
  if (sfr)
    {
#ifdef DBG_RADIO
      fprintf(stderr, "%s:%d CC2530 radio %s: declaration of CC2530 registers to ucsim machinery.\n", __FILE__, __LINE__, __FUNCTION__);
#endif
      register_cell(sfr, RFIRQF1, &cell_RFIRQF1, wtd_restore_write);
      register_cell(sfr, U0CSR, &cell_U0CSR, wtd_restore_write);
      register_cell(sfr, P0IFG, &cell_P0IFG, wtd_restore_write);
      register_cell(sfr, P1IFG, &cell_P1IFG, wtd_restore_write);
      register_cell(sfr, P2IFG, &cell_P2IFG, wtd_restore_write);
      register_cell(sfr, PICTL, &cell_PICTL, wtd_restore_write);
      register_cell(sfr, P1IEN, &cell_P1IEN, wtd_restore_write);
      register_cell(sfr, P0INP, &cell_P0INP, wtd_restore_write);
      register_cell(sfr, DPS, &cell_DPS, wtd_restore_write);
      register_cell(sfr, MPAGE, &cell_MPAGE, wtd_restore_write);
      register_cell(sfr, T2CTRL, &cell_T2CTRL, wtd_restore_write);
      register_cell(sfr, ST0, &cell_ST0, wtd_restore_write);
      register_cell(sfr, ST1, &cell_ST1, wtd_restore_write);
      register_cell(sfr, ST2, &cell_ST2, wtd_restore_write);
      register_cell(sfr, IEN2, &cell_IEN2, wtd_restore_write);
      register_cell(sfr, S1CON, &cell_S1CON, wtd_restore_write);
      register_cell(sfr, T2EVTCFG, &cell_T2EVTCFG, wtd_restore_write);
      register_cell(sfr, SLEEPSTA, &cell_SLEEPSTA, wtd_restore_write);
      register_cell(sfr, CLKCONSTA, &cell_CLKCONSTA, wtd_restore_write);
      register_cell(sfr, FMAP, &cell_FMAP, wtd_restore_write);
      register_cell(sfr, T2IRQF, &cell_T2IRQF, wtd_restore_write);
      register_cell(sfr, T2M0, &cell_T2M0, wtd_restore_write);
      register_cell(sfr, T2M1, &cell_T2M1, wtd_restore_write);
      register_cell(sfr, T2MOVF0, &cell_T2MOVF0, wtd_restore_write);
      register_cell(sfr, T2MOVF1, &cell_T2MOVF1, wtd_restore_write);
      register_cell(sfr, T2MOVF2, &cell_T2MOVF2, wtd_restore_write);
      register_cell(sfr, T2IRQM, &cell_T2IRQM, wtd_restore_write);
      register_cell(sfr, IEN1, &cell_IEN1, wtd_restore_write);
      register_cell(sfr, P0IEN, &cell_P0IEN, wtd_restore_write);
      register_cell(sfr, P2IEN, &cell_P2IEN, wtd_restore_write);
      register_cell(sfr, STLOAD, &cell_STLOAD, wtd_restore_write);
      register_cell(sfr, PMUX, &cell_PMUX, wtd_restore_write);
      register_cell(sfr, T1STAT, &cell_T1STAT, wtd_restore_write);
      register_cell(sfr, ENCDI, &cell_ENCDI, wtd_restore_write);
      register_cell(sfr, ENCDO, &cell_ENCDO, wtd_restore_write);
      register_cell(sfr, ENCCS, &cell_ENCCS, wtd_restore_write);
      register_cell(sfr, ADCCON1, &cell_ADCCON1, wtd_restore_write);
      register_cell(sfr, ADCCON2, &cell_ADCCON2, wtd_restore_write);
      register_cell(sfr, ADCCON3, &cell_ADCCON3, wtd_restore_write);
      register_cell(sfr, IP1, &cell_IP1, wtd_restore_write);
      register_cell(sfr, ADCL, &cell_ADCL, wtd_restore_write);
      register_cell(sfr, ADCH, &cell_ADCH, wtd_restore_write);
      register_cell(sfr, RNDL, &cell_RNDL, wtd_restore_write);
      register_cell(sfr, RNDH, &cell_RNDH, wtd_restore_write);
      register_cell(sfr, SLEEPCMD, &cell_SLEEPCMD, wtd_restore_write);
      register_cell(sfr, RFERRF, &cell_RFERRF, wtd_restore_write);
      register_cell(sfr, IRCON, &cell_IRCON, wtd_restore_write);
      register_cell(sfr, U0DBUF, &cell_U0DBUF, wtd_restore_write);
      register_cell(sfr, U0BAUD, &cell_U0BAUD, wtd_restore_write);
      register_cell(sfr, T2MSEL, &cell_T2MSEL, wtd_restore_write);
      register_cell(sfr, U0UCR, &cell_U0UCR, wtd_restore_write);
      register_cell(sfr, U0GCR, &cell_U0GCR, wtd_restore_write);
      register_cell(sfr, CLKCONCMD, &cell_CLKCONCMD, wtd_restore_write);
      register_cell(sfr, WDCTL, &cell_WDCTL, wtd_restore_write);
      register_cell(sfr, T3CNT, &cell_T3CNT, wtd_restore_write);
      register_cell(sfr, T3CTL, &cell_T3CTL, wtd_restore_write);
      register_cell(sfr, T3CCTL0, &cell_T3CCTL0, wtd_restore_write);
      register_cell(sfr, T3CC0, &cell_T3CC0, wtd_restore_write);
      register_cell(sfr, T3CCTL1, &cell_T3CCTL1, wtd_restore_write);
      register_cell(sfr, T3CC1, &cell_T3CC1, wtd_restore_write);
      register_cell(sfr, DMAIRQ, &cell_DMAIRQ, wtd_restore_write);
      register_cell(sfr, DMA1CFGL, &cell_DMA1CFGL, wtd_restore_write);
      register_cell(sfr, DMA1CFGH, &cell_DMA1CFGH, wtd_restore_write);
      register_cell(sfr, DMA0CFGL, &cell_DMA0CFGL, wtd_restore_write);
      register_cell(sfr, DMA0CFGH, &cell_DMA0CFGH, wtd_restore_write);
      register_cell(sfr, DMAARM, &cell_DMAARM, wtd_restore_write);
      register_cell(sfr, DMAREQ, &cell_DMAREQ, wtd_restore_write);
      register_cell(sfr, TIMIF, &cell_TIMIF, wtd_restore_write);
      register_cell(sfr, RFD, &cell_RFD, wtd_restore_write);
      register_cell(sfr, T1CC0L, &cell_T1CC0L, wtd_restore_write);
      register_cell(sfr, T1CC0H, &cell_T1CC0H, wtd_restore_write);
      register_cell(sfr, T1CC1L, &cell_T1CC1L, wtd_restore_write);
      register_cell(sfr, T1CC1H, &cell_T1CC1H, wtd_restore_write);
      register_cell(sfr, T1CC2L, &cell_T1CC2L, wtd_restore_write);
      register_cell(sfr, T1CC2H, &cell_T1CC2H, wtd_restore_write);
      register_cell(sfr, RFST, &cell_RFST, wtd_restore_write);
      register_cell(sfr, T1CNTL, &cell_T1CNTL, wtd_restore_write);
      register_cell(sfr, T1CNTH, &cell_T1CNTH, wtd_restore_write);
      register_cell(sfr, T1CTL, &cell_T1CTL, wtd_restore_write);
      register_cell(sfr, T1CCTL0, &cell_T1CCTL0, wtd_restore_write);
      register_cell(sfr, T1CCTL1, &cell_T1CCTL1, wtd_restore_write);
      register_cell(sfr, T1CCTL2, &cell_T1CCTL2, wtd_restore_write);
      register_cell(sfr, IRCON2, &cell_IRCON2, wtd_restore_write);
      register_cell(sfr, RFIRQF0, &cell_RFIRQF0, wtd_restore_write);
      register_cell(sfr, T4CNT, &cell_T4CNT, wtd_restore_write);
      register_cell(sfr, T4CTL, &cell_T4CTL, wtd_restore_write);
      register_cell(sfr, T4CCTL0, &cell_T4CCTL0, wtd_restore_write);
      register_cell(sfr, T4CC0, &cell_T4CC0, wtd_restore_write);
      register_cell(sfr, T4CCTL1, &cell_T4CCTL1, wtd_restore_write);
      register_cell(sfr, T4CC1, &cell_T4CC1, wtd_restore_write);
      register_cell(sfr, PERCFG, &cell_PERCFG, wtd_restore_write);
      register_cell(sfr, APCFG, &cell_APCFG, wtd_restore_write);
      register_cell(sfr, P0SEL, &cell_P0SEL, wtd_restore_write);
      register_cell(sfr, P1SEL, &cell_P1SEL, wtd_restore_write);
      register_cell(sfr, P2SEL, &cell_P2SEL, wtd_restore_write);
      register_cell(sfr, P1INP, &cell_P1INP, wtd_restore_write);
      register_cell(sfr, P2INP, &cell_P2INP, wtd_restore_write);
      register_cell(sfr, U1CSR, &cell_U1CSR, wtd_restore_write);
      register_cell(sfr, U1DBUF, &cell_U1DBUF, wtd_restore_write);
      register_cell(sfr, U1BAUD, &cell_U1BAUD, wtd_restore_write);
      register_cell(sfr, U1UCR, &cell_U1UCR, wtd_restore_write);
      register_cell(sfr, U1GCR, &cell_U1GCR, wtd_restore_write);
      register_cell(sfr, P0DIR, &cell_P0DIR, wtd_restore_write);
      register_cell(sfr, P1DIR, &cell_P1DIR, wtd_restore_write);

    }
  return(0);
}

#ifdef OOO
void
cl_radio::added_to_uc(void)
{
  uc->it_sources->add(new cl_it_src(bmEX0, RFIRQF1, bmIE0, 0x0003, true,
				    "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, RFIRQF1, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, U0CSR, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, U0CSR, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, P0IFG, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P0IFG, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, P1IFG, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P1IFG, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, P2IFG, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P2IFG, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, PICTL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, PICTL, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, P1IEN, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P1IEN, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, P0INP, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P0INP, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, DPS, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, DPS, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, MPAGE, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, MPAGE, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, T2CTRL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T2CTRL, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, ST0, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, ST0, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, ST1, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, ST1, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, ST2, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, ST2, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, IEN2, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, IEN2, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, S1CON, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, S1CON, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, T2EVTCFG, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T2EVTCFG, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, SLEEPSTA, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, SLEEPSTA, bmIE1, 0x0013, true, "external #1", 3));

  uc->it_sources->add(new cl_it_src(bmEX0, CLKCONSTA, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, CLKCONSTA, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, FMAP, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, FMAP, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T2IRQF, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T2IRQF, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T2M0, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T2M0, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T2M1, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T2M1, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T2MOVF0, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T2MOVF0, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T2MOVF1, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T2MOVF1, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T2MOVF2, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T2MOVF2, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T2IRQM, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T2IRQM, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, IEN1, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, IEN1, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, P0IEN, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P0IEN, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, P2IEN, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P2IEN, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, STLOAD, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, STLOAD, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, PMUX, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, PMUX, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T1STAT, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T1STAT, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, ENCDI, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, ENCDI, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, ENCDO, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, ENCDO, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, ENCCS, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, ENCCS, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, ADCCON1, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, ADCCON1, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, ADCCON2, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, ADCCON2, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, ADCCON3, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, ADCCON3, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, IP1, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, IP1, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, ADCL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, ADCL, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, ADCH, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, ADCH, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, RNDL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, RNDL, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, RNDH, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, RNDH, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, SLEEPCMD, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, SLEEPCMD, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, RFERRF, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, RFERRF, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, IRCON, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, IRCON, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, U0DBUF, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, U0DBUF, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, U0BAUD, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, U0BAUD, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T2MSEL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T2MSEL, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, U0UCR, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, U0UCR, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, U0GCR, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, U0GCR, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, CLKCONCMD, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, CLKCONCMD, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, WDCTL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, WDCTL, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T3CNT, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T3CNT, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T3CTL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T3CTL, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T3CCTL0, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T3CCTL0, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T3CC0, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T3CC0, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T3CCTL1, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T3CCTL1, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T3CC1, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T3CC1, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, DMAIRQ, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, DMAIRQ, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, DMA1CFGL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, DMA1CFGL, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, DMA1CFGH, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, DMA1CFGH, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, DMA0CFGL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, DMA0CFGL, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, DMA0CFGH, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, DMA0CFGH, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, DMAARM, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, DMAARM, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, DMAREQ, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, DMAREQ, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, TIMIF, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, TIMIF, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, RFD, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, RFD, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T1CC0L, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T1CC0L, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T1CC0H, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T1CC0H, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T1CC1L, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T1CC1L, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T1CC1H, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T1CC1H, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T1CC2L, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T1CC2L, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T1CC2H, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T1CC2H, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, RFST, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, RFST, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T1CNTL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T1CNTL, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T1CNTH, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T1CNTH, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T1CTL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T1CTL, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T1CCTL0, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T1CCTL0, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T1CCTL1, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T1CCTL1, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T1CCTL2, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T1CCTL2, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, IRCON2, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, IRCON2, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, RFIRQF0, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, RFIRQF0, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T4CNT, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T4CNT, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T4CTL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T4CTL, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T4CCTL0, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T4CCTL0, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T4CC0, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T4CC0, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T4CCTL1, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T4CCTL1, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, T4CC1, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, T4CC1, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, PERCFG, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, PERCFG, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, APCFG, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, APCFG, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, P0SEL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P0SEL, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, P1SEL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P1SEL, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, P2SEL, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P2SEL, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, P1INP, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P1INP, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, P2INP, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P2INP, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, U1CSR, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, U1CSR, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, U1DBUF, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, U1DBUF, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, U1BAUD, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, U1BAUD, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, U1UCR, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, U1UCR, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, U1GCR, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, U1GCR, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, P0DIR, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P0DIR, bmIE1, 0x0013, true, "external #1", 3));
  uc->it_sources->add(new cl_it_src(bmEX0, P1DIR, bmIE0, 0x0003, true, "external #0", 1));
  uc->it_sources->add(new cl_it_src(bmEX1, P1DIR, bmIE1, 0x0013, true, "external #1", 3));

}

void
cl_radio::write(class cl_memory_cell *cell, t_mem *val)
{
#ifdef DBG_RADIO
  if (cell == cell_RFIRQF1)
    {
      fprintf(stderr, "%s:%d CC2530 radio %s: write to RFIRQF1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U0CSR){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to U0CSR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P0IFG){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P0IFG\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P1IFG){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P1IFG\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P2IFG){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P2IFG\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_PICTL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to PICTL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P1IEN){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P1IEN\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P0INP){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P0INP\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DPS){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to DPS\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_MPAGE){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to MPAGE\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2CTRL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T2CTRL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ST0){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to ST0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ST1){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to ST1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ST2){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to ST2\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_IEN2){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to IEN2\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_S1CON){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to S1CON\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2EVTCFG){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T2EVTCFG\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_SLEEPSTA){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to SLEEPSTA\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_CLKCONSTA){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to CLKCONSTA\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_FMAP){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to FMAP\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2IRQF){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T2IRQF\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2M0){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T2M0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2M1){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T2M1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2MOVF0){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T2MOVF0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2MOVF1){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T2MOVF1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2MOVF2){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T2MOVF2\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2IRQM){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T2IRQM\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_IEN1){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to IEN1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P0IEN){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P0IEN\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P2IEN){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P2IEN\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_STLOAD){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to STLOAD\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_PMUX){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to PMUX\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1STAT){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T1STAT\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ENCDI){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to ENCDI\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ENCDO){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to ENCDO\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ENCCS){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to ENCCS\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ADCCON1){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to ADCCON1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ADCCON2){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to ADCCON2\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ADCCON3){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to ADCCON3\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_IP1){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to IP1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ADCL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to ADCL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ADCH){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to ADCH\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_RNDL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to RNDL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_RNDH){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to RNDH\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_SLEEPCMD){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to SLEEPCMD\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_RFERRF){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to RFERRF\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_IRCON){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to IRCON\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U0DBUF){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to U0DBUF\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U0BAUD){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to U0BAUD\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2MSEL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T2MSEL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U0UCR){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to U0UCR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U0GCR){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to U0GCR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_CLKCONCMD){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to CLKCONCMD\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_WDCTL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to WDCTL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T3CNT){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T3CNT\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T3CTL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T3CTL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T3CCTL0){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T3CCTL0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T3CC0){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T3CC0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T3CCTL1){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T3CCTL1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T3CC1){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T3CC1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DMAIRQ){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to DMAIRQ\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DMA1CFGL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to DMA1CFGL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DMA1CFGH){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to DMA1CFGH\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DMA0CFGL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to DMA0CFGL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DMA0CFGH){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to DMA0CFGH\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DMAARM){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to DMAARM\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DMAREQ){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to DMAREQ\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_TIMIF){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to TIMIF\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_RFD){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to RFD\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CC0L){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T1CC0L\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CC0H){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T1CC0H\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CC1L){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T1CC1L\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CC1H){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T1CC1H\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CC2L){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T1CC2L\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CC2H){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T1CC2H\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_RFST){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to RFST\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CNTL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T1CNTL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CNTH){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T1CNTH\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CTL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T1CTL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CCTL0){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T1CCTL0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CCTL1){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T1CCTL1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CCTL2){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T1CCTL2\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_IRCON2){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to IRCON2\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_RFIRQF0){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to RFIRQF0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T4CNT){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T4CNT\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T4CTL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T4CTL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T4CCTL0){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T4CCTL0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T4CC0){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T4CC0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T4CCTL1){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T4CCTL1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T4CC1){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to T4CC1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_PERCFG){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to PERCFG\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_APCFG){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to APCFG\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P0SEL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P0SEL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P1SEL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P1SEL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P2SEL){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P2SEL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P1INP){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P1INP\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P2INP){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P2INP\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U1CSR){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to U1CSR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U1DBUF){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to U1DBUF\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U1BAUD){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to U1BAUD\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U1UCR){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to U1UCR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U1GCR){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to U1GCR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P0DIR){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P0DIR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P1DIR){
    fprintf(stderr, "%s:%d CC2530 radio %s: write to P1DIR\n", __FILE__, __LINE__, __FUNCTION__);}
  else {
    fprintf(stderr, "%s:%d CC2530 radio %s: write to unknown register\n", __FILE__, __LINE__, __FUNCTION__);
  }
#endif
}







t_mem
cl_radio::read(class cl_memory_cell *cell)
{
#ifdef DBG_RADIO
  if (cell == cell_RFIRQF1)
    {
      fprintf(stderr, "%s:%d CC2530 radio %s: read of RFIRQF1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U0CSR){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of U0CSR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P0IFG){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of P0IFG\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P1IFG){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of P1IFG\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P2IFG){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of P2IFG\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_PICTL){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of PICTL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P1IEN){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of P1IEN\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P0INP){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of P0INP\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DPS){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of DPS\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_MPAGE){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of MPAGE\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2CTRL){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T2CTRL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ST0){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of ST0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ST1){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of ST1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ST2){
    fprintf(stderr,  "%s:%d CC2530 radio %s: read of ST2\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_IEN2){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of IEN2\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_S1CON){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of S1CON\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2EVTCFG){
    fprintf(stderr,  "%s:%d CC2530 radio %s: read of T2EVTCFG\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_SLEEPSTA){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of SLEEPSTA\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_CLKCONSTA){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of CLKCONSTA\n", __FILE__, __LINE__, __FUNCTION__);}
  /*  else if (cell == cell_FMAP){
      fprintf(stderr,  "%s:%d CC2530 radio %s: read of FMAP\n", __FILE__, __LINE__, __FUNCTION__);}*/
  else if (cell == cell_T2IRQF){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T2IRQF\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2M0){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T2M0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2M1){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of T2M1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2MOVF0){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T2MOVF0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2MOVF1){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T2MOVF1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2MOVF2){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of T2MOVF2\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2IRQM){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T2IRQM\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_IEN1){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of IEN1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P0IEN){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of P0IEN\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P2IEN){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of P2IEN\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_STLOAD){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of STLOAD\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_PMUX){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of PMUX\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1STAT){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T1STAT\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ENCDI){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of ENCDI\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ENCDO){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of ENCDO\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ENCCS){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of ENCCS\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ADCCON1){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of ADCCON1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ADCCON2){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of ADCCON2\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ADCCON3){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of ADCCON3\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_IP1){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of IP1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ADCL){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of ADCL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_ADCH){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of ADCH\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_RNDL){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of RNDL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_RNDH){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of RNDH\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_SLEEPCMD){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of SLEEPCMD\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_RFERRF){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of RFERRF\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_IRCON){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of IRCON\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U0DBUF){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of U0DBUF\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U0BAUD){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of U0BAUD\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T2MSEL){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of T2MSEL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U0UCR){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of U0UCR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U0GCR){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of U0GCR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_CLKCONCMD){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of CLKCONCMD\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_WDCTL){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of WDCTL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T3CNT){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T3CNT\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T3CTL){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of T3CTL\n", __FILE__, __LINE__, __FUNCTION__);}
  /*  else if (cell == cell_T3CCTL0){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T3CCTL0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T3CC0){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T3CC0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T3CCTL1){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of T3CCTL1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T3CC1){
  fprintf(stderr, "%s:%d CC2530 radio %s: read of T3CC1\n", __FILE__, __LINE__, __FUNCTION__);}*/
  else if (cell == cell_DMAIRQ){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of DMAIRQ\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DMA1CFGL){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of DMA1CFGL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DMA1CFGH){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of DMA1CFGH\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DMA0CFGL){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of DMA0CFGL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DMA0CFGH){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of DMA0CFGH\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DMAARM){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of DMAARM\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_DMAREQ){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of DMAREQ\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_TIMIF){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of TIMIF\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_RFD){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of RFD\n", __FILE__, __LINE__, __FUNCTION__);}
  /*  else if (cell == cell_T1CC0L){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T1CC0L\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CC0H){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of T1CC0H\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CC1L){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T1CC1L\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CC1H){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T1CC1H\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CC2L){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of T1CC2L\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CC2H){
  fprintf(stderr, "%s:%d CC2530 radio %s: read of T1CC2H\n", __FILE__, __LINE__, __FUNCTION__);}*/
  else if (cell == cell_RFST){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of RFST\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CNTL){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of T1CNTL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CNTH){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T1CNTH\n", __FILE__, __LINE__, __FUNCTION__);}
  /*  else if (cell == cell_T1CTL){
      fprintf(stderr, "%s:%d CC2530 radio %s: read of T1CTL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CCTL0){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of T1CCTL0\n", __FILE__, __LINE__, __FUNCTION__);}
    else if (cell == cell_T1CCTL1){
      fprintf(stderr, "%s:%d CC2530 radio %s: read of T1CCTL1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T1CCTL2){
  fprintf(stderr, "%s:%d CC2530 radio %s: read of T1CCTL2\n", __FILE__, __LINE__, __FUNCTION__);}*/
  else if (cell == cell_IRCON2){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of IRCON2\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_RFIRQF0){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of RFIRQF0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T4CNT){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T4CNT\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T4CTL){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of T4CTL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T4CCTL0){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T4CCTL0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T4CC0){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T4CC0\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T4CCTL1){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of T4CCTL1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_T4CC1){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of T4CC1\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_PERCFG){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of PERCFG\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_APCFG){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of APCFG\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P0SEL){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of P0SEL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P1SEL){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of P1SEL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P2SEL){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of P2SEL\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P1INP){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of P1INP\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P2INP){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of P2INP\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U1CSR){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of U1CSR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U1DBUF){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of U1DBUF\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U1BAUD){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of U1BAUD\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U1UCR){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of U1UCR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_U1GCR){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of U1GCR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P0DIR){
    fprintf(stderr, "%s:%d CC2530 radio %s: read of P0DIR\n", __FILE__, __LINE__, __FUNCTION__);}
  else if (cell == cell_P1DIR){
    fprintf(stderr, "%s:%d  CC2530 radio %s: read of P1DIR\n", __FILE__, __LINE__, __FUNCTION__);}
  /*  else {
      fprintf(stderr, "%s:%d CC2530 radio %s: read of unknown register\n", __FILE__, __LINE__, __FUNCTION__);
      }*/
#endif
  return (cell->get());
}

#endif


/*void
cl_radio::mem_cell_changed(class cl_m *mem, t_addr addr)
{
}*/

int
cl_radio::tick(int cycles)
{
  return(resGO);
}

void
cl_radio::reset(void)
{
 
}

void
cl_radio::happen(class cl_hw *where, enum hw_event he, void *params)
{
  struct ev_port_changed *ep= (struct ev_port_changed *)params;

  if (where->cathegory == HW_PORT &&
      he == EV_PORT_CHANGED &&
      ep->id == 3)
    {
      t_mem p3n= ep->new_pins & ep->new_value;
      t_mem p3o= ep->pins & ep->prev_value;

    }
}


void
cl_radio::print_info(class cl_console *con)
{
  int ie= sfr->get(IE);
  int i;

  con->dd_printf("Radios are %s. Radio sources:\n",
		 (ie&bmEA)?"enabled":"disabled");
  con->dd_printf("  Handler  En  Pr Req Act Name\n");
  for (i= 0; i < uc->it_sources->count; i++)
    {
      class cl_it_src *is= (class cl_it_src *)(uc->it_sources->at(i));
      con->dd_printf("  0x%06x", is->addr);
      con->dd_printf(" %-3s", (ie&(is->ie_mask))?"en":"dis");
      con->dd_printf(" %2d", uc->it_priority(is->ie_mask));
      con->dd_printf(" %-3s",
		     (sfr->get(is->src_reg)&(is->src_mask))?
		     "YES":"no");
      con->dd_printf(" %-3s", (is->active)?"act":"no");
      con->dd_printf(" %s", object_name(is));
      con->dd_printf("\n");
    }
  con->dd_printf("Active radio service(s):\n");
  con->dd_printf("  Pr Handler  PC       Source\n");
  for (i= 0; i < uc->it_levels->count; i++)
    {
      class it_level *il= (class it_level *)(uc->it_levels->at(i));
      if (il->level >= 0)
	{
	  con->dd_printf("  %2d", il->level);
	  con->dd_printf(" 0x%06x", il->addr);
	  con->dd_printf(" 0x%06x", il->PC);
	  con->dd_printf(" %s", (il->source)?(object_name(il->source)):
			 "nothing");
	  con->dd_printf("\n");
	}
    }
}


/* End of s51.src/interrupt.cc */
