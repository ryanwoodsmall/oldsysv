#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)help:keysrch.sh	1.3"

echo "\nCommands found using \\S${*}\\S:\n"
for word
do
	line="${line}`grep \"^key='${word}'	\" ${KEYWORDS_TABLE}`"
	if [ $? -eq 0 ]
	then
		line="${line};"
	fi
done
if [ -z "${line}" ]
then
	echo "\n\n\n               No commands found using \\S ${*} \\S\n"
else
	eval "${line}" | sort -u
fi
