#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)help:admstart.sh	1.3"

trap 'rm -f /tmp/$$.admstart 2>/dev/null ; rm -f /tmp/*$$ ; trap 0 ; exit' 0 1 2 3 15
if [ -f /tmp/*.admstart ]
then
	echo "Someone else is using helpadm to update starter
information."
	echo "Try to update it later."
else
	>/tmp/$$.admstart
	cat <<-!
         helpadm:  starter
	
Which screens of starter do you want to make changes to?

        choice          description

           c            commands screen

           d            documents screen

           e            education screen

           l            local screen

           t            teach screen

           q            quit






	!
	CHOICE=
	while [ "${CHOICE}" = "" ]
	do
		echo "Enter choice> \c"
		read CHOICE
		case $CHOICE
		in
			c ) SCREEN=basiccmds
			    NAME=commands
			    ;;
			d ) SCREEN=documents
			    NAME=documents
			    ;;
			e ) SCREEN=education
			    NAME=education
			    ;;
			l ) SCREEN=local
			    NAME=local
			    ;;
			t ) SCREEN=teach
			    NAME=teach
			    ;;
			q ) exit
			    ;;
			* ) #bad input
		 	    echo "Illegal input--try again."
			    CHOICE=
			    ;;
		esac
	done
	>/tmp/$$.${NAME}
	cd /tmp
	${HELP}/usr/lib/help/extract ${DATABASE}/screens.a ${SCREEN} 2>/dev/null
	if [ -w /tmp/${SCREEN} ]
	then
		mv /tmp/${SCREEN} /tmp/${SCREEN}$$
		echo "\n Editing starter ${NAME} Screen"
		LEN=bad EDFILE=/tmp/${SCREEN}$$ ${LIB}/checklen
####explicitely request all changes to database
		ANS=
		while [ \( "${ANS}" != "y" \) -a \( "${ANS}" != "n" \) ]
		do
			echo "Do you want the data you have entered to be added to the help facility database (y or n)?> \c"
			read ANS
		done
		if [ "${ANS}" != "y" ]
		then
			rm /tmp/${SCREEN}$$ 2>/dev/null
			echo "Exiting with no updates to the help database."
		else
			chmod 644 /tmp/${SCREEN}$$
			mv /tmp/${SCREEN}$$ /tmp/${SCREEN}
			${HELP}/usr/lib/help/replace ${DATABASE}/screens.a ${SCREEN}
			rm /tmp/${SCREEN} 2>/dev/null
			echo "Modifications to help starter database complete."
		fi
	else
		echo "You do not have permission to change this starter screen."
		echo "Exiting with no changes to the help facility database."
	fi
	rm /tmp/$$.admstart
fi
exit
