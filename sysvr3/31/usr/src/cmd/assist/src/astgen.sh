#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)setup:astgen.sh	1.3"
export ASSISTBIN ASSISTLIB
ASSISTBIN=/a8/45414/hps/muse/bin
ASSISTLIB=/a8/45414/hps/muse/lib
exec $ASSISTBIN/tsetup $*
