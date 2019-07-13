# Copyright (c) 1988, The Regents of the University of California.
# Edward Moy, Workstation Software Support Group, Workstation Support Serices,
# Information Systems and Technology.
#
# Permission is granted to any individual or institution to use, copy,
# or redistribute this software so long as it is not sold for profit,
# provided that this notice and the original copyright notices are
# retained.  The University of California makes no representations about the
# suitability of this software for any purpose.  It is provided "as is"
# without express or implied warranty.
#
# SCCSid = "@(#)Makefile	2.2 10/24/89"
# RCSid = "$Header: /afs/sipb.mit.edu/project/sipbsrc/sun4/macps/RCS/Makefile,v 1.3 1993/10/01 00:11:02 ckclark Exp $"
#
# For System V, include -DSYSV in DEFINES below.
# To specify a fixed path for macps.config, include -DCONFIGDIR=\"path\"
# in CFLAGS below (path is the full pathname of the directory).
# To do save/restore context, include -DSAVE in CFLAGS below.
#
DESTDIR=
BINDIR= 	/mit/sipb/$(MACHINE)bin
MANSECT= 	1
MANDIR= 	/mit/sipb/man/man$(MANSECT)
INSTALL= 	install
CP= 		cp -f
CONFIG= 	/mit/sipb/lib/macps
DEFINES=	
CFLAGS= 	-O $(DEFINES) -DCONFIGDIR=\"$(CONFIG)\"
MACPS= 		macps.o macaux.o ucbwhich.o
PREPFIX= 	prepfix.o macaux.o
PREPFILES= 	LaserPrep5.1 LaserPrep5.2 LaserPrep6.0

all : macps prepfix

install: all
	$(INSTALL) -c -s macps $(DESTDIR)$(BINDIR)/macps
	$(INSTALL) -c -s prepfix $(DESTDIR)$(BINDIR)/prepfix
	$(CP) macps.1 $(DESTDIR)$(MANDIR)/macps.$(MANSECT)
	$(CP) prepfix.1 $(DESTDIR)$(MANDIR)/prepfix.$(MANSECT)
	-if [ ! -d ${DESTDIR}${CONFIG} ]; then \
		rm -f ${DESTDIR}${CONFIG}; \
		mkdir ${DESTDIR}${CONFIG}; fi
	$(CP) macps.config $(DESTDIR)$(CONFIG)/macps.config
	$(CP) $(PREPFILES) $(DESTDIR)$(CONFIG)

macps : $(MACPS)
	$(CC) -o macps $(CFLAGS) $(MACPS)

prepfix : $(PREPFIX)
	$(CC) -o prepfix $(CFLAGS) $(PREPFIX)

clean :
	/bin/rm -f *.o macps prepfix
