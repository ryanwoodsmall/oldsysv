#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./rmountall.sh	1.7"

if u3b2
then echo "
#	Mount remote resources according to a file system table
#	such as /etc/fstab.

#	file-system-table format:
#
#	column 1	block special file name of file system
#	column 2	mount-point directory
#	column 3	\"-r\" if to be mounted read-only
#			\"-d\" if distributed (remote) resource
#	column 4	file system type (may be column 4)
#	column 5+	ignored
#	White-space separates columns.
#	Lines beginning with \"#\" are comments.  Empty lines are ignored.
#	a '-' in any field is a no-op.

#!	chmod +x \${file}

if [ \$# -lt 1 ]
then
	echo >&2 \"Usage:  \$0 file-system-table ...\"
	exit 1
fi

cat \$*  |
	while  read dev fs fsflags fstype dummy
	do
		case \${dev} in
		'#' | '')	#  Ignore comments, empty lines
			continue
		esac
		case \${dev} in
		'-')		#  Ignore no-action lines
			continue
		esac 
		case \${fsflags} in

		'-d' | '-dr' | '-rd') #  remote mounts 
 			if [ \"\${fstype}\" = \"-\" ]
 			then
 				fstype=''
 			fi
			if [ -x /bin/setpgrp ]
			then
				/bin/setpgrp sh /etc/rmount \${fsflags} \${dev} \\
				\${fs} \${fstype} >/dev/console 2>&1&
			else
				/usr/lbin/setpgrp sh /etc/rmount \${fsflags} \\
				\${dev} \${fs} \${fstype} >/dev/console 2>&1&
			fi
		esac 

	done
" >rmountall
fi
