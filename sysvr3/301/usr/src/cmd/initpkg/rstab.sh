#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:rstab.sh	1.1"
if u3b2
then echo "
#	place adv(1M) (advertise) commands here for automatic ececution
#	on entering init state 3.
#
#	adv [-r] [-d \"<text>\"] <resource> <directory> 
#	.e.g,
#	adv -d \"/usr/news\" NEWS /usr/news
" >rstab
fi
