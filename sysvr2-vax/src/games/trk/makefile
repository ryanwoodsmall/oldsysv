#	STAR TREK GAME MAKEFILE
#
#	@(#)makefile	1.1
#
CFLGS=-c $(CFLAGS)

all:	trk

trk:	abandon.o adm.o attack.o checkcond.o compkl.o\
	computer.o damage.o destruct.o dock.o dump.o\
	events.o externs.o extra.o getcodi.o\
	getpar.o help.o impulse.o input.o initquad.o kill.o\
	klmove.o log.o lose.o lrscan.o main.o move.o\
	nova.o phaser.o play.o query.o ram.o ranf.o\
	report.o reset.o rest.o schedule.o score.o setup.o setwarp.o\
	shell.o shield.o snova.o srscan.o ssradio.o torped.o visual.o\
	warp.o win.o
	$(CC) *.o -i -s -lm -o trk

abandon.o:	trek.h abandon.c
	$(CC) $(CFLGS) abandon.c
adm.o:	trek.h adm.c
	$(CC) $(CFLGS) adm.c
attack.o:	trek.h attack.c
	$(CC) $(CFLGS) attack.c
checkcond.o:	trek.h checkcond.c
	$(CC) $(CFLGS) checkcond.c
compkl.o:	trek.h compkl.c
	$(CC) $(CFLGS) compkl.c
computer.o:	trek.h computer.c
	$(CC) $(CFLGS) computer.c
damage.o:	trek.h damage.c
	$(CC) $(CFLGS) damage.c
destruct.o:	trek.h destruct.c
	$(CC) $(CFLGS) destruct.c
dock.o:	trek.h dock.c
	$(CC) $(CFLGS) dock.c
dump.o:	trek.h dump.c
	$(CC) $(CFLGS) dump.c
events.o:	trek.h events.c
	$(CC) $(CFLGS) events.c
externs.o:	trek.h externs.c
	$(CC) $(CFLGS) externs.c
extra.o:	trek.h extra.c
	$(CC) $(CFLGS) extra.c
getcodi.o:	trek.h getcodi.c
	$(CC) $(CFLGS) getcodi.c
getpar.o:	trek.h getpar.c
	$(CC) $(CFLGS) getpar.c
help.o:	trek.h help.c
	$(CC) $(CFLGS) help.c
impulse.o:	trek.h impulse.c
	$(CC) $(CFLGS) impulse.c
input.o:	trek.h input.c
	$(CC) $(CFLGS) input.c
initquad.o:	trek.h initquad.c
	$(CC) $(CFLGS) initquad.c
kill.o:	trek.h kill.c
	$(CC) $(CFLGS) kill.c
klmove.o:	trek.h klmove.c
	$(CC) $(CFLGS) klmove.c
log.o:	trek.h log.c
	$(CC) $(CFLGS) log.c
lose.o:	trek.h lose.c
	$(CC) $(CFLGS) lose.c
lrscan.o:	trek.h lrscan.c
	$(CC) $(CFLGS) lrscan.c
main.o:	trek.h main.c
	$(CC) $(CFLGS) main.c
move.o:	trek.h move.c
	$(CC) $(CFLGS) move.c
nova.o:	trek.h nova.c
	$(CC) $(CFLGS) nova.c
phaser.o:	trek.h phaser.c
	$(CC) $(CFLGS) phaser.c
play.o:	trek.h play.c
	$(CC) $(CFLGS) play.c
query.o:	trek.h query.c
	$(CC) $(CFLGS) query.c
ram.o:	trek.h ram.c
	$(CC) $(CFLGS) ram.c
ranf.o:	trek.h ranf.c
	$(CC) $(CFLGS) ranf.c
report.o:	trek.h report.c
	$(CC) $(CFLGS) report.c
reset.o:	trek.h reset.c
	$(CC) $(CFLGS) reset.c
rest.o:	trek.h rest.c
	$(CC) $(CFLGS) rest.c
schedule.o:	trek.h schedule.c
	$(CC) $(CFLGS) schedule.c
score.o:	trek.h score.c
	$(CC) $(CFLGS) score.c
setup.o:	trek.h setup.c
	$(CC) $(CFLGS) setup.c
setwarp.o:	trek.h setwarp.c
	$(CC) $(CFLGS) setwarp.c
shell.o:	trek.h shell.c
	$(CC) $(CFLGS) shell.c
shield.o:	trek.h shield.c
	$(CC) $(CFLGS) shield.c
snova.o:	trek.h snova.c
	$(CC) $(CFLGS) snova.c
srscan.o:	trek.h srscan.c
	$(CC) $(CFLGS) srscan.c
ssradio.o:	trek.h ssradio.c
	$(CC) $(CFLGS) ssradio.c
torped.o:	trek.h torped.c
	$(CC) $(CFLGS) torped.c
visual.o:	trek.h visual.c
	$(CC) $(CFLGS) visual.c
warp.o:	trek.h warp.c
	$(CC) $(CFLGS) warp.c
win.o:	trek.h win.c
	$(CC) $(CFLGS) win.c

install: all
	cp trk /usr/games

clean:
	rm -f *.o

clobber: clean
	rm -f trk

lint:
	lint -abchx *.c
