//CC2530.cc 
#define APP_GO     1


 //Program main, executes an instruction at every System tick
int
main(int argc, char *argv[])
{
  int retval;
  class cl_app *app;

  fprintf(stderr, "Hi from Calypso in CC2530.cc!\n");
 
  cpus= cpus_51;
  application= new cl_app();
  application->init(argc, argv);
  application2= new cl_app();
  application2->init(argc, argv);

  sim= new cl_sim51(application);
  if (sim->init())
    return(1);

  while ((application->APP_GO) && (application2->APP_GO))
    {
     
      retval= application->step();
      retval= application2->step();
      clk++;
    }

  delete application;
  
  return(retval);
}

int
cl_app::step(void)
{
  if (going)
    {
      uc->do_inst(1);	
    }
  return(0);
}
