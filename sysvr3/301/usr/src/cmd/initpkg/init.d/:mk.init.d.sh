#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:init.d/:mk.init.d.sh	1.2"

INSDIR=${ROOT}/etc/init.d
if u3b2
then
	if [ ! -d ${INSDIR} ] 
	then 
		mkdir ${INSDIR} 
		if [ $? != 0 ]
		then
			exit 1
		fi
		eval ${CH}chmod 755 ${INSDIR}
		eval ${CH}chgrp sys ${INSDIR}
		eval ${CH}chown root ${INSDIR}
	fi 
	for f in [a-zA-Z0-9]* 
	do 
		cp $f ${INSDIR} 
		eval ${CH}chmod 744 ${INSDIR}/$f
		eval ${CH}chgrp sys ${INSDIR}/$f
		eval ${CH}chown root ${INSDIR}/$f
	done
fi
