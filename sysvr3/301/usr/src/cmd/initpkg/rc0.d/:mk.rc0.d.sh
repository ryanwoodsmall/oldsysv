#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./rc0.d/:mk.rc0.d.sh	1.7"

STARTLST= 
STOPLST="00ANNOUNCE 50fumounts 60rumounts 65rfs"

INSDIR=${ROOT}/etc/rc0.d
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
fi
