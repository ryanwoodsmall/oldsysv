#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./mountall.sh	1.8"
if u3b2
then echo "#	Mount file systems according to file system table /etc/fstab.
#	Note: written to depend on as few commands as possible.

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

fstab=/etc/fstab

cat \${fstab} |
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
		'-r') 		# This is the only valid flag in mountall
			;;
		'-d' | '-dr' | '-rd') #  remote mounts are done in rmountall
			continue
			;;

		'-')		# Ignore '-'
			fsflags=''
			;;

		*)		# It isn't an option, must be fstype
			fstype=\${fsflags}
			fsflags=''
			;;
		esac 
 		if [ \"\${fstype}\" = \"-\" ]
 		then
 			fstype=''
 		fi
		if [ \"\${fstype}\" != \"\" ]
		then fstype=\"-f \${fstype}\"
		fi

		#	First check file system state and repair if necessary.

		if [ \"\${dev}\" = \"-\" ]
		then
			
			/etc/mount \${fsflags} \${fstype} \${dev} \${fs}
			continue
		fi

		msg=\`/etc/fsstat \${dev} 2>&1\`
		case \$? in
		0)
			/etc/mount \${fsflags} \${fstype} \${dev} \${fs}
			;;
		1)
			rdev=\`basename \${dev}\`
			if [ -c /dev/rdsk/\${rdev} ]
			then
				rdev=/dev/rdsk/\${rdev}
			elif [ -c /dev/r\${rdev} ]
			then
				rdev=/dev/r\${rdev}
			else
				rdev=\${dev}
			fi
			echo \"\${msg}\\n\\t \${rdev} is being checked.\"
			/etc/fsck -y -D \${rdev}

			/etc/mount \${fsflags} \${fstype} \${dev} \${fs}
			;;
		2)
			# echo \"\${dev} already mounted\"
			;;
		3)
			/etc/mount \${fsflags} \${fstype} \${dev} \${fs}
			if [ \$? != 0 ]
			then
				echo \"\${dev} is not a valid filesystem\"
			fi
			;;
		esac
	done
" >mountall
fi
