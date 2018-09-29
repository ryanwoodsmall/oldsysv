# @(#)error.mk	1.3
########
#
#	error makefile for dec
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

PRODUCTS = errpt errdead errstop errdemon

UINC = /usr/include

errstop:
	cp errstop.sh errstop

install:
	$(INS) -n /usr/bin errpt
	$(INS) -n /etc errdead
	$(INS) -n /etc errstop
	$(INS) -n /usr/lib -o errdemon

errdead:	errdead.c
	$(CCLD1_CMD) -o errdead errdead.c $(LIB_LIST)

errdemon:	errdemon.c
	$(CCLD1_CMD) -o errdemon errdemon.c $(LIB_LIST)

errpt:	errpt.c
	$(CCLD1_CMD) -o errpt errpt.c $(LIB_LIST)

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
