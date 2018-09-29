#	macref - macro package cross referencer (for nroff/troff files)
#
#  SCCS:  @(#)macref.sh	1.1
#
#


trap "rm -f /tmp/mc$$;exit 1" 2 3 4 5 6 7 8 10 12 13
DIR=/usr/lib/macref
MAIN=yes
for i in $*
do case $i in
	-t) TOC=yes; shift;;
	-s) SUM=yes; shift;;
	-n) MAIN=no; shift;;
	-*) echo unknown option: $i
	    while [ $# -gt 0 ]
		do shift
		done;;
	*)  break;;
esac
done
if [ $# -eq 0 ]
    then echo "usage:  macref [-n] [-s] [-t] file ..."
	 exit 1
fi

if [ ! -r "$1" ]
    then echo $1 not found; exit 1
    elif [ -z "$1" ]
    then echo no input file; exit 1
fi
sno $DIR/macref.sno $* </dev/null | sort > /tmp/mc$$
if [ "$TOC" = "yes" ]
	then sno $DIR/macrtoc.sno < /tmp/mc$$ | pr -4 -t
	echo ""
fi
if [ "$SUM" = "yes" ]
	then sno $DIR/macrstat.sno < /tmp/mc$$
	echo ""
fi
if [ "$MAIN" = "yes" ]
	then sno $DIR/macrform.sno < /tmp/mc$$ | newform
fi

rm -f /tmp/mc$$
