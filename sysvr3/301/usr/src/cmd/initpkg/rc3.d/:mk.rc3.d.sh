#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./rc3.d/:mk.rc3.d.sh	1.7"

STARTLST="21rfs"
STOPLST= 

INSDIR=${ROOT}/etc/rc3.d
if u3b2
then
	if [ ! -d ${INSDIR} ] 
	then 
		mkdir ${INSDIR} 
		eval ${CH}chmod 755 ${INSDIR}
		eval ${CH}chgrp sys ${INSDIR}
		eval ${CH}chown root ${INSDIR}
	fi 
	for f in ${STARTLST}
	do 
		name=`echo $f | sed -e 's/^..//'`
		ln ${ROOT}/etc/init.d/${name} ${INSDIR}/S$f
		eval ${CH}chmod 744 ${INSDIR}/S$f
		eval ${CH}chgrp sys ${INSDIR}/S$f
		eval ${CH}chown root ${INSDIR}/S$f
	done
fi
