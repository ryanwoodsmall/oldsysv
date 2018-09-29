#	MAN command.    SCCS @(#)man.sh	1.53
if test $# = 0;  	then help text2 >&2;  exit 1;  fi
trap 'trap 0; if test "$m" = 0; then mesg -y; fi; exit $z' 0
trap 'trap 0; if test "$m" = 0; then mesg -y; fi; exit 1' 1 2 3 15
if test -t;  then mesg >/dev/null;  m=$?;  mesg -n; else m=1; fi
ec=/usr/pub/eqnchar
PATH=$PATH:/bin:/usr/bin;  y=0;  tbl="tbl";  u="-can";
sec=\?;  mdir=0
ptty=  o="|daps"  x=  Teqn=
	# If GCOS, set o="|apsend"
	# If on-line typesetter, set o="|daps"
cmd="n" fil= opt= i= all= eopt= a= c=
for i
do case $i in
	[1-8])	sec=$i ;;
	-s)	if test "$cmd" = "n";  then cmd=t;  fi
		opt="$opt -rs1";  eopt=-s9 ;;
	-t)	cmd=t ;;
	-Tcat)	cmd=t  ptty=cat  Teqn=-Tcat  ec=/usr/pub/cateqnchar  o=  ;;
	-Taps)	cmd=t  ptty=aps  Teqn=-Taps  ec=/usr/pub/apseqnchar  ;;
	-Ti10)	cmd=t  ptty=i10  Teqn=-Ti10  o="|di10"  x=  ;;
	-TX97.*)	cmd=t  ptty=`echo $i | sed "s/-T//"`
		Teqn=-TX97  o="|dx9700"  x=$ptty  ;;
	-Di10)	cmd=t  o="|di10"  x=  ;;
	-D4014|-Dtek|-T4014|-Ttek)	cmd=k ;;
	-c)	c=c ;;
	-12)	y=1;;
	-d)	mdir=1 ;;
	-w)	cmd=w ;;
	-y)	u="-man" ;;
	-z)	cmd=t  o=  x=  ;;
	-T*)	TERM=`echo $i | sed "s/-T//"`
		ptty=$TERM  o=  x= ;;
	-*)	opt="$opt $i" ;;
	*)	if test "$mdir" = 0
		then
			cd /usr/man
			fil=`find */man$sec/$i.* -print 2>/dev/null`
			if test -n "$fil"
			   then all="$all $fil"
			   else echo man: $i not found >&2
				exit 1
			fi
		else
			if test ! -r "$i"
			then
				echo man: $i not found >&2
				exit 1
			else
				all="$all $i"
			fi
		fi
   esac
done
if test "$cmd" = "w";  then echo $all;  z=0;  exit;  fi
if test "$cmd" = n
then
	v=0  h="-h"  g=
	if test "$TERM" = "";  then TERM=450; fi
	case "$TERM" in
		300|300s|450|37|300-12|300s-12|450-12|4000a|382)	;;
		X)	v=4;;
		X97)	g="|xerox"; v=4;;
		4014|tek)	g="|4014" ;;
		1620)	TERM=450 ;;
		1620-12)	TERM=450-12 ;;
		hp|2621|2640|2645)
			v=2;  c=c;  a="-u1 $a";  g="|hp -m";  TERM=hp ;;
		735|745|40/4|40/2)	v=2;  c=c ;;
		43)	v=2;  c=c;  opt="$opt -rT1" ;;
		2631|2631-c|2631-e)	v=3;  c=c ;;
		*)	TERM=lp;  v=2;  c=c ;;
	esac
	if test $y = 1 -a \( $TERM = 300 -o $TERM = 300s -o $TERM = 450 \)
		then TERM="$TERM"-12
	fi
	if test "$c" = c
	then
		case "$TERM" in
			300|300s|450|300-12|300s-12|450-12|4014|tek)
				g="|col -f|greek -T$TERM" ;;
			37|4000a|382)	g="|col -f" ;;
			X)	g="|col -f"; v=5;;
			X97)	g="|col -f|xerox"; v=5;;
			hp)	g="|col|hp -m" ;;
			2631-c|2631-e)	g="|col -p" ;;
			735|745|43)	g="|col -x" ;;
			40/4|40/2)	g="|col -b" ;;
			lp|2631)	g="|col" ;;
		esac
		h=
		if test "$v" = 0;  then v=1;  fi
	fi
	if test "$v" -gt 1;  then tbl="tbl -TX";  fi
	if test "$TERM" = 4014 -o "$TERM" = hp;  then v=1;  fi
	case "$v" in
		1|5)	TERM=37;;
		2)	TERM=lp;;
	esac
else
	if test "cat" = "$ptty"
	then if test -n "$o$x"
		then echo "$0: invalid multiple -T options" >&2;  exit 1;
		else troff=otroff
		fi
	else if test -n "$ptty"
		then troff="troff -T$ptty"
		else troff=troff
		fi
	fi
fi
for fil in $all
do
	ln=`line < $fil`
	ck=`echo $ln | sed "s/ .*//"`
	if test "$ck" = ".so"
	then
		rfil=`echo $ln | sed "s/.so //"`
	else
		rfil=$fil
	fi
	prep="cat $rfil"
	ln=`line < $rfil`
	ck=`echo $ln | sed "s/ .*//"`
	if test "$ck" = "'\\\""
	then
		case `echo $ln | sed "s/....//"` in
		e | ce | ec)
			if test "$cmd" != n
			then
				prep="eqn $Teqn $eopt $ec $rfil"
			else
				prep="neqn $ec $rfil"
			fi ;;
		t | ct | tc)
			prep="$tbl $rfil" ;;
		et | te | cet | cte | ect | etc | tce | tec)
			if test "$cmd" != n
			then
				prep="$tbl $rfil | eqn $Teqn $eopt $ec -"
			else
				prep="$tbl $rfil | neqn $ec -"
			fi ;;
		esac
	fi
	d=`/usr/lib/manprog $rfil`
	if test "|apsend" = "$o";  then x="$x c=$fil";  fi
	case $cmd in
		n)	eval "$prep | nroff -T$TERM $d$opt $h $a $u $g" ;;
		t)	eval "$prep | $troff $d$opt $u $o $x" ;;
		k)	eval "$prep | $troff $d $opt $u | tc" ;;
	esac
done
z=0;  exit
