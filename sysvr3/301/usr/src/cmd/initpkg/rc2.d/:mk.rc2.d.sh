#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./rc2.d/:mk.rc2.d.sh	1.12"

STARTLST=" 00firstcheck 01MOUNTFSYS 05RMTMPFILES 10disks \
15autoconfig 20sysetup 70uucp 75cron"

STOPLST="30fumounts 40rumounts 50rfs"

INSDIR=${ROOT}/etc/rc2.d
if u3b2
then
	if [ ! -d ${INSDIR} ] 
	then 
		mkdir ${INSDIR} 
		eval ${CH}chmod 755 ${INSDIR}
		eval ${CH}chgrp sys ${INSDIR}
		eval ${CH}chown root ${INSDIR}
	fi 
	for f in ${STOPLST}
	do 
		name=`echo $f | sed -e 's/^..//'`
		ln ${ROOT}/etc/init.d/${name} ${INSDIR}/K$f
		eval ${CH}chmod 744 ${INSDIR}/K$f
		eval ${CH}chgrp sys ${INSDIR}/K$f
		eval ${CH}chown root ${INSDIR}/K$f
	done
	for f in ${STARTLST}
	do 
		name=`echo $f | sed -e 's/^..//'`
		ln ${ROOT}/etc/init.d/${name} ${INSDIR}/S$f
		eval ${CH}chmod 744 ${INSDIR}/S$f
		eval ${CH}chgrp sys ${INSDIR}/S$f
		eval ${CH}chown root ${INSDIR}/S$f
	done
fi
