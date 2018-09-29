#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)dirname:dirname.sh	1.5"
if [ $# -gt 1 ]
then
	echo >&2 "Usage: dirname [ path ]"
	exit 1
fi
#	First check for pathnames of form //*non-slash*/* in which case the 
#	dirname is /.
#	Otherwise, remove the last component in the pathname and slashes 
#	that come before it.
#	If nothing is left, dirname is "."
exec /bin/expr \
	"${1:-.}/" : '\(/\)/*[^/]*//*$'  \| \
	"${1:-.}/" : '\(.*[^/]\)//*[^/][^/]*//*$' \| \
	.
