#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)tools:tools.mk	1.16"

CURSES = -lcurses
LIBES = $(CURSES) -lm
INCLUDE =
FORMS = ../forms
CFLAGS=-I$(FORMS) $(INCLUDE)
FFLAG =
LDFLAGS = $(FFLAG) -s
INS = /etc/install

OBS =   bottom.o show_cap.o get_path.o m_field.o m_global.o vsupport.o cf_global.o \
	dhelp.o err_mess.o move.o cf_field.o pick_val.o util.o editval.o \
	select.o control.o writefile.o perm_chk.o \
	$(FORMS)/read_in.o field_type.o $(FORMS)/support.o

tforms:	$(OBS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o tforms $(OBS) $(LIBES)

bottom.o:	

show_cap.o:	$(FORMS)/muse.h $(FORMS)/musedefs.h

get_path.o:	$(FORMS)/muse.h $(FORMS)/musedefs.h

m_field.o:	$(FORMS)/muse.h tools.h vdefs.h $(FORMS)/musedefs.h

perm_chk.o:	$(FORMS)/muse.h tools.h vdefs.h $(FORMS)/musedefs.h

m_global.o:	$(FORMS)/muse.h tools.h vdefs.h $(FORMS)/musedefs.h

field_type.o:	$(FORMS)/muse.h vdefs.h $(FORMS)/musedefs.h

control.o:	$(FORMS)/muse.h vdefs.h $(FORMS)/musedefs.h

editval.o:	$(FORMS)/muse.h vdefs.h $(FORMS)/musedefs.h

select.o:	$(FORMS)/muse.h vdefs.h $(FORMS)/musedefs.h

vsupport.o:	$(FORMS)/muse.h vdefs.h $(FORMS)/musedefs.h

writefile.o:	$(FORMS)/muse.h vdefs.h $(FORMS)/musedefs.h

util.o:	$(FORMS)/muse.h vdefs.h $(FORMS)/musedefs.h

cf_field.o:	$(FORMS)/muse.h tools.h vdefs.h $(FORMS)/musedefs.h

cf_global.o:	$(FORMS)/muse.h tools.h vdefs.h $(FORMS)/musedefs.h

pick_val.o:	$(FORMS)/muse.h tools.h vdefs.h $(FORMS)/musedefs.h

move.o:	$(FORMS)/muse.h tools.h $(FORMS)/musedefs.h

err_mess.o:	$(FORMS)/muse.h tools.h  $(FORMS)/musedefs.h

dhelp.o:	$(FORMS)/muse.h tools.h $(FORMS)/musedefs.h

show_field.o:	$(FORMS)/muse.h $(FORMS)/musedefs.h

$(FORMS)/read_in.o: $(FORMS)/muse.h $(FORMS)/read_in.c $(FORMS)/musedefs.h

$(FORMS)/support.o: $(FORMS)/muse.h $(FORMS)/support.c $(FORMS)/musedefs.h

install: tforms
	$(INS) -f $(BINDIR) tforms 

clobber: clean
	-rm -f tforms

clean:
	-rm -f $(OBS)
