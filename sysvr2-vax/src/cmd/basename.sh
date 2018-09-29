#	@(#)basename.sh	1.4
AQQQ=${1-.}
AQQQ=`expr //$AQQQ : "\(.*\)\/$" \| $AQQQ`
BQQQ=`expr //$AQQQ : '.*/\(.*\)'`
expr $BQQQ : "\(.*\)$2$" \| $BQQQ
