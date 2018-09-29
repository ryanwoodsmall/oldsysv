#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./umountall.sh	1.4"
echo "#	Unmounts all but the root, \"/\", file system.
#	Note: written to depend on as few commands as possible.

#!	chmod +x \${file}

kill=
if [ \$# -ge 1 ]
then
	case \"\$1\" in
	-k )
		if [ ! -x /etc/fuser ]
		then
			echo >&2 \"\$0:  -k option not available.  No /etc/fuser found.\"
			exit 1
		fi
		kill=yes
		;;
	* )
		echo >&2 \"Usage:  \$0 [ -k ]
-k	kill processes with files open in each file system before unmounting.\"
		exit 1
	esac
fi

/etc/mount  |
	sort -r  |
	while read fs dummy1 dev mode mode2 dummy2
	do
		case \"\${mode}\" in
		'read/write/remote')
			mode=-d		# remote resource
			;;
		'read')
			case \"\${mode2}\" in
			'only/remote')
				mode=-d
				;;
			esac
			;;
		esac

		case \"\${fs}\" in
		/  |  '' )
			continue
			;;
		* )
			if [ \"\${mode}\" = \"-d\" ]
			then
				/etc/umount -d \${dev}
			else
				if [ \${kill} ]
				then
					/etc/fuser -k \${dev}
					sleep 10
				fi
				/etc/umount \${dev}
			fi
		esac
	done 
" >umountall
