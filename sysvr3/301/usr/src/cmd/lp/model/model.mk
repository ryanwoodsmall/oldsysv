#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident "@(#)lp:model/model.mk	1.11"
#	model make file

ROOT =
OL	= $(ROOT)/
SL	= $(ROOT)/usr/src/cmd
RDIR	= $(SL)/lp/model
INS	= :
REL	= current
OWN	= bin
GRP	= bin
LIST	= lp
LP      = $(OL)usr/spool/lp
BIN	= $(OL)usr/spool/lp/model
SOURCE	= 1640 dqp10 dumb f450 hp lqp40 prx pprx ph.daps 5310
MAKE	= make

compile all:	mklp  $(SOURCE)
	$(INS) if [ ! -d $(BIN) ]; then mkdir $(BIN); fi  
	$(INS) for i in $(SOURCE); \
	  $(INS) do cp $$i $(BIN); \
	  $(INS) chmod 775 $(BIN)/$$i; \
	  $(INS) if [ "$(OL)" = "/" ]; \
	     $(INS) then (cd $(BIN); $(INS) chgrp $(GRP) $$i; $(INS) chown $(OWN) $$i;) \
	  $(INS) fi; \
	$(INS) done

mklp:
	$(INS) if [ ! -d $(LP) ];then mkdir $(LP); fi


install:
	$(MAKE) -f model.mk INS= OL=$(OL) $(ARGS)
#######################################################################
#################################DSL only section - for development use

build:	bldmk
	get -p -r`gsid model $(REL)` s.model.src $(REWIRE) | ntar -d $(RDIR) -g
bldmk:  ;  get -p -r`gsid model.mk $(REL)` s.model.mk > $(RDIR)/model.mk

listing:
	pr model.mk $(SOURCE) | $(LIST)
listmk: ;  pr model.mk | $(LIST)

edit:
	get -e -p s.model.src | ntar -g

delta:
	ntar -p $(SOURCE) > model.src
	delta s.model.src
	rm -f $(SOURCE)

mkedit:  ;  get -e s.model.mk
mkdelta: ;  delta s.model.mk
#######################################################################

clean:	; :
clobber:  ; :
delete:	; :
	rm -f $(SOURCE)
