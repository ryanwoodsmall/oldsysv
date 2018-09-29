#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)graf:src/whatis.d/Makefile	1.5"
###
### Standard Constants 
###

ROOT     =
LROOT    =
BIN      = $(ROOT)/usr/bin/graf
CC       = cc
CFLAGS   = -c -O $(FFLAG)

###
### Local Constants
###

HELP     = abs       af        bar       bel       bucket    ceil    \
           cor       cusum     cvrtopt   dtoc      erase     exp     \
           floor     gamma     gas       gd        ged       gtop    \
           hardcopy  hilo      hist      hpd       label     list    \
           log       lreg      mean      mod       pair      pd      \
           pie       plot      point     power     prime     prod    \
           ptog      qsort     quit      rand      rank      remcom  \
           root      round     siline    sin       subset    td      \
           tekset    title     total     ttoc      var       vtoc    \
           whatis    yoo

USRLIB   = $(ROOT)/$(LROOT)/usr/lib/graf/whatis

###### Dependencies

all:
	for i in $(HELP) ; do cp $$i.w $$i ; chmod 644 $$i ; done

install      : $(HELP)
	
abs          : abs.w;         -cp abs.w abs; chmod 644 abs; cpset abs $(USRLIB)
af           : af.w;          -cp af.w af; chmod 644 af; cpset af $(USRLIB)
bar          : bar.w;         -cp bar.w bar; chmod 644 bar; cpset bar $(USRLIB)
bel          : bel.w;         -cp bel.w bel; chmod 644 bel; cpset bel $(USRLIB)
bucket       : bucket.w;      -cp bucket.w bucket; chmod 644 bucket; cpset bucket $(USRLIB)
ceil         : ceil.w;        -cp ceil.w ceil; chmod 644 ceil; cpset ceil $(USRLIB)
cor          : cor.w;         -cp cor.w cor; chmod 644 cor; cpset cor $(USRLIB)
cusum        : cusum.w;       -cp cusum.w cusum; chmod 644 cusum; cpset cusum $(USRLIB)
cvrtopt      : cvrtopt.w;     -cp cvrtopt.w cvrtopt; chmod 644 cvrtopt; cpset cvrtopt $(USRLIB)
dtoc         : dtoc.w;        -cp dtoc.w dtoc; chmod 644 dtoc; cpset dtoc $(USRLIB)
erase        : erase.w;       -cp erase.w erase; chmod 644 erase; cpset erase $(USRLIB)
exp          : exp.w;         -cp exp.w exp; chmod 644 exp; cpset exp $(USRLIB)
floor        : floor.w;       -cp floor.w floor; chmod 644 floor; cpset floor $(USRLIB)
gamma        : gamma.w;       -cp gamma.w gamma; chmod 644 gamma; cpset gamma $(USRLIB)
gas          : gas.w;         -cp gas.w gas; chmod 644 gas; cpset gas $(USRLIB)
gd           : gd.w;          -cp gd.w gd; chmod 644 gd; cpset gd $(USRLIB)
ged          : ged.w;         -cp ged.w ged; chmod 644 ged; cpset ged $(USRLIB)
gtop         : gtop.w;        -cp gtop.w gtop; chmod 644 gtop; cpset gtop $(USRLIB)
hardcopy     : hardcopy.w;    -cp hardcopy.w hardcopy; chmod 644 hardcopy; cpset hardcopy $(USRLIB)
hilo         : hilo.w;        -cp hilo.w hilo; chmod 644 hilo; cpset hilo $(USRLIB)
hist         : hist.w;        -cp hist.w hist; chmod 644 hist; cpset hist $(USRLIB)
hpd          : hpd.w;         -cp hpd.w hpd; chmod 644 hpd; cpset hpd $(USRLIB)
label        : label.w;       -cp label.w label; chmod 644 label; cpset label $(USRLIB)
list         : list.w;        -cp list.w list; chmod 644 list; cpset list $(USRLIB)
log          : log.w;         -cp log.w log; chmod 644 log; cpset log $(USRLIB)
lreg         : lreg.w;        -cp lreg.w lreg; chmod 644 lreg; cpset lreg $(USRLIB)
mean         : mean.w;        -cp mean.w mean; chmod 644 mean; cpset mean $(USRLIB)
mod          : mod.w;         -cp mod.w mod; chmod 644 mod; cpset mod $(USRLIB)
pair         : pair.w;        -cp pair.w pair; chmod 644 pair; cpset pair $(USRLIB)
pd           : pd.w;          -cp pd.w pd; chmod 644 pd; cpset pd $(USRLIB)
pie          : pie.w;         -cp pie.w pie; chmod 644 pie; cpset pie $(USRLIB)
plot         : plot.w;        -cp plot.w plot; chmod 644 plot; cpset plot $(USRLIB)
point        : point.w;       -cp point.w point; chmod 644 point; cpset point $(USRLIB)
power        : power.w;       -cp power.w power; chmod 644 power; cpset power $(USRLIB)
prime        : prime.w;       -cp prime.w prime; chmod 644 prime; cpset prime $(USRLIB)
prod         : prod.w;        -cp prod.w prod; chmod 644 prod; cpset prod $(USRLIB)
ptog         : ptog.w;        -cp ptog.w ptog; chmod 644 ptog; cpset ptog $(USRLIB)
qsort        : qsort.w;       -cp qsort.w qsort; chmod 644 qsort; cpset qsort $(USRLIB)
quit         : quit.w;        -cp quit.w quit; chmod 644 quit; cpset quit $(USRLIB)
rand         : rand.w;        -cp rand.w rand; chmod 644 rand; cpset rand $(USRLIB)
rank         : rank.w;        -cp rank.w rank; chmod 644 rank; cpset rank $(USRLIB)
remcom       : remcom.w;      -cp remcom.w remcom; chmod 644 remcom; cpset remcom $(USRLIB)
root         : root.w;        -cp root.w root; chmod 644 root; cpset root $(USRLIB)
round        : round.w;       -cp round.w round; chmod 644 round; cpset round $(USRLIB)
siline       : siline.w;      -cp siline.w siline; chmod 644 siline; cpset siline $(USRLIB)
sin          : sin.w;         -cp sin.w sin; chmod 644 sin; cpset sin $(USRLIB)
subset       : subset.w;      -cp subset.w subset; chmod 644 subset; cpset subset $(USRLIB)
td           : td.w;          -cp td.w td; chmod 644 td; cpset td $(USRLIB)
tekset       : tekset.w;      -cp tekset.w tekset; chmod 644 tekset; cpset tekset $(USRLIB)
title        : title.w;       -cp title.w title; chmod 644 title; cpset title $(USRLIB)
total        : total.w;       -cp total.w total; chmod 644 total; cpset total $(USRLIB)
ttoc         : ttoc.w;        -cp ttoc.w ttoc; chmod 644 ttoc; cpset ttoc $(USRLIB)
var          : var.w;         -cp var.w var; chmod 644 var; cpset var $(USRLIB)
vtoc         : vtoc.w;        -cp vtoc.w vtoc; chmod 644 vtoc; cpset vtoc $(USRLIB)
whatis       : whatis.w;      -cp whatis.w whatis; chmod 644 whatis; cpset whatis $(USRLIB)
yoo          : yoo.w;         -cp yoo.w yoo; chmod 644 yoo; cpset yoo $(USRLIB)

#######

clobber      :  ;             -rm -f $(HELP)

clean        :

#######

listings    :
	
