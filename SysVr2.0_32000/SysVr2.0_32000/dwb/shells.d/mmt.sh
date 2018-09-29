#	MMTSID (@(#)mmt.sh	1.23)
u=-cm;  if echo `basename $0` | grep mv >/dev/null;  then u=-mv;  fi
if test $# = 0;  then help text1 >&2;  exit 1;  fi
ptty=  x=  Teqn=  ec=/usr/pub/eqnchar
PATH=$PATH:/bin:/usr/bin  O=  o="|daps"  y=
#	'If phototypesetter connected to unix, use o="|daps"'
#	'If sending to MHCC use o="|apsend"'
a= e= f= p= z=
while test -n "$1" -a ! -r "$1"
do case $1 in
	-a)	O="-a"  o=  x=  ;;
	-Taps)	ptty=aps  Teqn=-Taps  ec=/usr/pub/apseqnchar  ;;
	-Tcat)	ptty=cat  Teqn=-Tcat  ec=/usr/pub/cateqnchar  o=  ;;
	-Ti10)	ptty=i10  Teqn=-Ti10  o="|di10"  x=  ;;
	-TX97*)	ptty=`echo $1 | sed "s/-T//"`
		Teqn=-TX97  o="|dx9700"  x=$ptty  ;;
	-Di10)	o="|di10"  x=  ;;
	-D4014|-Dtek|-T4014|-Ttek)	o="|tc"  x=  y="-rX1" ;;
	-T*)	ptty=`echo $1 | sed "s/-T//"`
		o=  x=  ;;
	-e)	e=eqn ;;
	-t)	f=tbl ;;
	-p)	p=pic ;;
	-y)	if echo `basename $0` | grep -v mv >/dev/null
		    then u=-mm
		fi;;
	-)	break ;;
	-z)	z=z  ;;
	*)	a="$a $1" ;;
   esac
   shift
done
if test -z "$1";  then echo "$0: no input file" >&2;  exit 1;  fi
if test "cat" = "$ptty"
then if test -n "$o$x"
	then echo "$0: invalid multiple -T options" >&2;  exit 1;
	else troff="otroff"
	fi
else if test -n "$ptty"
	then troff="troff -T$ptty"
	else troff=troff
	fi
fi
if test -n "$z";  then o=  x=;  fi
if test "|apsend" = "$o";  then x="$x c=$1";  fi
d="$*"
if test -n "$p";  then p="pic $Teqn $d|";  d="-";  fi
if test -n "$f";  then f="tbl $d|";  d="-";  fi
if test -n "$e";  then e="eqn $Teqn $ec $d|";  d="-";  fi
if test "$u" != "-mv";  then y=;  fi
eval "$p $f $e $troff $y $O $u $a $d $o $x"; exit 0
