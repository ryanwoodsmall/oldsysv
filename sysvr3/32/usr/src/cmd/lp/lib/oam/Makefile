#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:lib/oam/Makefile	1.6"
#
# Makefile for LP message management routines
#


RM	=	/bin/rm -f
AR	=	ar
INS	=	install


DEBUG	=	-g

##########
#
# Define the following if the message formatter should break long
# messages at word boundaries. The alternative is to break at a fixed
# length.
##########
SMARTDEF	=	-DSMART_WRAP

INC	=	$(ROOT)/usr/include
LPINC	=	../../include

CFLAGS	=	$(DEBUG) -I$(LPINC) -I$(INC) $(SMARTDEF) -O


LIBRARY	=	liblpoam.a
PLIBRARY=	p$(LIBRARY)
QLIBRARY=	q$(LIBRARY)

LINTSRC	=	llib-llpoam
LINTLIB	=	$(LINTSRC).ln


TEXTSRC	= \
		e_adm__msgs.c \
		e_can__msgs.c \
		e_cmb__msgs.c \
		e_fl__msgs.c \
		e_for__msgs.c \
		e_lp__msgs.c \
		e_lpp__msgs.c \
		e_lpu__msgs.c \
		e_mov__msgs.c \
		e_sht__msgs.c \
		e_stat__msgs.c

SRCS	= \
		agettxt.c \
		buffers.c \
		fmtmsg.c


LINTS	=

OBJS	=	$(SRCS:.c=.o) $(TEXTSRC:.c=.o)
POBJS	=	$(SRCS:.c=.p) $(TEXTSRC:.c=.p)
QOBJS	=	$(SRCS:.c=.q) $(TEXTSRC:.c=.q)

.SUFFIXES:	.o .p .q

.c.p:
	@-if [ -f $*.o ] ; then mv $*.o tmp ; fi
	$(CC) -p $(CFLAGS) -c $*.c
	@-mv $*.o $*.p
	@-if [ -f tmp ] ; then mv tmp $*.o ; fi

.c.q:
	@-if [ -f $*.o ] ; then mv $*.o tmp ; fi
	$(CC) -ql $(CFLAGS) -c $*.c
	@-mv $*.o $*.q
	@-if [ -f tmp ] ; then mv tmp $*.o ; fi


all:		$(LIBRARY) $(LINTLIB)

install:	all
	@echo "Not a delivered part of LP; not installed"
	@false

clean:	
	$(RM) $(OBJS) $(POBJS) $(QOBJS) $(TEXTSRC) $(LPINC)/oam_def.h xx??

clobber:	clean
	$(RM) $(LIBRARY) $(PLIBRARY) $(QLIBRARY) $(LINTLIB)

strip:
	$(STRIP) $(LIBRARY)


$(LIBRARY):	$(OBJS)
	$(AR) rv $(LIBRARY) $?

prof:		$(PLIBRARY)

$(PLIBRARY):	$(POBJS)
	$(AR) rv $(PLIBRARY) $?

lprof:		$(QLIBRARY)

$(QLIBRARY):	$(QOBJS)
	$(AR) rv $(QLIBRARY) $?


$(SRCS):	$(LPINC)/oam_def.h

$(LPINC)/oam_def.h:	msg.source \
			gen-defs \
			gen-text
	sh gen-defs >$(LPINC)/oam_def.h
	sh gen-text


spell:
	for file in $(TEXTSRC); \
	do \
		sed -e 's/\\n/ /g' -e 's/\\"//g' -e 's/%s//g' $$file; \
	done \
	| spell

lint:		spell
	lint -ux $(CFLAGS) $(SRCS) $(LINTS)

lintsrc:	$(LINTSRC)

$(LINTSRC):	$(SRCS)
	if [ -n "$(FUNCDCL)" ]; then $(FUNCDCL) $(SRCS) >$(LINTSRC); fi

lintlib:	$(LINTLIB)

$(LINTLIB):	$(LINTSRC)
	$(CC) -E -C $(CFLAGS) -Dlint $(LINTSRC) | /usr/lib/lint1 -vx -H./LINT_WORK > $(LINTLIB)
	$(RM) ./LINT_WORK
