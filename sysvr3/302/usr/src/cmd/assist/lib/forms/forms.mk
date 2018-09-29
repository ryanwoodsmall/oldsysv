#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)forms.files:forms.mk	1.27"

LIBDIR = ..
CH =

FILES = \
Batch.fs Cflist.fs Compare.fs Compile.fs Create.fs Debug.fs Directory.fs \
Edit.fs Electr.fs Find.fs Format.fs Look.fs Maint.fs Perm.fs \
Preproc.fs Prog.fs Setting.fs Sorting.fs Stats.fs System.fs TOP.fs ar.fs \
ar.val ar_d.fs ar_m.fs ar_p.fs ar_q.fs ar_r.fs ar_t.fs ar_x.fs assistwalk.fs \
awk.fs bfs.fs cal.fs cat.fs cc.fs cccomp.fs cctyp.fs cd.fs chmod.fs chmoda.fs \
chmods.fs chown.fs cmp.fs \
comm.fs cp.fs cpio.fs cpioi.fs cpioo.fs cpiop.fs crypt.fs csplit.fs cu.fs cut.fs \
d_help_c d_help_m d_help_p d_help_t date.fs dc.fs df.fs diff.fs dircmp.fs du.fs \
echo.fs ed.fs egrep.fs env.fs fgrep.fs file.fs find.fs g_help_c \
g_help_m g_help_p g_help_t grep.fs id.fs kill.fs ln.fs lp.fs lpstat.fs ls.fs \
lscomp.fs lstyp.fs mail.fs make.fs mesg.fs mkdir.fs mm.fs mmcomp.fs mmtyp.fs \
mv.fs nohup.fs nroff.fs od.fs passwd.fs paste.fs pg.fs pr.fs prcomp.fs prtyp.fs \
ps.fs pu_menu.fs pwd.fs rm.fs rmdir.fs sdb.fs sdbwalk.fs sed.fs sh.fs shell.lpstat \
size.fs sortc.fs sort.fs sorttyp.fs spell.fs split.fs strip.fs \
su.fs tail.fs taili.fs tailn.fs umask.fs uname.fs unixwalk.fs uucp.fs \
lex.fs yacc.fs ld.fs uucpl.fs uucpr.fs vi.fs viwalk.fs wc.fs who.fs \
stty.fs sttyrep.fs sttyset.fs 

install : $(FILES)
	-mkdir $(LIBDIR)/forms
	-cp $(FILES) $(LIBDIR)/forms
	$(CH)cd $(LIBDIR)/forms; chmod 444 $(FILES) 
	$(CH)cd $(LIBDIR)/forms; chgrp bin $(FILES); chown bin $(FILES)
	$(CH)chmod 755 $(LIBDIR)/forms; chgrp bin $(LIBDIR)/forms; chown bin $(LIBDIR)/forms

clobber :
	:
