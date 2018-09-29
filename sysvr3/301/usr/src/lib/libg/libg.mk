#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libg:libg.mk	1.13"
# machine independent libg.mk

ROOT=
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib

all:
	if vax; then							\
		cd vax;					\
		$(MAKE) -f libg.mk all ROOT=$(ROOT);			\
	elif u3b; then							\
		cd u3b;					\
		$(MAKE) -f libg.mk all ROOT=$(ROOT);			\
	elif u3b15 || u3b5 || u3b2; then							\
		cd m32;					\
		$(MAKE) -f libg.mk all ROOT=$(ROOT);			\
	fi;

install:	all
	if vax; then							\
		cd vax;					\
		$(MAKE) -f libg.mk install ROOT=$(ROOT);		\
	elif u3b; then							\
		cd u3b;					\
		$(MAKE) -f libg.mk install ROOT=$(ROOT);		\
	elif u3b15 || u3b5 || u3b2; then						\
		cd m32;					\
		$(MAKE) -f libg.mk LIB=$(LIB) USRLIB=$(USRLIB) install ROOT=$(ROOT);	\
	fi;

clean:
	if vax; then							\
		cd vax;					\
		$(MAKE) -f libg.mk clean ROOT=$(ROOT);			\
	elif u3b; then							\
		cd u3b;					\
		$(MAKE) -f libg.mk clean ROOT=$(ROOT);			\
	elif u3b15 || u3b5 || u3b2; then							\
		cd m32;					\
		$(MAKE) -f libg.mk clean ROOT=$(ROOT);			\
	fi;

clobber:
	if vax; then							\
		cd vax;					\
		$(MAKE) -f libg.mk clobber ROOT=$(ROOT);		\
	elif u3b; then							\
		cd u3b;					\
		$(MAKE) -f libg.mk clobber ROOT=$(ROOT);		\
	elif u3b15 || u3b5 || u3b2; then							\
		cd m32;					\
		$(MAKE) -f libg.mk clobber ROOT=$(ROOT);		\
	fi;
