# '@(#)dzbload.sh	1.2'
: usage: dzbload device
if
test -z "$1" -o -n "$2"
then
echo "dzbload: arg count"
exit 2
else
if
kasb -d $1 <<!
.reset
.load "/lib/dzkmcb.o"
.reset
.run 3
!
then
echo "$1" "loaded with dzb code"
else
exit 2
fi
fi
