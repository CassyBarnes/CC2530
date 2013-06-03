#include "CC2530timer3cl.h"
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


cl_CC2530_timer3::cl_CC2530_timer3(class cl_uc *auc, int aid, char *aid_string):
  cl_CC2530_timer(auc, aid, aid_string)
{
  TRACE();
  addr_tl  = T3CNT;
  addr_th  = ZERO;
  TxCC0L = T3CC0;
  TxCC0H = ZERO;//non existing address in sfr, returns 0x00 when read
  sfr= uc->address_space(MEM_SFR_ID);
  ChMax=2;
  init();
}


int
cl_CC2530_timer3::init(void)
{
  TRACE();
  //cl_CC2530_timer::init();
  assert(sfr);
  up_down=0;//0 => count up, 1=> count down
  tickspd=1;
  tickcount=0;
  prescale=1;
  //ticks=0;
  ticks=0;
  freq=CC2530xtal;

  register_cell(sfr, TIMIF, &cell_txstat, wtd_restore_write);
  register_cell(sfr, T3CTL, &cell_txctl, wtd_restore_write); 

  cell_tl = NULL;
  use_cell(sfr, addr_tl, &cell_tl, wtd_restore);
  assert(cell_tl);
  use_cell(sfr, addr_th, &cell_th, wtd_restore);

  tabCh[0]={0, 0, 0, 0, T3CC0, ZERO, T3CCTL0};
  tabCh[1]={0, 0, 0, 0, T3CC1, ZERO, T3CCTL1};

  return(0);
}

void
cl_CC2530_timer3::reset(void)
{
  cell_tl->write(0);
  ticks=0;
}

int
cl_CC2530_timer3::tick(int cycles)
{
  TRACE();
  cl_CC2530_timer::tick(cycles);
  TRACE();

  switch (mode)
    {
    case 0: cl_CC2530_timer::do_FreeRunningMode(cycles); break;
    case 1: cl_CC2530_timer::do_DownMode(cycles); break;
    case 2: cl_CC2530_timer::do_ModuloMode(cycles); break;
    case 3: cl_CC2530_timer::do_UpDownMode(cycles); break;
    }

  return(resGO);
}

/* End of s51.src/CC2530timer3.cc */
