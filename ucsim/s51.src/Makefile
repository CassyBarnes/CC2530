#
# uCsim s51.src/Makefile
#
# (c) Drotos Daniel, Talker Bt. 1997
#

STARTYEAR	= 1997

SHELL		= /bin/sh
CXX		= g++
CPP		= gcc -E
CXXCPP		= g++ -E
RANLIB		= ranlib
INSTALL		= /usr/bin/install -c
STRIP		= strip

PRJDIR		= ..

DEFS            = $(subs -DHAVE_CONFIG_H,,-DHAVE_CONFIG_H)
CPPFLAGS        =  -I. -I$(PRJDIR) \
		  -I$(PRJDIR)/cmd.src -I$(PRJDIR)/sim.src -I$(PRJDIR)/gui.src
CFLAGS          = -g -O2 -Wall
CXXFLAGS        = -g -O2 -gstabs+ -g3 -pipe -Wall
M_OR_MM         = -MM
PICOPT		= -fPIC
SHAREDLIB	= yes

SDCC		= sdcc
SDCFLAGS	= --debug --stack-after-data --model-small
SDCPPFLAGS	= 

LIBS		= -lnsl  -L$(PRJDIR) -Wl,--start-group -lutil -lsim -lcmd -lguiucsim -Wl,--end-group
DL		= -ldl
dl_ok		= yes

prefix          = /home/calypso/Desktop/UCSIM_BIN
exec_prefix     = ${prefix}
bindir          = ${exec_prefix}/bin
libdir          = ${exec_prefix}/lib
datadir         = ${prefix}/share
includedir      = ${prefix}/include
mandir          = ${prefix}/man
man1dir         = $(mandir)/man1
man2dir         = $(mandir)/man2
infodir         = ${prefix}/info
srcdir          = .

EXEEXT		= 

OBJECTS_SHARED	= glob.o sim51.o \
		  inc.o jmp.o mov.o logic.o arith.o bit.o \
		  timer0.o timer1.o timer2.o serial.o port.o interrupt.o \
		  wdt.o pca.o CC2530timer.o CC2530timer1.o CC2530timer3.o\
		  CC2530timer4.o uc51.o uc52.o uc51r.o uc89c51r.o uc251.o \
		  uc390.o uc390hw.o
OBJECTS_EXE	= s51.o 
OBJECTS		= $(OBJECTS_SHARED) $(OBJECTS_EXE)

enable_dlso	= no
dlso_ok		= no


# Compiling entire program or any subproject
# ------------------------------------------
all: checkconf otherlibs s51.src


# Compiling and installing everything and runing test
# ---------------------------------------------------
install: all installdirs
	$(INSTALL) -s s51$(EXEEXT) $(DESTDIR)$(bindir)


# Deleting all the installed files
# --------------------------------
uninstall:
	rm -f $(DESTDIR)$(bindir)/s51$(EXEEXT)


# Performing self-test
# --------------------
check: test

test: test_ser.ihx

test_ser.ihx: test_ser.rel
	$(SDCC) $(SDCFLAGS) $<

# Performing installation test
# ----------------------------
installcheck:


# Creating installation directories
# ---------------------------------
installdirs:
	test -d $(DESTDIR)$(bindir) || $(INSTALL) -d $(DESTDIR)$(bindir)


# Creating dependencies
# ---------------------
dep: Makefile.dep

Makefile.dep: *.cc *.h
	$(CXXCPP) $(CPPFLAGS) $(M_OR_MM) *.cc >Makefile.dep

include Makefile.dep
include clean.mk

#parser.cc: parser.y

#plex.cc: plex.l

# My rules
# --------
.SUFFIXES: .rel

s51.src: s51$(EXEEXT) shared_lib

s51$(EXEEXT): $(OBJECTS) $(PRJDIR)/*.a
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LIBS) -o $@

ifeq ($(dlso_ok),yes)
shared_lib: $(PRJDIR)/s51.so
else
shared_lib:
	@$(PRJDIR)/mkecho $(PRJDIR) "No 51 shared lib made."
	@$(PRJDIR)/mkecho $(PRJDIR) "(SHAREDLIB="$(SHAREDLIB)",dl_ok="$(dl_ok)",enable_dlso="$(enable_dlso)")"
endif

$(PRJDIR)/s51.so: $(OBJECTS_SHARED)
	$(CXX) -shared $(OBJECTS_SHARED) -o $@

otherlibs:
	cd $(PRJDIR) && $(MAKE) libs
	cd $(PRJDIR)/cmd.src && $(MAKE) all
	cd $(PRJDIR)/sim.src && $(MAKE) all
	$(MAKE) -C $(PRJDIR)/gui.src checkconf ucsim_lib

.cc.o:
	$(CXX) $(CXXFLAGS) $(PICOPT) $(CPPFLAGS) -c $< -o $@

.c.rel:
	$(SDCC) $(SDCFLAGS) $(SDCPPFLAGS) -c $<


# Remaking configuration
# ----------------------
checkconf:
	@if [ -f $(PRJDIR)/devel ]; then\
	  $(MAKE) -f conf.mk srcdir="$(srcdir)" PRJDIR="$(PRJDIR)" freshconf;\
	fi

# End of s51.src/Makefile.in
