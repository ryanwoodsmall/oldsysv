#	@(#)runit.sh	5.2 of 5/14/82
# The argument to -rW is in basic units and should be:
#	-rW2052		for small (-rs1) troff (4.75i * 432)
#	-rW2808		for large troff & nroff (6.5i * 432)

mmt -rs1 -rW2052 errmacs errintro
mmt -t -rs1 -rW2052 errmacs errmess?
