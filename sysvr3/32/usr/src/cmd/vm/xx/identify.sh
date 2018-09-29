#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm:xx/identify.sh	1.1"
#
# This file contains a script in a simple language to identify terminals
# automatically.
#
# Note that special characters in "case" strings must be backslashed twice,
# since first the escape character handler processes them , then regcmp(3X).
# For more info on how to modify this file, see "Termtest: A Tool for the
# Automatic Identification of Terminals" by M. P. Lindner
#

# ANSI and ANSI-like terminals
query \E[c
case \E\\[?8;7;[23]*c
	print TERM=5620
	exit
case \E\\[?[78];[47];[0-9]*c
	# AT&T 600 series terminal
	query \E[c
	case \E\\[?7;4;.*
		print TERM=att610
		break
	case \E\\[?8;7;.*
		print TERM=att620
		break
	case \E\\[?8;4;.*
		print TERM=att615
		break
	end
	query \E[>0c
	case \E\\[>1;.*c
		print TERM=$TERM-103k
	end
	query \E[s
	case \E\\[[0-9]{10}2.*
		print COLUMNS=132
	end
	exit
case \E\\[?;[0-9]*;30;[0-9]*c
	print TERM=gs5430
	exit
# AT&T 4400 series terminals
case \E\\[?1;2;([0-9][0-9])$0;[0-9]*c
	print TERM=att44$0
	exit
case \E\\[?6c
	print TERM=510a
	exit
case \E\\[?;[0-9]*;5101;[0-9]*c\r
	print TERM=gs510A
	exit
case \E\\[?;[0-9]*;510;[0-9]*c
	print TERM=gs510D
	exit
case \E\\[?;[0-9]*;251;[0-9]*c
	print TERM=gs5425
	exit
# 6300 with AT&T emots color terminal emulator
case \E\\[?[0-9]*;[0-9]*;630[12];[0-9]*c
	print TERM=emots
	exit
case \E\\[?8c
	print TERM=tvi970
	exit
case \E\\[?1;[0-9]*c
	print TERM=vt100
	exit
case \E\\[=1;[12]c
	print TERM=avt
	exit
end

# AT&T 5420 with EPIC PROMS (obsolete)
query \E[F
case \E\\[1F
	print TERM=gs5420
	exit
end

# Wyse terminals
query \E\s
case ([0-9]+)$0^M
	print TERM=wy$0
	exit
end
