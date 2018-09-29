################################################################################
#                         Copyright (c) 1985 AT&T                              #
#                           All Rights Reserved                                #
#                                                                              #
#                                                                              #
#          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 #
#        The copyright notice above does not evidence any actual               #
#        or intended publication of such source code.                          #
################################################################################
#	@(#)prdaily.sh	1.5 of 7/22/83	#
#	"prdaily	prints daily report"
#	"last command executed in runacct"
#	"if given a date mmdd, will print that report"
PATH=/usr/lib/acct:/bin:/usr/bin:/etc

set -- `getopt cl $*`
if [ $? != 0 ]
then
	echo Usage: prdaily [-c] [-l] [mmdd]
	exit 2
fi
for i in $*
do
	case $i in
	-c)	CMDEXCPT=1; shift;;
	-l)	LINEEXCPT=1; shift;;
	--)	shift; break;;
	esac
done
date=`date +%m%d`
_sysname="`uname`"
_nite=/usr/adm/acct/nite
_lib=/usr/lib/acct
_sum=/usr/adm/acct/sum

cd ${_nite}
if [ `expr "$1" : [01][0-9][0-3][0-9]` -eq 4 -a "$1" != "$date" ]; then
	if [ "$CMDEXCPT" = "1" ]
	then
		echo "Cannot print command exception reports except for `date '+%h %d'`"
		exit 5
	fi
	if [ "$LINEEXCPT" = "1" ]
	then
		acctmerg -a < ${_sum}/tacct$1 | awk -f ${_lib}/ptelus.awk
		exit $?
	fi
	cat ${_sum}/rprt$1
	exit 0
fi

if [ "$CMDEXCPT" = 1 ]
then
	acctcms -a -s ${_sum}/daycms | awk -f ${_lib}/ptecms.awk
fi
if [ "$LINEEXCPT" = 1 ]
then
	acctmerg -a < ${_sum}/tacct${date} | awk -f ${_lib}/ptelus.awk
fi
if [ "$CMDEXCPT" = 1 -o "$LINEEXCPT" = 1 ]
then
	exit 0
fi
(cat reboots; echo ""; cat lineuse) | pr -h "DAILY REPORT FOR ${_sysname}"  

prtacct daytacct "DAILY USAGE REPORT FOR ${_sysname}"  
pr -h "DAILY COMMAND SUMMARY" daycms
pr -h "MONTHLY TOTAL COMMAND SUMMARY" cms 
pr -h "LAST LOGIN" -3 ../sum/loginlog  
exit 0
