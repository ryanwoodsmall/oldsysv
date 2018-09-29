# Creates in $TMPDIR a file called M.folio that can be
#  run off by typing:   ``mm -t -rN2 -T450-12 M.folio''.

set x $*
. ./.param

trap "rm -f $TMPDIR/folio*; exit" 0 1 2 3 9 15

case `basename $MANDIR` in
u_man)
cat > $TMPDIR/M.folio <<!
.OP
.tl '''May 15, 1982'
.sp 2
.ce
UNIX System User's Manual
.ce
Release 5.0
.tr ~
.sp 2
.ce
.ul
PRINTING INSTRUCTIONS
.sp 2
Print entire manual two-sided; mock-ups of front and back
cover (together with sizing information), as well as cover
title and tabs copy are attached.
.sp
Total page count (including blank pages) is 750.
.nf
.sp 2
.ce
.I "FOLIO LIST"
.sp 2
.I "Front Matter:"
.sp
.ta 5 10 25 34
.ul
	No tab:
		Title page			~~~~1*
		Trademark page			~~~~2
.FS *
Page 1 is
.I not
numbered.
.I All
other pages
.I "that carry any text"
are numbered by entry and should
be placed back-to-back, regardless of odd or even number, with
the exception of new "Tab" divisions.
.FE
.sp
.ul
	Tab "INTRO":
		Introduction			~~~~3-4 incl.
		How To Get Started			~~~~5-8 incl.
.sp
.ul
	Tab "CONTENTS":
		Contents			~~~~1-9 incl.
		     Blank page	~~~~10
.sp
.ul
	Tab "INDEX":
		Permuted Index		~~~~1-37 incl.
		     Blank page	~~~~38
.bp
.nf
.sp 2
.ul
Tab "1":
'\"	Section 1 goes here
.TS H
l1 l| l1 l| l1 l.
ENTRY	PAGES	ENTRY	PAGES	ENTRY	PAGES
_
.TH
.so $TMPDIR/folio1.input
.TE
.sp 2
.ul
Tab "2":
'\"	Section 2 goes here
.TS H
l1 l| l1 l| l1 l.
ENTRY	PAGES	ENTRY	PAGES	ENTRY	PAGES
_
.TH
.so $TMPDIR/folio2.input
.TE
.bp
.sp 2
.ul
Tab "3":
'\"	Section 3 goes here
.TS H
l1 l| l1 l| l1 l.
ENTRY	PAGES	ENTRY	PAGES	ENTRY	PAGES
_
.TH
.so $TMPDIR/folio3.input
.TE
.bp
.sp 2
.ul
Tab "4":
'\"	Section 4 goes here
.TS H
l1 l| l1 l| l1 l.
ENTRY	PAGES	ENTRY	PAGES	ENTRY	PAGES
_
.TH
.so $TMPDIR/folio4.input
.TE
.sp 2
.ul
Tab "5":
'\"	Section 5 goes here
.TS H
l1 l| l1 l| l1 l.
ENTRY	PAGES	ENTRY	PAGES	ENTRY	PAGES
_
.TH
.so $TMPDIR/folio5.input
.TE
.sp 2
.ul
Tab "6":
'\"	Section 6 goes here
.TS H
l1 l| l1 l| l1 l.
ENTRY	PAGES	ENTRY	PAGES	ENTRY	PAGES
_
.TH
.so $TMPDIR/folio6.input
.TE
!
	;;
a_man)
cat > $TMPDIR/M.folio <<!
.OP
.tl '''May 15, 1982'
.sp 2
.ce
UNIX System Administrator's Manual
.ce
Release 5.0
.tr ~
.sp 2
.ce
.ul
PRINTING INSTRUCTIONS
.sp 2
Print entire manual two-sided; mock-ups of front and back
cover (together with sizing information), as well as cover
title and tabs copy are attached.
.sp
Total page count (including blank pages) is 308.
.nf
.sp 2
.ce
.I "FOLIO LIST"
.sp 2
.I "Front Matter:"
.sp
.ta 5 10 25 34
.ul
	No tab:
		Title page			~~~~1*
		Trademark page			~~~~2
.FS *
Page 1 is
.I not
numbered.
.I All
other pages
.I "that carry any text"
are numbered by entry and should
be placed back-to-back, regardless of odd or even number, with
the exception of new "Tab" divisions.
.FE
.sp
.ul
	Tab "INTRO":
		Introduction			~~~~3-4 incl.
.sp
.ul
	Tab "CONTENTS":
		Contents			~~~~1-4 incl.
.sp
.ul
	Tab "INDEX":
		Permuted Index		~~~~1-49 incl.
		     Blank page ~~~~50
.bp
.nf
.sp 2
.ul
Tab "1":
'\"	Section 1 goes here
.TS H
l1 l| l1 l| l1 l.
ENTRY	PAGES	ENTRY	PAGES	ENTRY	PAGES
_
.TH
.so $TMPDIR/folio1.input
.TE
.sp 2
.br
.ne 25v
.ul
Tab "7":
'\"	Section 7 goes here
.TS H
l1 l| l1 l| l1 l.
ENTRY	PAGES	ENTRY	PAGES	ENTRY	PAGES
_
.TH
.so $TMPDIR/folio7.input
.TE
.sp 2
.ul
Tab "8":
'\"	Section 8 goes here
.TS H
l1 l| l1 l| l1 l.
ENTRY	PAGES	ENTRY	PAGES	ENTRY	PAGES
_
.TH
.so $TMPDIR/folio8.input
.TE
!
	;;
*)
	echo "$0: Unknown manual directory"
	exit 1
	;;
esac

if [ ! -f $TMPDIR/totalpgs ]; then
	echo "$0: $TOOLS/pgcnt has not been run!"
	exit 1
fi

for sec in $DIRLST
do
	if [ ! -f $TMPDIR/pages${sec} ]; then
		echo "$0: $TOOLS/pgcnt has not been run!"
		exit 1
	fi

	cat $TMPDIR/pages${sec} | tr [a-z] [A-Z] > $TMPDIR/folio${sec}

	ed - $TMPDIR/folio${sec} <<-!
	1,2d
	/INTRO/m0
	g/\.${sec}/s/\.${sec}/(${sec}/
	g/(${sec}/s/	/)	/
	g/	\([^1]\)$/s//	1-\1 incl./
	g/	\([1-9][0-9]\)$/s//	1-\1 incl./
	g/	/s//@/
	w
	q
	!

	total=`grep "total pages Section ${sec}" $TMPDIR/totalpgs | \
		sed 's/.* //'`
	case "$total" in
		*[02468])	;;
		*[13579])	echo "blank page@1" >> $TMPDIR/folio${sec};;
		*)		echo "$0: Invalid output from totalpgs";;
	esac

	pr -t -s@ -w80 -l1000 -3 $TMPDIR/folio${sec} | \
		 tr "@" "\011" > $TMPDIR/folio${sec}.input
done

case `basename $MANDIR` in
u_man)
	ed - $TMPDIR/M.folio <<-!
	/\.so .*folio1.input/d
	-r $TMPDIR/folio1.input
	/\.so .*folio2.input/d
	-r $TMPDIR/folio2.input
	/\.so .*folio3.input/d
	-r $TMPDIR/folio3.input
	/\.so .*folio4.input/d
	-r $TMPDIR/folio4.input
	/\.so .*folio5.input/d
	-r $TMPDIR/folio5.input
	/\.so .*folio6.input/d
	-r $TMPDIR/folio6.input
	w
	q
	!
	;;
a_man)
	ed - $TMPDIR/M.folio <<-!
	/\.so .*folio1.input/d
	-r $TMPDIR/folio1.input
	/\.so .*folio7.input/d
	-r $TMPDIR/folio7.input
	/\.so .*folio8.input/d
	-r $TMPDIR/folio8.input
	w
	q
	!
	;;
esac
#	@(#)folio.mk	5.2 of 5/19/82
