#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./rumountall.sh	1.12"

if u3b2
then
echo "#	Unmounts remote file resources
#	Note: written to depend on as few commands as possible.

ULIST=/tmp/ulist\$\$

trap 'rm -f \${ULIST}' 0 1 2 3 15

kill=
while [ \$# -ge 1 ]
do
	case \"\$1\" in
	-k )
		if [ ! -x /etc/fuser ]
		then
			echo >&2 \"\$0:  WARNING!!!  /etc/fuser not found.\"
		else
			kill=yes
		fi
		;;
	* )
		echo >&2 \"Usage:  \$0 [ -k ]
-k	kill processes with files open in each file system before unmounting.\"
		exit 1
	esac
	shift
done
#		kill outstanding rmount scripts
 
for pid in \`ps -ef | grep /etc/rmount | grep -v grep \\
		   | sed -e 's/ *[^ ]*  *//' -e 's/ .*//'\`
do
	kill -9 \${pid} 
done
 
if [ \${kill} ]
then
	>\${ULIST}
	/etc/mount  |
		sort -r  |
		while read fs dummy1 dev mode1 mode2 dummy2
		do
			if [ \`echo \${mode1}\${mode2} | grep remote\` ]
			then
				echo  \"\${dev} \\\c\" >>\${ULIST}
			fi
		done 
	klist=\`cat \${ULIST}\`
	if [ \"\${klist}\" = \"\" ]
	then
		exit
	fi
	/etc/fuser -k \${klist} >/dev/null 2>&1
	for dev in \${klist}
	do
		/etc/umount -d \${dev}
	done 
else
	/etc/mount  |
		sort -r  |
		while read fs dummy1 dev mode1 mode2 dummy2
		do
			if [ \`echo \${mode1}\${mode2} | grep remote\` ]
			then
				/etc/umount -d \${dev}
			fi
		done 
fi" >rumountall
fi
