#	@(#)bcheckrc.sh	1.3

# ***** This file has those commands necessary to check the file
# system, date, and anything else that should be done before mounting
# the file systems.

trap "" 2

# **** Check date

while :
do
	echo "Is the date `date` correct? (y or n) \c"
	read reply
	if
		[ "$reply" = y ]
	then
		break
	else
		echo "Enter the correct date:  \c"
		read reply
		date "$reply"
	fi
done

# ***** Auto check, if necessary

while :
do
	echo "Do you want to check the file systems? (y or n) \c"
	read reply
	case "$reply" in
		 y )
			;;

		n* )
			break
			;;

		* )
			echo "Invalid input. Try again."
			continue
			;;
	esac
	trap "echo Interrupt" 2
	/etc/fsck
	trap "" 2
	break
done
