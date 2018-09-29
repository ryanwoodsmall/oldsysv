# '@(#)dzbload.sh	6.1'
: usage: dzbload device
if
test -z "$1" -o -n "$2"
then
echo "dzbload: arg count"
else
if
kasb -d $1 <<!
.reset
!
then
sleep 10
kasb -d $1 <<!
.reset
.load "/lib/dzkmcb.o"
.reset
.run 3
!
echo "$1" "loaded with dzb code"
else
exit 2
fi
fi
