# @(#)sysdef.mk	1.2
########
#
#	sysdef makefile for ns32000
#
########
#
#	Standard Macros
#
########
CC = cc
CCLD1_CMD = $(CCLD_CMD) $(PPDEFS) $(CFLAGS) $(INC_LIST)
CCLD_CMD = $(CC) $(LDFLAGS)
CFLAGS = -O -s
LDFLAGS = 
PPDEFS =

INC_LIST	=

INS = /etc/install

all:
install:	all

PRODUCTS = sysdef

UINC = /usr/include

install:
	$(INS) -n /etc sysdef

sysdef:	sysdef.c
	$(CCLD1_CMD) -o sysdef sysdef.c $(LIB_LIST)

########
#
#	Standard Targets
#
#	all		builds all the products specified by PRODUCTS
#	clean		removes all temporary files (ex. installable object)
#	clobber		"cleans", and then removes $(PRODUCTS)
#
########

all:		$(PRODUCTS)

clean:

clobber:	clean
		rm -f $(PRODUCTS)
