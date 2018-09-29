#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)help:help.mk	1.13"
#       help makefile

ROOT	= 
BIN	= $(ROOT)/usr/bin
ADM	= $(ROOT)/etc
LIB	= $(ROOT)/usr/lib/help
DB	= $(LIB)/db
TMP	= /usr/tmp
CFLAGS	= -O -n -s -i
INS	= install -f

all:		help interact keysrch fetch helpadm admstart admgloss \
		editcmd defnlen checklen locking glossary

interact:	interact.c switch.h
	$(CC) $(CFLAGS) -o interact interact.c -lcurses

fetch:		fetch.c fetch.h
	cc $(CFLAGS) -o replace fetch.c -lld
	$(CC) $(CFLAGS) -o fetch fetch.c -lld

db:	install_fetch
	mkdir $(TMP)/descriptions
	cp db/descriptions/* $(TMP)/descriptions
	-chmod 644 $(TMP)/descriptions/*
	-chgrp bin $(TMP)/descriptions/*
	-chown root $(TMP)/descriptions/*

	mkdir $(TMP)/options
	cp db/options/* $(TMP)/options
	-chmod 644 $(TMP)/options/*
	-chgrp bin $(TMP)/options/*
	-chown root $(TMP)/options/*

	mkdir $(TMP)/examples
	cp db/examples/* $(TMP)/examples
	-chmod 644 $(TMP)/examples/*
	-chgrp bin $(TMP)/examples/*
	-chown root $(TMP)/examples/*

	mkdir $(TMP)/glossary
	cp db/glossary/* $(TMP)/glossary
	-chmod 644 $(TMP)/glossary/*
	-chgrp bin $(TMP)/glossary/*
	-chown root $(TMP)/glossary/*

	mkdir $(TMP)/screens
	cp db/screens/* $(TMP)/screens
	-chmod 644 $(TMP)/screens/*
	-chgrp bin $(TMP)/screens/*
	-chown root $(TMP)/screens/*

	mkdir $(TMP)/screensfoot
	cp db/screensfoot/* $(TMP)/screensfoot
	-chmod 644 $(TMP)/screensfoot/*
	-chgrp bin $(TMP)/screensfoot/*
	-chown root $(TMP)/screensfoot/*

	-./replace $(TMP)/descriptions.a $(TMP)/descriptions/*
	-./replace $(TMP)/examples.a $(TMP)/examples/*
	-./replace $(TMP)/options.a $(TMP)/options/*
	-./replace $(TMP)/glossary.a $(TMP)/glossary/*
	-./replace $(TMP)/screens.a $(TMP)/screens/*
	-./replace $(TMP)/screens.a $(TMP)/screensfoot/*

	-rm -fr $(TMP)/descriptions $(TMP)/examples $(TMP)/options $(TMP)/glossary \
	$(TMP)/screens $(TMP)/screensfoot

install:	install_interact install_fetch install_help install_keysrch \
		install_helpadm install_db log install_glossary

install_fetch:	fetch lib
	@rm -f $(LIB)/fetch $(LIB)/list $(LIB)/extract $(LIB)/delete $(LIB)/replace
	$(INS) $(LIB) fetch
	ln $(LIB)/fetch $(LIB)/extract
	ln $(LIB)/fetch $(LIB)/list
	ln $(LIB)/fetch $(LIB)/delete
	ln $(LIB)/fetch $(LIB)/replace

install_interact:	interact lib
	@rm -f $(LIB)/interact
	$(INS) $(LIB) interact

install_help:	help
	@rm -f $(BIN)/help $(BIN)/glossary $(BIN)/locate $(BIN)/starter $(BIN)/usage
	$(INS) $(BIN) help
	ln $(BIN)/help $(BIN)/glossary
	ln $(BIN)/help $(BIN)/locate
	ln $(BIN)/help $(BIN)/starter
	ln $(BIN)/help $(BIN)/usage

install_keysrch:	keysrch lib
	@rm -f $(LIB)/keysrch
	$(INS) $(LIB) keysrch

install_glossary:	glossary lib
	@rm -f $(LIB)/glossary
	$(INS) $(LIB) glossary

install_helpadm:	helpadm admgloss admstart checklen defnlen editcmd locking lib
	@rm -f $(ADM)/helpadm $(LIB)/admstart $(LIB)/admgloss \
		$(LIB)/editcmd $(LIB)/defnlen $(LIB)/checklen $(LIB)/locking
	$(INS) $(ADM) -m 754 helpadm
	$(INS) $(LIB) -m 754 admstart
	$(INS) $(LIB) -m 754 admgloss
	$(INS) $(LIB) -m 754 editcmd
	$(INS) $(LIB) -m 754 defnlen
	$(INS) $(LIB) -m 754 checklen
	$(INS) $(LIB) -m 754 locking

install_db:	db lib daba tables
	@rm -f $(DB)/tables/display $(DB)/tables/keywords $(DB)/tables/responses \
		$(DB)/descriptions.a $(DB)/examples.a \
		$(DB)/options.a $(DB)/glossary.a $(DB)/screens.a
	cp db/tables/display $(DB)/tables/display
	cp db/tables/keywords $(DB)/tables/keywords
	cp db/tables/responses $(DB)/tables/responses
	mv $(TMP)/descriptions.a $(DB)/descriptions.a
	mv $(TMP)/examples.a $(DB)/examples.a
	mv $(TMP)/options.a $(DB)/options.a
	mv $(TMP)/glossary.a $(DB)/glossary.a
	mv $(TMP)/screens.a $(DB)/screens.a

	$(CH)chmod 664 $(DB)/tables/display
	$(CH)chgrp bin $(DB)/tables/display
	$(CH)chown root $(DB)/tables/display

	$(CH)chmod 664 $(DB)/tables/keywords
	$(CH)chgrp bin $(DB)/tables/keywords
	$(CH)chown root $(DB)/tables/keywords

	$(CH)chmod 664 $(DB)/tables/responses
	$(CH)chgrp bin $(DB)/tables/responses
	$(CH)chown root $(DB)/tables/responses

	$(CH)chmod 664 $(DB)/descriptions.a
	$(CH)chgrp bin $(DB)/descriptions.a
	$(CH)chown root $(DB)/descriptions.a

	$(CH)chmod 664 $(DB)/examples.a
	$(CH)chgrp bin $(DB)/examples.a
	$(CH)chown root $(DB)/examples.a

	$(CH)chmod 664 $(DB)/options.a
	$(CH)chgrp bin $(DB)/options.a
	$(CH)chown root $(DB)/options.a

	$(CH)chmod 664 $(DB)/glossary.a
	$(CH)chgrp bin $(DB)/glossary.a
	$(CH)chown root $(DB)/glossary.a

	$(CH)chmod 664 $(DB)/screens.a
	$(CH)chgrp bin $(DB)/screens.a
	$(CH)chown root $(DB)/screens.a

log:
	> $(LIB)/HELPLOG
	$(CH)chmod 555 $(LIB)/HELPLOG
	$(CH)chgrp bin $(LIB)/HELPLOG
	$(CH)chown bin $(LIB)/HELPLOG
	$(INS) $(LIB) -u bin -g bin helpclean
lib:
	-mkdir $(LIB)
	$(CH)chgrp bin $(LIB)
	$(CH)chown bin $(LIB)

daba:
	-mkdir $(DB)
	$(CH)chmod 775 $(DB)
	$(CH)chgrp bin $(DB)
	$(CH)chown root $(DB)

tables:
	-mkdir $(DB)/tables
	$(CH)chmod 775 $(DB)/tables
	$(CH)chgrp bin $(DB)/tables
	$(CH)chown root $(DB)/tables

clobber:	clean
		-rm -rf *.o
		-rm ./replace

clean:
	rm -f help display fetch interact keysrch helpadm glossary \
		admgloss admstart checklen defnlen editcmd locking

.PRECIOUS:	help.sh keysrch.sh interact.c fetch.c helpadm.sh glossary.sh


