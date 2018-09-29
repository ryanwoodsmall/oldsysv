################################################################################
#                         Copyright (c) 1985 AT&T                              #
#                           All Rights Reserved                                #
#                                                                              #
#                                                                              #
#          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 #
#        The copyright notice above does not evidence any actual               #
#        or intended publication of such source code.                          #
################################################################################
#	@(#)whodo.sh	1.4
: '/etc/whodo -- combines info from who(1) and ps(1)'
#     PATH defined to insure whodo works even if user redefines
#     standard commands

PATH=/bin:/usr/bin
trap  "rm -f /tmp/$$*" 1 2 3
ps -a>/tmp/$$b&
date
who -s >/tmp/$$a
ed - /tmp/$$a<<\!
g/^/s/^\(........\) *\(........\) *.......\(.*\)/\2 \1 \3/
w
!
uname
wait
ed - /tmp/$$b<<\!
1d
g/<defunct>/d
g/^/s/^\(......\) *\([^ ][^ ]*\)\(.*\)/\2-    \1    \3/
g/-sh$/s/-/+/
g/-rsh$/s/-/*/
w
!
sort /tmp/$$a /tmp/$$b >/tmp/$$c
ed - /tmp/$$c<<\!
g/^..+/s/.....//\
s/\([0-9]\)  *.*/\1/\
.-1,.j
g/^..\*/s/.....//\
s/\([0-9]\)  *.*/\1/\
.-1s/ /r/\
j
g/^\([^ ][^ ]*\)- /s//  \1/
1,$p
Q
!
rm -f /tmp/$$*
