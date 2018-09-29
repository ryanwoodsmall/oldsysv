#	@(#)makefile	1.5
#
# makefile for libc/port/gen
#
#

.SUFFIXES: .p
CC=cc
CFLAGS= -O
ROOT=
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include/sys

OBJECTS=\
a64l.o        abort.o       abs.o         assert.o	\
atof.o        atoi.o        atol.o        bsearch.o	\
calloc.o      clock.o       crypt.o       ctime.o	\
ctype.o       dial.o        drand48.o     ecvt.o	\
errlst.o      execvp.o      fakcu.o       frexp.o	\
ftok.o        ftw.o         gcvt.o        getcwd.o	\
getenv.o      getgrent.o    getgrgid.o    getgrnam.o	\
getlogin.o    getopt.o      getpw.o       getpwent.o	\
getpwnam.o    getpwuid.o    \
getut.o       hsearch.o     isatty.o	\
l3.o          l64a.o        ldexp.o       lfind.o	\
lsearch.o	\
malloc.o      memccpy.o     memchr.o      memcmp.o	\
memcpy.o      memset.o      mktemp.o      modf.o	\
mon.o         nlist.o       perror.o      putenv.o	\
putpwent.o	\
qsort.o       rand.o        sleep.o       ssignal.o	\
strcat.o      strchr.o      strcmp.o      strcpy.o	\
strcspn.o     strlen.o      strncat.o     strncmp.o	\
strncpy.o     strpbrk.o     strrchr.o     strspn.o	\
strtod.o	\
strtok.o      strtol.o      swab.o        tell.o	\
tfind.o	\
tolower.o     toupper.o     tsearch.o     ttyname.o	\
ttyslot.o 

POBJECTS=\
a64l.p        abort.p       abs.p         assert.p	\
atof.p        atoi.p        atol.p        bsearch.p	\
calloc.p      clock.p       crypt.p       ctime.p	\
ctype.p       dial.p        drand48.p     ecvt.p	\
errlst.p      execvp.p      fakcu.p       frexp.p	\
ftok.p        ftw.p         gcvt.p        getcwd.p	\
getenv.p      getgrent.p    getgrgid.p    getgrnam.p	\
getlogin.p    getopt.p      getpw.p       getpwent.p	\
getpwnam.p    getpwuid.p    \
getut.p       hsearch.p     isatty.p	\
l3.p          l64a.p        ldexp.p       lfind.p	\
lsearch.p	\
malloc.p      memccpy.p     memchr.p      memcmp.p	\
memcpy.p      memset.p      mktemp.p      modf.p	\
mon.p         nlist.p       perror.p      putenv.p	\
putpwent.p	\
qsort.p       rand.p        sleep.p       ssignal.p	\
strcat.p      strchr.p      strcmp.p      strcpy.p	\
strcspn.p     strlen.p      strncat.p     strncmp.p	\
strncpy.p     strpbrk.p     strrchr.p     strspn.p	\
strtod.p	\
strtok.p      strtol.p      swab.p        tell.p	\
tfind.p\
tolower.p     toupper.p     tsearch.p     ttyname.p	\
ttyslot.p 

nonprof: $(OBJECTS)

prof: $(POBJECTS)

nlist.o nlist.p: $(INC)/a.out.h $(INC)/ar.h
atof.o atof.p ctype.o ctype.p getpw.o getpw.p \
	strtol.o strtol.p: $(INC)/ctype.h
dial.o dial.p: $(INC)/dial.h $(INCSYS)/termio.h
dial.o dial.p ftw.o ftw.p \
	getut.o getut.p: $(INC)/errno.h $(INCSYS)/errno.h
dial.o dial.p: $(INC)/fcntl.h
ftw.o ftw.p: $(INC)/ftw.h
getgrent.o getgrent.p getgrgid.o getgrgid.p \
	getgrnam.o getgrnam.p: $(INC)/grp.h
mon.o mon.p: $(INC)/mon.h
ecvt.o ecvt.p frexp.o frexp.p ldexp.o ldexp.p \
	modf.o modf.p: $(INC)/nan.h
getpwent.o getpwent.p getpwnam.o getpwnam.p \
	getpwuid.o getpwuid.p \
	putpwent.o putpwent.p: $(INC)/search.h
tsearch.o tsearch.p tfind.o tfind.p: $(INC)/search.h
abort.o abort.p dial.o dial.p sleep.o sleep.p \
	ssignal.o ssignal.p: $(INC)/signal.h \
		$(INCSYS)/signal.h
dial.o dial.p drand48.o drand48.p getcwd.o getcwd.p \
	getgrent.o getgrent.p getopt.o getopt.p \
	getpw.o getpw.p getpwent.o getpwent.p \
	getut.o getut.p \
	hsearch.o hsearch.p \
	putpwent.o putpwent.p: $(INC)/string.h
dial.o dial.p: $(INC)/string.h
ftw.o ftw.p ttyname.o ttyname.p: $(INCSYS)/dir.h
execvp.o execvp.p getcwd.o getcwd.p: $(INCSYS)/errno.h
clock.o clock.p getpwent.o getpwent.p: $(INCSYS)/param.h
ftok.o ftok.p ftw.o ftw.p getut.o getut.p \
	ttyname.o ttyname.p: $(INCSYS)/stat.h
ftok.o ftok.p: $(INCSYS)/sysmacros.h
isatty.o isatty.p: $(INCSYS)/termio.h
clock.o clock.p: $(INCSYS)/times.h
dial.o dial.p: $(INC)/termio.h \
		$(INCSYS)/termio.h
ctime.o ctime.p: $(INC)/time.h
getlogin.o getlogin.p getut.o getut.p \
	ttyslot.o ttyslot.p: $(INC)/utmp.h
atof.o atof.p ecvt.o ecvt.p l64a.o l64a.p \
	ldexp.o ldexp.p modf.o modf.p strtod.o strtod.p: $(INC)/values.h

.c.o .c.p:
	@echo $*.c:
	$(PROF)$(CC) -c -p $(CFLAGS) $*.c   && mv $(*F).o $*.p
	$(CC) $(CFLAGS) -c $*.c
