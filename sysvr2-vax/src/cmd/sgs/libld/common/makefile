#
#	COMMON LIBLD MAKEFILE
#
#	@(#) makefile: 1.7 3/17/83
#
SGS	= 
#
#	Host Dependent Parameters
#	ARFORMAT = archive format
#		OLDAR	= old format
#		PORT5AR	= 5.0 random access format
#		PORTAR	= 6.0 portable ascii format
#
ARCH	= AR32W
FLEX	= -DFLEXNAMES
ARFORMAT = PORTAR
#
#	Compilation Parameters
#
CC	= cc
CFLAGS	= -O
DEFLIST	= -D$(ARFORMAT) -D$(ARCH) $(FLEX)
INCLIST	= -I$(INCCOM) -I$(USRINC)
ENV	= 
FRC	=
CC_CMD	= $(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
#
AR	= ar
ARFLAGS	= r
AR_CMD	= $(AR) $(ARFLAGS)
#
LINT	= lint
LINTFLAGS = -p
LINT_CMD = $(LINT) $(LINTFLAGS)
O	= o
#
#	External Directories
#
ROOT 	= 
LIBDIR	= $(ROOT)/lib
#
#	Internal Directories
#
SGSBASE	=  ../..
INCCOM	= $(SGSBASE)/inc/common
VERSION	=
USRINC	= $(ROOT)/usr/$(VERSION)include
#
CP	= cp
RM	= rm
#
PRINT	= pr
PRFLAGS	= -n
LP	= lp
LPFLAGS	=
#
OBJECTS = ldaclose.$O ldaopen.$O ldclose.$O ldahread.$O ldlseek.$O ldnlseek.$O \
	ldnrseek.$O ldnsseek.$O ldnshread.$O ldopen.$O ldrseek.$O ldlread.$O \
	ldsseek.$O ldshread.$O ldfhread.$O ldtbseek.$O ldohseek.$O ldtbread.$O \
	ldtbindex.$O ldgetname.$O allocldptr.$O freeldptr.$O vldldptr.$O \
	sgetl.$O sputl.$O
#
CFILES = ldaclose.c ldaopen.c ldclose.c ldahread.c ldlseek.c ldnlseek.c \
	ldnrseek.c ldnsseek.c ldnshread.c ldopen.c ldrseek.c ldlread.c \
	ldsseek.c ldshread.c ldfhread.c ldtbseek.c ldohseek.c ldtbread.c \
	ldtbindex.c allocldptr.c freeldptr.c vldldptr.c sgetl.c sputl.c \
	ldgetname.c
#
HEADERS	= $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h $(INCCOM)/scnhdr.h \
	  $(INCCOM)/syms.h $(INCCOM)/storclass.h $(INCCOM)/linenum.h lddef.h
#
all:	libld.a
#
libld.a:  $(INCCOM)/ldfcn.h $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h \
	  $(INCCOM)/syms.h lddef.h $(OBJECTS) $(FRC)
	$(RM) -f libld.a
	$(AR_CMD) libld.a $(OBJECTS)
#
allocldptr.$O:	$(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h lddef.h \
		allocldptr.c $(FRC)
	$(CC_CMD) allocldptr.c
#
freeldptr.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h lddef.h \
	      freeldptr.c $(FRC)
	$(CC_CMD) freeldptr.c
#
vldldptr.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h lddef.h \
	     vldldptr.c $(FRC)
	$(CC_CMD) vldldptr.c
#
ldaclose.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h ldaclose.c $(FRC)
	$(CC_CMD) ldaclose.c
#
ldaopen.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h ldaopen.c $(FRC)
	$(CC_CMD) ldaopen.c
#
ldclose.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h ldclose.c $(FRC)
	$(CC_CMD) ldclose.c
#
ldahread.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h ldahread.c $(FRC)
	$(CC_CMD) ldahread.c
#
ldlseek.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
		ldlseek.c $(FRC)
	$(CC_CMD) ldlseek.c
#
ldnlseek.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
	     ldnlseek.c $(FRC)
	$(CC_CMD) ldnlseek.c
#
ldnrseek.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
	     ldnrseek.c $(FRC)
	$(CC_CMD) ldnrseek.c
#
ldnsseek.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
	     ldnsseek.c $(FRC)
	$(CC_CMD) ldnsseek.c
#
ldnshread.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
	      ldnshread.c $(FRC)
	$(CC_CMD) ldnshread.c
#
ldopen.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h ldopen.c $(FRC)
	$(CC_CMD) ldopen.c
#
ldrseek.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h \
		$(INCCOM)/ldfcn.h ldrseek.c $(FRC)
	$(CC_CMD) ldrseek.c
#
ldtbread.$O:  $(INCCOM)/filehdr.h $(INCCOM)/syms.h $(INCCOM)/storclass.h \
	     $(INCCOM)/ldfcn.h ldtbread.c $(FRC)
	$(CC_CMD) ldtbread.c
#
ldsseek.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h \
		$(INCCOM)/ldfcn.h ldsseek.c $(FRC)
	$(CC_CMD) ldsseek.c
#
ldshread.$O:  $(INCCOM)/filehdr.h $(INCCOM)/scnhdr.h $(INCCOM)/ldfcn.h \
	     ldshread.c $(FRC)
	$(CC_CMD) ldshread.c
#
ldfhread.$O:  $(INCCOM)/filehdr.h $(INCCOM)/ldfcn.h ldfhread.c $(FRC)
	$(CC_CMD) ldfhread.c
#
ldtbseek.$O:  $(INCCOM)/ldfcn.h $(INCCOM)/filehdr.h ldtbseek.c $(FRC)
	$(CC_CMD) ldtbseek.c
#
ldohseek.$O:  $(INCCOM)/ldfcn.h $(INCCOM)/filehdr.h ldohseek.c $(FRC)
	$(CC_CMD) ldohseek.c
#
ldtbindex.$O:  $(INCCOM)/ldfcn.h $(INCCOM)/filehdr.h $(INCCOM)/syms.h \
	      $(INCCOM)/storclass.h ldtbindex.c $(FRC)
	$(CC_CMD) ldtbindex.c
#
ldlread.$O:  $(INCCOM)/ldfcn.h $(INCCOM)/filehdr.h \
		$(INCCOM)/scnhdr.h $(INCCOM)/syms.h \
	    $(INCCOM)/linenum.h $(INCCOM)/storclass.h ldlread.c $(FRC)
	$(CC_CMD) ldlread.c
#
sgetl.$O:  sgetl.c $(FRC)
	$(CC_CMD) sgetl.c
#
sputl.$O:  sputl.c $(FRC)
	$(CC_CMD) sputl.c
#
ldgetname.$O:  $(INCCOM)/ldfcn.h $(INCCOM)/filehdr.h $(INCCOM)/syms.h \
	      $(INCCOM)/storclass.h ldgetname.c $(FRC)
	$(CC_CMD) ldgetname.c
#
install:	$(LIBDIR)/libld.a
#
$(LIBDIR)/libld.a:	libld.a
	$(RM) -f $(LIBDIR)/libld.a
	$(CP) libld.a $(LIBDIR)/libld.a
#
save:
	$(RM) -f $(LIBDIR)/libld.a.back
	$(CP) $(LIBDIR)/libld.a $(LIBDIR)/libld.a.back
#
uninstall:	$(LIBDIR)/libld.a.back
	$(RM) -f $(LIBDIR)/libld.a
	$(CP) $(LIBDIR)/libld.a.back $(LIBDIR)/libld.a
#
shrink:
	$(RM) -f $(OBJECTS) libld.a
#
clean:
	$(RM) -f $(OBJECTS)
#
clobber:	clean
	$(RM) -f libld.a
#
lint:	$(HEADERS) $(CFILES)
	$(LINT_CMD) $(DEFLIST) $(INCLIST) $(CFILES)
#
listing:	$(HEADERS) $(CFILES)
	$(PRINT) $(PRFLAGS) $(HEADERS) $(SOURCE) | $(LP) $(LPFLAGS)
#
FRC:
