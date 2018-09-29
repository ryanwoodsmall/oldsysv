#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident "@(#)pump:pump.mk	1.5"
#
#		Copyright 1984 AT&T
#
########
#
#	Standard Macros
#
########
AUX_CLEAN = 
CCLD1_CMD = $(CCLD_CMD) $(PPDEFS) $(CFLAGS) $(INC_LIST)
CCLD_CMD = $(CC) $(LDFLAGS)
CFLAGS = -s -O
DEFS = 
LDFLAGS = 
MAKE.LO = make.lo
MAKE.ROOT = ../../../../make.root
MKGEN = mkgen
PPDEFS = $(DEFS)
SGS = $(SGSX)
SGSX = 

INC_LIST	=\
	-I$(UINC)

CLEAN =

all:
install:	all

##########
#
#	make.root
#
##########

AUX_MACHINES=x86
SGSM32=m32
SGSX86=x86
UINC=${ROOT}/usr/include

########
#
#	make.lo
#
########

LIB_LIST = -lld $(LDLIBS)
LD = $(SGS)ld

PRODUCTS = pump

all:	$(PRODUCTS)

install:	all
		install -n ${ROOT}/etc ${PRODUCTS}
		rm -f $(PRODUCTS)

########
#
#	All dependencies and rules not explicitly stated
#	(including header and nested header dependencies)
#
########

pump:	$(UINC)/fcntl.h
pump:	$(UINC)/filehdr.h
pump:	$(UINC)/ldfcn.h
pump:	pump.c
pump:	$(UINC)/stdio.h
pump:	$(UINC)/string.h
pump:	$(UINC)/sys/cio_defs.h
pump:	$(UINC)/sys/pump.h
pump:	$(UINC)/sys/stat.h
pump:	$(UINC)/sys/sysmacros.h
pump:	$(UINC)/sys/types.h
	$(CCLD1_CMD) -o pump pump.c $(LIB_LIST)

########
#
#	Standard Targets
#
#	all		builds all the products specified by PRODUCTS
#	clean		removes all temporary files (ex. installable object)
#	clobber		"cleans", and then removes $(PRODUCTS)
#	makefile	regenerates makefile
#	install		installs products; user defined in make.lo 
#
########

all:		$(PRODUCTS)

clean:
		rm -f $(CLEAN) $(AUX_CLEAN)

clobber:	clean
		rm -f $(PRODUCTS)

makefile:	$(MAKE.LO) $(MAKE.ROOT)
		$(MKGEN) >make.out
		if [ -s make.out ]; then mv make.out makefile; fi

makefile_all:	makefile

install: 	# rules, if any, specified above
