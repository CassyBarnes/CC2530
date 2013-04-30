#
# ucsim cmd.src/Makefile
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

LEX		= :
YACC		= yacc
BISON_PLUS_PLUS	= :

PRJDIR		= ..

DEFS            = $(subs -DHAVE_CONFIG_H,,-DHAVE_CONFIG_H)
CPPFLAGS        =  -I. -I$(PRJDIR) \
		  -I$(PRJDIR)/sim.src -I$(PRJDIR)/gui.src
CFLAGS          = -g -O2 -Wall
CXXFLAGS        = -g -O2 -gstabs+ -g3 -pipe -Wall
M_OR_MM         = -MM

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

OBJECTS		= cmdset.o command.o newcmd.o cmdutil.o syntax.o \
		  get.o set.o timer.o bp.o info.o show.o cmdgui.o cmdconf.o \
		  cmduc.o cmdstat.o cmdmem.o \
		  cmdpars.o cmdlex.o

DEVEL		= $(shell ( bash -c "test -e $(PRJDIR)/devel && echo yes" ))

# Compiling entire program or any subproject
# ------------------------------------------
all: checkconf cmdlib

cmd.src: all


# Compiling and installing everything and runing test
# ---------------------------------------------------
install: all installdirs


# Deleting all the installed files
# --------------------------------
uninstall:


# Performing self-test
# --------------------
check:

test:


# Performing installation test
# ----------------------------
installcheck:


# Creating installation directories
# ---------------------------------
installdirs:


# Creating dependencies
# ---------------------
dep: Makefile.dep

Makefile.dep: *.cc *.h
	$(CXXCPP) $(CPPFLAGS) $(M_OR_MM) *.cc >Makefile.dep

include Makefile.dep
include clean.mk

cmdpars.cc: cmdpars.y

ifeq ($(DEVEL),yes)
  cmdlex.cc: cmdlex.l cmdpars.h
endif

# My rules
# --------

cmdlib: $(PRJDIR)/libcmd.a

$(PRJDIR)/libcmd.a: $(OBJECTS)
	ar -rcu $*.a $(OBJECTS)
	$(RANLIB) $*.a

.cc.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c $< -o $@

#.y.cc:
#	rm -f $*.cc $*.h
#	$(YACC) -d $<
#	mv y.tab.c $*.cc
#	mv y.tab.h $*.h

ifeq ($(DEVEL),yes)
.y.cc:
	rm -f $*.cc $*.h
	$(BISON_PLUS_PLUS) -d -o $*.cc $<

.l.cc:
	rm -f $*.cc
	$(LEX) --c++ -o $*.cc $<
endif


# Remaking configuration
# ----------------------
checkconf:
	@if [ -f $(PRJDIR)/devel ]; then\
	  $(MAKE) -f conf.mk srcdir="$(srcdir)" PRJDIR="$(PRJDIR)" freshconf;\
	fi

# End of cmd.src/Makefile.in
