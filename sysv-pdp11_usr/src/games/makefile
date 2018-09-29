#		/usr/src/games makefile
#
#		@(#)makefile	1.1
#
#	Directory for game installation
DIR=/usr/games
#
all:	arithmetic back bj craps fish fortune hangman master/mastermind maze \
	moo number psych quiz random trk/trk ttt wump
back:	back.c
	@echo cc -O back.c -o back
	@if pdp11 ; then pcc -O back.c -o back; else cc -O back.c -o back;fi
bj:	bj.c
	cc -O bj.c -o bj -lm
master/mastermind:
	cd master; make
psych:	psych.c
	cc -s -n -O psych.c -o psych -l4014 -lm
trk/trk:
	cd trk; make

install:	all
	rm -f $(DIR)/arithmetic $(DIR)/back $(DIR)/bj $(DIR)/craps $(DIR)/fish \
	$(DIR)/fortune $(DIR)/hangman $(DIR)/mastermind $(DIR)/maze \
	$(DIR)/moo $(DIR)/number $(DIR)/psych $(DIR)/quiz $(DIR)/random \
	$(DIR)/trk $(DIR)/ttt $(DIR)/wump $(DIR)/mmhow
	cp arithmetic back bj craps fish fortune hangman master/mastermind \
	maze moo number psych quiz random trk/trk ttt wump mmhow $(DIR)
	chmod = $(DIR)/arithmetic $(DIR)/back $(DIR)/bj $(DIR)/craps $(DIR)/fish \
	$(DIR)/fortune $(DIR)/hangman $(DIR)/mastermind $(DIR)/maze \
	$(DIR)/moo $(DIR)/number $(DIR)/psych $(DIR)/quiz $(DIR)/random \
	$(DIR)/trk $(DIR)/ttt $(DIR)/wump
	chown games $(DIR)/arithmetic $(DIR)/back $(DIR)/bj $(DIR)/craps $(DIR)/fish \
	$(DIR)/fortune $(DIR)/hangman $(DIR)/mastermind $(DIR)/maze \
	$(DIR)/moo $(DIR)/number $(DIR)/psych $(DIR)/quiz $(DIR)/random \
	$(DIR)/trk $(DIR)/ttt $(DIR)/wump $(DIR)/mmhow

clean:
	rm -f *.o */*.o

clobber:	clean
	rm -f arithmetic back bj craps fish fortune hangman master/mastermind \
	maze moo number psych quiz random trk/trk ttt wump
