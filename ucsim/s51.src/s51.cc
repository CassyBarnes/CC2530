/*
 * Simulator of microcontrollers (s51.cc)
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

#include <string.h>

#include "ddconfig.h"

// prj
#include "globals.h"

// sim.src
#include "appcl.h"

// local
#include "sim51cl.h"


/*
 * Main function of the Simulator of MCS51. Everything starts here.
 */

int
main(int argc, char *argv[])
{
  int retval;
  //class cl_app *app;
  class cl_sim *sim;

  cpus= cpus_51;
  application= new cl_app();
  application->init(argc, argv);

  sim= new cl_sim51(application);
  if (sim->init())
    return(1);
  if (argc > 1)
    {
      if (strcmp(argv[1], "twoCC2530") == 0)
	{
	  class cl_sim *sim2;
	  class cl_app *application2;
	  unsigned long clk = 0;
	  fprintf(stderr, "Hi from Calypso in CC2530.cc!\n");
	  application2= new cl_app();
	  application2->init(argc, argv);
	  sim2= new cl_sim51(application2);
	  if (sim2->init())
	    return(1);
	  sim->state = SIM_GO;
	  sim->uc->Simulator = SIMULATOR_A;
	  sim2->state = SIM_GO;
	  sim2->uc->Simulator = SIMULATOR_B;
	  while ((sim->state & SIM_GO) && (sim2->state & SIM_GO))
	    {fprintf(stderr, " \n\n\n\n\n\nFirst instance of CC2530\n");
	      retval= sim->step(); fprintf(stderr, "\n\n\n\n\n\nSecond inst of CC2530\n");
	      retval= sim2->step();
	      clk++;
	    }
	  delete application2;
	}
      else
	{
	  fprintf(stderr, "Hi from Calypso in s51.cc!\n");
	  application->set_simulator(sim);
	  retval= /*sim->main()*/application->run();
	}
    }
  else
    {
      fprintf(stderr, "Hi from Calypso in s51.cc!\n");
      application->set_simulator(sim);
      retval= /*sim->main()*/application->run();
    }
  delete application;

  return(retval);
}

/* End of s51.src/s51.cc */
