#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# Copyright  (c) 1986 AT&T
#	All Rights Reserved
#
#ident	"@(#)vm.vinstall:src/vinstall/vmsetup.sh	1.1"
#
#       program to initially set up and customize
#       a new Viewmaster user's home environment

CREATE=no
ERROR=
USAGE="Usage:  $0 [ -a -c -s -m ] -u login"

set -- `getopt csamu: $* 2> /dev/null`
if [ $? -ne 0 ]
then
	echo $USAGE
	ERROR=1
	exit
fi
for i in $*
do
	case $i in
	   -c)	# create login on a 3b2
		shift
		if (u3b2) 2> /dev/null		# only allow if on a 3b2
		then
			CREATE=yes
		fi
		;;
	   -m)	# turn on flag to send mail to $LOGNAME when finished
		shift
		MAIL=yes
		;;
	   -u)	# get user login
		LOGINID=$2
		shift 2
		;;
	  --)
		shift
		break
		;;
	esac
done
if [ -z "$LOGINID" ]
then
	echo $USAGE
	ERROR=1
	exit
fi


VMSYS=`sed -n -e '/^vmsys:/s/^.*:\([^:][^:]*\):[^:]*$/\1/p' < /etc/passwd`
if [ ! -d "${VMSYS}" ]
then
	echo "The value for VMSYS is not set."
	ERROR=1
	exit
fi

##### INITIALIZE VARIABLES #####

for i in standard 
do
	if [ ! -d ${VMSYS}/${i} ]
	then
		echo "\nCannot access $VMSYS/$i!!"
		ERROR=1
		exit
	fi
done


trap    "
	ERROR=1
	exit
" 1 2 3 15 16

# get passed variables from newuser
# GROUPID group id number of the new user

GRPNAME=`grep -s "^$LOGINID:" /etc/passwd 2> /dev/null | cut -f4 -d: `
if [ -z "${GRPNAME}" ]
then 
	echo "\n$LOGINID's group is not in /etc/group.\n"
	ERROR=1
	exit
fi
UHOME=`grep -s "^$LOGINID:" /etc/passwd | cut -f6 -d:`
if [ -z "${UHOME}" ]
then 
	echo "\n${LOGINID}'s home directory has not\nbeen retrieved correctly."
	ERROR=1
	exit
fi

filetype=standard
cd $VMSYS/${filetype}

##################################################################
# 	Move files to real Viewmaster home directory, set 
#   permissions

# ignore all signals

ALLFILES=/tmp/allfiles$$
ALLDIRS=/tmp/alldirs$$

trap "" 1 2 3 15 16
find . \( -type f ! -name \. \) -print | sed -e "s/^.\///" > $ALLFILES
find . \( -type d ! -name \. \) -print | sed -e "s/^.\///" > $ALLDIRS

cat "$ALLFILES" | while read i
do
	if [ -f "$UHOME"/"$i" ]
	then
		dname=`dirname $i`
		bname=`basename $i`
		if [ "$dname" = "." ]
		then
			backup=old${bname}
		else
			backup=${dname}/old${bname}
		fi
		mv $UHOME/$i $UHOME/$backup 2> /dev/null
	fi
done

# Copy in files to user's $HOME
cat $ALLDIRS $ALLFILES | cpio -pdum $UHOME 2>/dev/null 
cd "$UHOME"
rm -f temp.profile
cat $ALLFILES $ALLDIRS | while read i
do
	chown $LOGINID $i 2>/dev/null
	NEWOWN=$LOGINID
	NEWGROUP=$GRPNAME
	NEWMODE=755
	chmod $NEWMODE $UHOME/$i 2> /dev/null && chgrp $NEWGROUP $UHOME/$i 2> /dev/null && chown $NEWOWN $UHOME/$i 2> /dev/null
	if [ $? != 0 ]
	then
		echo "Could not set correct permissions on $i." >> $ERRORFILE
	fi
done

if [ "$?" -eq "10" ]	
then
	ERROR=1
	exit
fi
if [ -s "$ERRORFILE" ]
then
	cat <<- EOF

		NOTE:
		The following is a list of directories and/or
		files whose protections could not be modified.
		Please make a note of those that need correcting!!

		`pr -to2 $ERRORFILE`
	EOF
	ERROR=1
fi
rm -f $ALLFILES $ALLDIRS
exit
