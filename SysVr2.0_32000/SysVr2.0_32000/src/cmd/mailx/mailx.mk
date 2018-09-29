#
# "@(#)mailx.mk	1.3"
# mailx -- a modified version of a University of California at Berkeley
#	mail program
#
# for standard Unix
#

ROOT=
VERSION=	2.14
HDR=	hdr
HELP=	help
DESTDIR= $(ROOT)/usr/bin
DESTLIB= $(ROOT)/usr/lib/mailx

DEFINES= -DUSG -DUSG_TTY
CFLAGS=	-O -I${HDR} ${DEFINES}
LDFLAGS= -n -i -s
LIBES=

XSTR=	./xstr
SH=	sh
CC=	cc
INS=/etc/install
RM=-/bin/rm
SIZE=/bin/size
MV=/bin/mv
CPIO=/bin/cpio
PR=/bin/pr
LP=/usr/bin/lp
TR=/usr/bin/tr
ECHO=/bin/echo
LINT=/usr/bin/lint
CHMOD=-/bin/chmod
CHGRP=-/bin/chgrp

OBJS=	version.o \
	aux.o \
	cmd1.o \
	cmd2.o \
	cmd3.o \
	cmd4.o \
	cmdtab.o \
	collect.o \
	config.o \
	edit.o \
	fio.o \
	getname.o \
	hostname.o \
	head.o \
	usg.local.o \
	lock.o \
	lex.o \
	list.o \
	main.o \
	names.o \
	optim.o \
	popen.o \
	quit.o \
	send.o \
	stralloc.o \
	temp.o \
	tty.o \
	vars.o \
	sigretro.o \
	lpaths.o \
	extern.o \
	str.o		# must be last

SRCS=	aux.c \
	cmd1.c \
	cmd2.c \
	cmd3.c \
	cmd4.c \
	cmdtab.c \
	collect.c \
	config.c \
	edit.c \
	fio.c \
	getname.c \
	head.c \
	hostname.c \
	usg.local.c \
	lock.c \
	lex.c \
	list.c \
	main.c \
	names.c \
	optim.c \
	popen.c \
	quit.c \
	send.c \
	stralloc.c \
	temp.c \
	tty.c \
	vars.c \
	sigretro.c \
	lpaths.c \
	extern.c

HDRS=	${HDR}/rcv.h \
	${HDR}/configdefs.h \
	${HDR}/def.h \
	${HDR}/glob.h \
	${HDR}/usg.local.h \
	${HDR}/local.h \
	${HDR}/uparm.h \
	${HDR}/sigretro.h

S=	${SRCS} version.c ${HDRS}

#
# Special massaging of C files for sharing of strings
#
.c.o:
	${CC} -E ${CFLAGS} $*.c | ${XSTR} -c -
	${CC} -c ${CFLAGS} x.c 
	${MV} x.o $*.o
	@${RM} -f x.c

all:	mailx rmmail

mailx:	${XSTR} $S ${OBJS}
	${RM} -f mailx
	${CC} ${LDFLAGS} -o mailx ${OBJS} ${LIBES}
	@${SIZE} mailx

rmmail:
	${CC} ${CFLAGS} ${LDFLAGS} -o rmmail rmmail.c

install: all
	${INS} -n ${DESTDIR} mailx
	if [ ! -d ${DESTLIB} ] ; then mkdir ${DESTLIB} ; fi
	${INS} -f ${DESTLIB} rmmail
	${CHGRP} mail ${DESTLIB}/rmmail
	${CHMOD} 2755 ${DESTLIB}/rmmail
	${INS} -f ${DESTLIB} ${HELP}/mailx.help
	${INS} -f ${DESTLIB} ${HELP}/mailx.help.~
	${CHMOD} 0664 ${DESTLIB}/mailx.help*

version.o:	/tmp version.c
	${CC} -c version.c
	${RM} -f version.c

version.c:
	${ECHO} \
	"char *version=\"mailx version ${VERSION} `date '+%m/%d/%y'`\";"\
		> version.c

clean:
	${RM} -f *.o
	${RM} -f version.c a.out x.c xs.c core strings

clobber:	clean
	${RM} -f mailx rmmail ${XSTR}

lint:	version.c
	${LINT} ${CFLAGS} ${SRCS}
	${RM} -f version.c
str.o:	strings
	${XSTR}
	${CC} -c xs.c
	${MV} xs.o str.o
	${RM} xs.c

cmdtab.o:	cmdtab.c
	${CC} -c ${CFLAGS} cmdtab.c

mailx.cpio:	${SRCS} ${HDRS} mailx.mk xstr.c
	@${ECHO} ${SRCS} ${HDRS} mailx.mk xstr.c | \
		${TR} " " "\012" | \
		${CPIO} -oc >mailx.cpio

${XSTR}:	xstr.c
	cc -O -o ${XSTR} xstr.c

listing:
	${PR} mailx.mk hdr/*.h [a-l]*.c | ${LP}
	${PR} [m-z]*.c | ${LP}
