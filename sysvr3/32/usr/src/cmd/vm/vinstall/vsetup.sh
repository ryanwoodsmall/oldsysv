#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)vm.vinstall:src/vinstall/vsetup.sh	1.7"

LOGINID=${1}
service=${2}
LOGDIR=`sed -n -e "/^${LOGINID}:/s/^.*:\([^:][^:]*\):[^:]*$/\1/p" < /etc/passwd`
if [ ! -d "${LOGDIR}" ]
then
	echo "${LOGINID}'s home directory doesn't exist"
	exit 1
fi

VMSYS=`sed -n -e '/^vmsys:/s/^.*:\([^:][^:]*\):[^:]*$/\1/p' < /etc/passwd`
if [ ! -d "${VMSYS}" ]
then
	echo "The value for VMSYS is not set."
	exit 1
fi

##### INITIALIZE VARIABLES #####

for i in standard 
do
	if [ ! -d ${VMSYS}/${i} ]
	then
		exit 1
	fi
done


trap    "
	exit 1
" 1 2 3 15 16

# get passed variables from newuser
# GROUPID group id number of the new user
UHOME=`grep -s "^$LOGINID:" /etc/passwd | cut -f6 -d:`
flag=false
if [ -z "${UHOME}" ]
then
	echo "\n${LOGNID}'s home directory has not been retrieved correctly."        
	exit 1
fi
GRPNAME=`grep -s "^$LOGINID:" /etc/passwd 2> /dev/null | cut -f4 -d: `
if [ -z "${GRPNAME}" ]
then 
	echo "\n$LOGINID's group is not in /etc/group.\n"
	exit 1
fi
cd ${UHOME}     
for dir in FILECABINET WASTEBASKET pref tmp
do
	if [ ! -d ${dir} ]
	then
		mkdir ${dir}
		echo "\t${dir} directory has been created for ${LOGINID}"
	else
		flag=true
		echo "\t${dir} directory already exists"
	fi
	chgrp ${GRPNAME} ${dir}
	chown ${LOGINID} ${dir}
	chmod 755 ${dir}
done
if cp .profile .profile.old
then
	echo "\n\tBackup of .profile saved as .profile.old"
else
	echo "\tCannot make backup copy of .profile"
	echo "\tCannot update ${LOGINID}'s .profile!!"
	exit 1
fi
cp $VMSYS/standard/.profile ${UHOME}
chmod 644 ${UHOME}/.profile
chown "${LOGINID}" ${UHOME}/.profile
chgrp "${GRPNAME}" ${UHOME}/.profile
for file in pref/.environ pref/.variables 
do
	if [ ! -f ${file} ]
	then
		cp $VMSYS/standard/${file} ${UHOME}/${file}
		chmod 755 ${UHOME}/${file}
		chgrp "${GRPNAME}" ${UHOME}/${file}
		chown "${LOGINID}" ${UHOME}/${file}
	else
		echo "\t${file} already exists for ${LOGINID}"
		if cp ${file} ${file}.old
		then
			echo "\tBackup of ${file} saved as ${file}.old"
		else
			echo "\tCannot make backup copy of ${file}"
			echo "\tCannot update ${LOGINID}'s Environment"
			exit 1
		fi
	fi
done
cd ${UHOME}/pref
echo "\n\tUpdating ${LOGINID}'s pref/.environ"
while read evar
do
	var=`echo "${evar}" | cut -d= -f1`
	new=`grep -s "^${var}" .environ`
	if [ -z "${new}" ]
	then
		echo ${evar} >> .environ
	fi
done < ${VMSYS}/standard/pref/.environ

echo "\n\tUpdating ${LOGINID}'s pref/.variables"
while read vvar
do
	var=`echo "${vvar}" | cut -d= -f1`
	new=`grep -s "^${var}" .variables`
	if  [ -z "${new}" ] 
	then
		echo ${vvar} >> .variables
	fi
done < ${VMSYS}/standard/pref/.variables

if [ ${flag} = "false" ]
then
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
		exit 1 
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
		exit 1
	fi
	rm -f $ALLFILES $ALLDIRS

fi


if [ "${service}" = "yes" ]
then 
	if [ -s $UHOME/pref/office ]
	then
		grep -v "name=System Administration" $UHOME/pref/office >/usr/tmp/oadm$$
		grep -v "action=\`run sysadm\`" /usr/tmp/oadm$$ >/usr/tmp/nadm$$
	else
		> /usr/tmp/nadm$$
	fi
	echo "name=System Administration" >>  /usr/tmp/nadm$$ 
	echo "action=\`run sysadm\`" >> /usr/tmp/nadm$$
	/bin/cp /usr/tmp/nadm$$ $UHOME/pref/office
	chown  ${LOGINID} $UHOME/pref/office  
	chgrp ${GRPNAME} $UHOME/pref/office  
	chmod 644 $UHOME/pref/office  
	echo "\n\tloginid: ${LOGINID} now has Sysadm privileges"
	rm -rf /usr/tmp/oadm$$
	rm -rf /usr/tmp/nadm$$
fi
if [ "${service}" = "no" ]
then
	echo "\n\tloginid: ${LOGINID} will not have Sysadm privileges"
	if [ -s $UHOME/pref/office ]
	then
		grep -v "name=System Administration" $UHOME/pref/office >/usr/tmp/oadm$$
		grep -v "action=\`run sysadm\`" /usr/tmp/oadm$$ >/usr/tmp/nadm$$

		cp /usr/tmp/nadm$$  $UHOME/pref/office
		rm -rf /usr/tmp/nadm$$
		rm -rf /usr/tmp/oadm$$
	fi
fi
exit 0
