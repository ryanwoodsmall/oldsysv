#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mvdir:mvdir.sh	1.3"
if [ $# != 2 ]
then
  echo "Usage: mvdir fromdir newname" 
  exit 2
fi
if [ $1 = . ]
then
	echo "mvdir: cannot move `.'"
	exit 2
fi
f=`basename $1`
t=$2
if [ -d $t ]
then
	t=$t/$f
fi
if [ -f $t  -o -d $t ]
then
  echo $t exists
  exit 1
fi
if [  ! -d $1 ]
then
  echo $1 must be a directory
  exit 1
fi

# *** common path tests: The full path name for $1 must not
# *** 			 be an anchored substring of the full
# ***			 path name for $2

here=`pwd`
cd $1
from=`pwd`
lfrom=`expr $from : $from`

cd $here
mkdir $t		# see if we can create the directory
if [ $? != 0 ]
then
	exit
fi
cd $t
to=`pwd`
cd $here
rmdir $t

a=`expr $to : $from`
if [ $a = $lfrom ]
then
  echo arguments have common path
  exit 1
fi
# ***

T=`expr $t : '\(.*\)/' \| .`
if /etc/link $1 $t ; then
	:
else
	echo "Cannot link to" $t 
	exit 2
fi
/etc/unlink $1
/etc/unlink $t/..
/etc/link $T $t/..
