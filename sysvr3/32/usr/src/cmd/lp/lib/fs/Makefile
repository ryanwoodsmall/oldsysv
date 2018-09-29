#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nlp:lib/fs/Makefile	1.4"
#
# Makefile for file system access routines (LP)
#


RM	=	/bin/rm -f
AR	=	ar
INS	=	install


#
# Define the following if the getdents() system call sets
# the ".d_off" member of the "struct dirent" structure
# as documented in the dirent(4) manual page.
# IF THIS HAPPENS, KNOW THAT THE CODE WILL NOT WORK ON
# MACHINES THAT DON'T HAVE THIS FIX IN THE KERNEL!

# GETDENTS = -DGETDENTS_FIXED


DEBUG	=	-g

INC	=	$(ROOT)/usr/include
LPINC	=	../../include

CFLAGS	=	$(DEBUG) -I$(LPINC) -I$(INC) -O $(GETDENTS)


LIBRARY	=	liblpfs.a
PLIBRARY=	p$(LIBRARY)
QLIBRARY=	q$(LIBRARY)

LINTSRC	=	llib-llpfs
LINTLIB	=	$(LINTSRC).ln


SRCS	= \
		Mkdir.c \
		Readdir.c \
		Rmdir.c \
		next.c \
		s5getdents.c \
		s5mkdir.c \
		s5readdir.c \
		s5rmdir.c \
		see_if_SVR3.c

LINTS	=

OBJS	=	$(SRCS:.c=.o)
POBJS	=	$(SRCS:.c=.p)
QOBJS	=	$(SRCS:.c=.q)

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
	$(RM) $(OBJS) $(POBJS) $(QOBJS)

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

lint:
	lint -ux $(CFLAGS) $(SRCS) $(LINTS)

lintsrc:	$(LINTSRC)

$(LINTSRC):	$(SRCS)
	if [ -n "$(FUNCDCL)" ]; then $(FUNCDCL) $(SRCS) >$(LINTSRC); fi

lintlib:	$(LINTLIB)

$(LINTLIB):	$(LINTSRC)
	$(CC) -E -C $(CFLAGS) -Dlint $(LINTSRC) | /usr/lib/lint1 -vx -H./LINT_WORK > $(LINTLIB)
	$(RM) ./LINT_WORK
