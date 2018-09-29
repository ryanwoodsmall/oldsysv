#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)help:helpadm.sh	1.7"

trap 'rm -f /tmp/*$$ ; trap 0 ; exit' 0 1 2 3 15
: ${EDITOR:=ed}
HELP=
DATABASE=${HELP}/usr/lib/help/db
SCRNLEN=17
LIB=${HELP}/usr/lib/help
SYSTEM_LOG=${HELP}/usr/lib/help/HELPLOG
export EDITOR HELP DATABASE SCRNLEN LIB
cat <<-!
         helpadm:  UNIX System On-Line help Administrative Utilities

These software tools will enable the administrator to change
information in the help facility's database, and to monitor use of
the help facility.

        choice          description

           1            starter
  
           2            glossary
  
           3            commands
  
           4            prevent recording use of help facility
  
           5            record use of the help facility
  
           q            quit




!
CHOICE=
while [ "${CHOICE}" = "" ]
do
	echo "Enter choice> \c"
	read CHOICE
	case $CHOICE
	in
		1 ) #change starter information
			${LIB}/admstart
			;;			
		2 ) #edit glossary information
			${LIB}/admgloss
			;;		
		3 ) #add commands information
			${LIB}/editcmd
			;;
		4 ) #logoff information
			> /tmp/$$
			a=`ls -o /tmp/$$ | grep root`
			b=`ls -o /tmp/$$ | grep bin`
			if [ -z "$a" -a -z "$b" ]
			then
				echo "The only users who may prevent monitoring the help facility are root and bin."
			else
				/bin/chmod a-w ${SYSTEM_LOG}
			fi
			rm /tmp/$$
			;;
		5 ) #logon information
			> /tmp/$$
			a=`ls -o /tmp/$$ | grep root`
			b=`ls -o /tmp/$$ | grep bin`
			if [ -z "$a" -a -z "$b" ]
			then
				echo "The only users who may enable monitoring the help facility are root and bin."
			else
				/bin/chmod a+w ${SYSTEM_LOG}
			fi
			rm /tmp/$$
			;;
		q ) #quit
			;;
		* ) #bad input
			echo "Illegal input--try again."
			CHOICE=
			;;
	esac
done
exit
