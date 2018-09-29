#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./bcheckrc.sh	1.3"

echo "
# ***** This file has those commands necessary to check the file
# system, date, and anything else that should be done before mounting
# the file systems.


#!	chmod +x \${file}
" >bcheckrc
if u3b2
then echo "
rootfs=/dev/root
/etc/fsstat \${rootfs}  >/dev/null 2>&1

if [ \$? -ne 0 ]
then
	echo \"
	fsstat: root file system needs checking
        The root file system (\${rootfs}) is being checked automatically.\"
	/etc/fsck -y -D -b \${rootfs}
fi
" >>bcheckrc
else echo "
trap \"\" 2

# **** Check date

while :
do
	echo \"Is the date \`date\` correct? (y or n) \c\"
	read reply
	if
		[ \"\$reply\" = y ]
	then
		break
	else
		echo \"Enter the correct date:  \\c\"
		read reply
		date \"\$reply\"
	fi
done

# ***** Auto check, if necessary

while :
do
	echo \"Do you want to check the file systems? (y or n) \\c\"
	read reply
	case \"\$reply\" in
		 y )
			;;

		n* )
			break
			;;

		* )
			echo \"Invalid input. Try again.\"
			continue
			;;
	esac
	trap \"echo Interrupt\" 2
	/etc/fsck
	trap \"\" 2
	break
done
" >>bcheckrc
fi
