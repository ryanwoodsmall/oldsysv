#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)help:glossary.sh	1.4"

cat <<!

\\Sglossary\\S:  UNIX System Terms & Symbols

       Enter any of these terms to obtain its definition.  Within each
  definition, important terms defined elsewhere in the \\Sglossary\\S are
  enclosed in double quotes ("") and footnoted on the bottom of the
  screen.
!
grep 'glossary:define' ${RESPONSE_TABLE} | \
	sed 's/:.*//' | sort -d | pr -a -t -2 -w79 - | pr -e -t
