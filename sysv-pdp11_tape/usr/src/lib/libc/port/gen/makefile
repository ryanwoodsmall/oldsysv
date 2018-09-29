#	@(#)makefile	1.1
#
# makefile for libc/port/gen
#
#

.SUFFIXES: .p
CC=cc
CFLAGS=-O

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
l3.o          l64a.o        ldexp.o       lsearch.o	\
malloc.o      memccpy.o     memchr.o      memcmp.o	\
memcpy.o      memset.o      mktemp.o      modf.o	\
mon.o         nlist.o       perror.o      putpwent.o	\
qsort.o       rand.o        sleep.o       ssignal.o	\
strcat.o      strchr.o      strcmp.o      strcpy.o	\
strcspn.o     strlen.o      strncat.o     strncmp.o	\
strncpy.o     strpbrk.o     strrchr.o     strspn.o	\
strtok.o      strtol.o      swab.o        tell.o	\
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
l3.p          l64a.p        ldexp.p       lsearch.p	\
malloc.p      memccpy.p     memchr.p      memcmp.p	\
memcpy.p      memset.p      mktemp.p      modf.p	\
mon.p         nlist.p       perror.p      putpwent.p	\
qsort.p       rand.p        sleep.p       ssignal.p	\
strcat.p      strchr.p      strcmp.p      strcpy.p	\
strcspn.p     strlen.p      strncat.p     strncmp.p	\
strncpy.p     strpbrk.p     strrchr.p     strspn.p	\
strtok.p      strtol.p      swab.p        tell.p	\
tolower.p     toupper.p     tsearch.p     ttyname.p	\
ttyslot.p 

nonprof: $(OBJECTS)

prof: $(POBJECTS)

nlist.o nlist.p: /usr/include/a.out.h /usr/include/ar.h
atof.o atof.p ctype.o ctype.p getpw.o getpw.p \
	strtol.o strtol.p: /usr/include/ctype.h
dial.o dial.p: /usr/include/dial.h /usr/include/sys/termio.h
dial.o dial.p ftw.o ftw.p \
	getut.o getut.p: /usr/include/errno.h /usr/include/sys/errno.h
dial.o dial.p: /usr/include/fcntl.h
ftw.o ftw.p: /usr/include/ftw.h
getgrent.o getgrent.p getgrgid.o getgrgid.p \
	getgrnam.o getgrnam.p: /usr/include/grp.h
mon.o mon.p: /usr/include/mon.h
ecvt.o ecvt.p frexp.o frexp.p ldexp.o ldexp.p \
	modf.o modf.p: /usr/include/nan.h
getpwent.o getpwent.p getpwnam.o getpwnam.p \
	getpwuid.o getpwuid.p \
	putpwent.o putpwent.p: /usr/include/search.h
tsearch.o tsearch.p: /usr/include/search.h
abort.o abort.p dial.o dial.p sleep.o sleep.p \
	ssignal.o ssignal.p: /usr/include/signal.h \
		/usr/include/sys/signal.h
dial.o dial.p drand48.o drand48.p getcwd.o getcwd.p \
	getgrent.o getgrent.p getopt.o getopt.p \
	getpw.o getpw.p getpwent.o getpwent.p \
	getut.o getut.p \
	hsearch.o hsearch.p \
	putpwent.o putpwent.p: /usr/include/string.h
dial.o dial.p: /usr/include/string.h
ftw.o ftw.p ttyname.o ttyname.p: /usr/include/sys/dir.h
execvp.o execvp.p getcwd.o getcwd.p: /usr/include/sys/errno.h
clock.o clock.p getpwent.o getpwent.p: /usr/include/sys/param.h
ftok.o ftok.p ftw.o ftw.p getut.o getut.p \
	ttyname.o ttyname.p: /usr/include/sys/stat.h
ftok.o ftok.p: /usr/include/sys/sysmacros.h
isatty.o isatty.p: /usr/include/sys/termio.h
clock.o clock.p: /usr/include/sys/times.h
clock.o clock.p ftok.o ftok.p ftw.o ftw.p \
	getlogin.o getlogin.p getut.o getut.p \
	ttyname.o ttyname.p \
	ttyslot.o ttyslot.p: /usr/include/sys/x25.h
dial.o dial.p: /usr/include/termio.h \
		/usr/include/sys/termio.h
ctime.o ctime.p: /usr/include/time.h
getlogin.o getlogin.p getut.o getut.p \
	ttyslot.o ttyslot.p: /usr/include/utmp.h
atof.o atof.p ecvt.o ecvt.p l64a.o l64a.p \
	ldexp.o ldexp.p modf.o modf.p: /usr/include/values.h

.c.o .c.p:
	@echo $*.c:
	$(PROF)$(CC) -p $(CFLAGS) -c $*.c   && mv $(*F).o $*.p
	$(CC) $(CFLAGS) -c $*.c
