#
# uCsim doc/Makefile
#
# (c) Drotos Daniel, Talker Bt. 1997
#

STARTYEAR	= 1997

SHELL		= /bin/sh
INSTALL		= /usr/bin/install -c

PRJDIR		= ..

prefix          = /usr/local
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
#docdir		= $(prefix)/share/doc/ucsim
docdir		= ${datadir}/sdcc/doc/ucsim


# Compiling entire program or any subproject
# ------------------------------------------
all: doc

doc: checkconf

# Compiling and installing everything and runing test
# ---------------------------------------------------
install: all installdirs
	cp *.html $(DESTDIR)$(docdir)
	cp *.gif $(DESTDIR)$(docdir)
	cp *.jpg $(DESTDIR)$(docdir)
	cp *.fig $(DESTDIR)$(docdir)


# Deleting all the installed files
# --------------------------------
uninstall:
	rm -rf $(DESTDIR)$(docdir)


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
	test -d $(DESTDIR)$(docdir) || $(INSTALL) -d $(DESTDIR)$(docdir)


# Creating dependencies
# ---------------------
include clean.mk

# My rules
# --------

# Remaking configuration
# ----------------------
checkconf:
	@if [ -f $(PRJDIR)/devel ]; then\
	  $(MAKE) -f conf.mk srcdir="$(srcdir)" PRJDIR="$(PRJDIR)" freshconf;\
	fi

# End of s51.src/Makefile.in
