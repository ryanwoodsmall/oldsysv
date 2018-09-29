#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)help:defnlen.sh	1.3"

trap 'rm -f /tmp/$$LCK.* ; rm -f /tmp/LCK.${term} 2>/dev/null ; rm -f /tmp/*$$ ; trap 0 ; exit' 0 1 2 3 15
while [ "${LEN}" = "bad" ]
do
	${EDITOR} ${EDFILE}
	if [ `wc -l <${EDFILE}` -le ${SCRNLEN} ]
	then
		ANS=~
		while [ \( "${ANS}" != "y" \) -a \( "${ANS}" != "n" \) ]
		do
			echo "Are you satisfied with this definition (y or n)? > \c"
			read ANS
		done
		if [ "${ANS}" = "y" ]
		then
			LEN=good
		else
			echo "Re-editing definition for ${term}"
		fi
	else
		echo "This definition is more than ${SCRNLEN} lines long."
		echo "Re-editing definition for ${term}."
	fi
done
