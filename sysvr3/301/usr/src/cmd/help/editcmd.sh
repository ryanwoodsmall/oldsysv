#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)help:editcmd.sh	1.8"

trap 'rm -f /tmp/$$.editcmd ; rm -f /tmp/$$.response ; rm -f /tmp/*$$ ; trap 0 ; exit' 0 1 2 3 15
if [ -f /tmp/*.editcmd ]
then
	echo "Someone else is using helpadm to update command information."
	echo "Try to update command information later."
else
	>/tmp/$$.editcmd
	echo "Enter the name of the command to be added/modified/deleted > \c"
	read CMD
	cd /tmp
	${HELP}/usr/lib/help/extract ${DATABASE}/descriptions.a ${CMD} 2>/dev/null
	mv /tmp/${CMD} /tmp/${CMD}$$ 2>/dev/null
	if [ -f /tmp/${CMD}$$ ]
	then
		echo "The  ${CMD} command is already included in help.\n"
		if [ ! -w /tmp/${CMD}$$ ]
		then
			echo "You do not have permission to change the database files for ${CMD}."
			echo "Exiting with no changes to the help facility database."
			rm /tmp/$$.editcmd
			exit
		fi
		rm /tmp/${CMD}$$
		ANS=
		while [ \( "${ANS}" != "y" \) -a \( "${ANS}" != "n" \) ]
		do
			echo "Do you want to delete information on ${CMD} (y or n)? >\c"
			read ANS
		done
	#### code to modify existing information
		if [ "${ANS}" != "y" ]
		then
			INPUT=yes
			while [ "$INPUT" = "yes" ]
			do
			echo "Which part of the command data do you wish to edit?"
			echo "Enter d(desc), o(options), e(examp), k(keywds), or q(quit) > \c"
			read CHOICE
			case $CHOICE
			in
				d ) #edit description screen
					SCREEN=descriptions.a
					NAME=Description
					FLAG=d
					CHOICE=q
					;;
				o ) #edit options screen
					SCREEN=options.a
					NAME=Options
					FLAG=o
					CHOICE=q
					;;
				e ) #edit examples screen
					SCREEN=examples.a
					NAME=Examples
					FLAG=e
					CHOICE=q
					;;
				k ) #edit keyword list
					SCREEN="tables/keywords"
					FLAG=k
					CHOICE=q
					;;
				q ) # get out of input loop
					INPUT=no
					FLAG=q
					;;
				* ) #bad input
					echo "Illegal input -- try again."
					FLAG=
					;;
			esac
	#code to modify descriptions, options, and examples
			if [ \( "${FLAG}" = "d" \) -o \( "${FLAG}" = "o" \) -o \( "${FLAG}" = "e" \) ]
			then
			${HELP}/usr/lib/help/extract ${DATABASE}/${SCREEN} ${CMD}
			mv /tmp/${CMD} /tmp/${FLAG}.${CMD}$$
			echo "\nEditing ${NAME} Screen for ${CMD}"
			LEN=bad EDFILE=/tmp/${FLAG}.${CMD}$$ ${LIB}/checklen
			fi
	#code to delete keywords
			if [ "${FLAG}" = "k" ]
			then
				echo "\nEditing keyword list for ${CMD}"
				echo "Remember that all keywords are a single word."
				grep "The \\\\\\\\S${CMD}\\\\\\\\S " <${DATABASE}/${SCREEN} >/tmp/cmd$$
				ed - /tmp/cmd$$ <<-!
				1,\$s/key='//
				1,\$s/'.*//
				w
				q
				!
				cp /tmp/cmd$$ /tmp/${FLAG}.${CMD}$$
				echo "\nThe current keyword list for ${CMD} is: "
				cat /tmp/cmd$$
				if [ `wc -l </tmp/cmd$$` -gt 1 ]
				then
					ed - /tmp/cmd$$ <<-!
					2,\$s/^/ /
					1,\$j
					w
					q
					!
				fi
	#WORDS will have all the keywords for the command on one line,
	#each separated by a blank
				WORDS=`cat /tmp/cmd$$`
				ANS=
				while [  \( "${ANS}" != "y" \) -a \( "${ANS}" != "n" \) ]
				do
					echo "Do you want to delete any of these?  (y or n) > \c"
					read ANS
				done
				if [ "$ANS" = "y" ]
				then
					OUTWORD=
					echo "Enter a keyword to be deleted after each colon (:)."
					echo "Enter a period (.) to stop deletions."
					while [ "${OUTWORD}" != "." ]
					do
						echo ": \c"
						read OUTWORD
						if [ "${OUTWORD}" = "." ]
						then
							break
						fi
	#all existing keywords except the one to be deleted are put into a
	#temporary file
						for i in $WORDS
						do
							if [ "${OUTWORD}" = "${i}" ]
							then
							:;
							else
							echo $i >> /tmp/${FLAG}.${CMD}2$$
							fi
						done
						if [ -f /tmp/${FLAG}.${CMD}2$$ ]
						then
							cp /tmp/${FLAG}.${CMD}2$$ /tmp/cmd$$
							if [ `wc -l </tmp/cmd$$` -gt 1 ]
							then
								ed - /tmp/cmd$$ <<-!
								2,\$s/^/ /
								1,\$j
								w
								q
								!
							fi
	#WORDS will have all the keywords for the command on one line, 
	#each separated by a blank
							WORDS=`cat /tmp/cmd$$`
							mv /tmp/${FLAG}.${CMD}2$$ /tmp/${FLAG}.${CMD}$$
						else
							>/tmp/${FLAG}.${CMD}$$
							cp /tmp/${FLAG}.${CMD}$$ /tmp/cmd$$
						fi
					done
					rm /tmp/cmd$$
					echo "New keyword list for $CMD: "
					cat /tmp/${FLAG}.${CMD}$$
				else
					:;
				fi
	#code to add keywords
			ANS=
			while [ \( "${ANS}" != "y" \) -a \( "${ANS}" != "n" \) ]
			do
			echo "Do you want to add new keywords for ${CMD}? (y or n) > \c"
			read ANS
			done
			if [ "${ANS}" = "y" ]
			then
				MOREKEY=yes
				echo "Enter a new keyword after each colon (:)."
				echo "Enter a period (.) to quit adding keywords."
				while [ "${MOREKEY}" = "yes" ]
				do
					echo "\t: \c"
					read KEY
					if [ "${KEY}" != "" ]
					then
						set ${KEY}
						if [ $# -gt 1 ]
						then
							echo "Not a single word keyword."
						elif [ "${KEY}" != "." ]
						then
							echo ${KEY} >>/tmp/${FLAG}.${CMD}$$
						else
							MOREKEY=no
						fi
					fi
				done
				echo "The updated keyword list for ${CMD} is: "
				cat /tmp/${FLAG}.${CMD}$$
				fi
			fi
	#code to quit editing command data
			if [ "${FLAG}" = "q" ]
			then
				INPUT=no
			fi
			done   ###end loop to edit command data
	####code to delete a command
		else
			ANS=
			while [ \( "${ANS}" != "y" \) -a \( "${ANS}" != "n" \) ]
			do
				echo "Are you sure you want to remove keywords, description, option, and example information for ${CMD} (y or n)? >\c"
				read ANS
			done
			if [ "${ANS}" = "y" ]
			then
				table=responses action="deleted from" info=${CMD} place=database ${LIB}/locking
				>/tmp/$$.response
				ed - ${DATABASE}/tables/responses <<-!
				g/^${CMD}:getdesc,listcmds,getexample,getoption,usage,keysrch:getdoe/d
				w
				q
				!
				rm /tmp/$$.response
				${HELP}/usr/lib/help/delete ${DATABASE}/descriptions.a ${CMD}
				${HELP}/usr/lib/help/delete ${DATABASE}/options.a ${CMD}
				${HELP}/usr/lib/help/delete ${DATABASE}/examples.a ${CMD}
				ed - ${DATABASE}/tables/keywords <<-!
				g/.*The \\\\\\\\S${CMD}\\\\\\\\S .*command/d
				w
				q
				!
				echo "Keywords, description, option, and example information for ${CMD} have"
				echo "been deleted from the help facility database."
				rm /tmp/$$.editcmd
				exit
			else
				echo "Exiting with no changes to the help facilty database."
				rm /tmp/$$.editcmd
				exit
			fi
		fi
	#end processing for existing command data######################
	#code to enter a new command's data
	else
		echo "New Command: ${CMD}"
		cat <<-!
	
	This command is not currently included in the help facility.
	You must enter a COMPLETE set of command data, including
	description and syntax information, option information, usage
	examples, and a keyword set, if ANY of the data you enter
	are to be added to the help facility's database.  You will
	be asked to explicitely request that this data be included in
	the help facility database at the end of this session.
	
	!
		for i in desc opt ex
		do
		case ${i}
		in
			desc )  #description screen
				FLAG=d
				SCREEN=Description
				;;
			opt )  #options screen
				FLAG=o
				SCREEN=Options
				;;
			ex )  #examples screen
				FLAG=e
				SCREEN=Examples
				;;
		esac
		EDFILE=/tmp/${FLAG}.${CMD}$$
		>> ${EDFILE}
		echo " \S ${CMD} \S " >> ${EDFILE}
		echo "Editing ${SCREEN} Screen for ${CMD}"
		EDFILE=/tmp/${FLAG}.${CMD}$$ LEN=bad ${LIB}/checklen
		echo "${SCREEN} Screen Completed."
		done
	####end addition of description, options, examples
	####add new keywords next
		FLAG=k
		EDFILE=/tmp/${FLAG}.${CMD}$$
		echo "Making Keyword List for ${CMD}"
		cat <<-!
	Enter a single keyword for the command after each colon (:).
	To stop adding keywords, enter a period (.).
	
	!
		NEWKEY=
		>/tmp/${FLAG}.${CMD}$$
		while [ "$NEWKEY" != "." ]
		do
		echo "	: \c"
		read NEWKEY
		if [ "${NEWKEY}" != "" ]
		then
			set ${NEWKEY}
			if [ $# -gt 1 ]
			then
				echo "Not a single word keyword."
			elif [ "${NEWKEY}" != "." ]
			then
				echo "${NEWKEY}" >> /tmp/${FLAG}.${CMD}$$
			elif [ `wc -l </tmp/${FLAG}.${CMD}$$` -le 0 ]
			then
				echo "You must enter at least one keyword.  Re-editing keyword list for ${CMD}."
				NEWKEY=
			else
				echo "Keyword list for ${CMD}: "
				cat /tmp/${FLAG}.${CMD}$$
				ANS=
				while [ \( "${ANS}" != "y" \) -a \( "${ANS}" != "n" \) ]
				do
				echo "Are you satisfied with this list? (y or n) > \c"
				read ANS
				done
				if [ "${ANS}" = "y" ]
				then
					:;
				else
					NEWKEY=
					echo "Add more keywords now."
					echo "To delete keywords, exit to the shell and reedit."
				fi
			fi
		fi
		done
	fi
	####explicitely request changes to database
	ANS=
	while [ \( "${ANS}" != "y" \) -a \( "${ANS}" != "n" \) ]
	do
		echo "Do you want the data you have entered to be added to the help"
		echo "facility database? (y or n) > \c"
		read ANS
	done
	if [ "${ANS}" != "y" ]
	then
		echo "Exiting with no updates to help database."
	else
	#if the command is not already included in the responses table, then
	#include it
		table=responses action="added to" info=${CMD} place=database ${LIB}/locking
		>/tmp/$$.response
		grep "^${CMD}:getdesc,listcmds,getexample,getoption,usage,keysrch:getdoe$" ${DATABASE}/tables/responses >/tmp/cmd$$
		if [ `wc -l </tmp/cmd$$` -lt 1 ]
		then
			echo "${CMD}:getdesc,listcmds,getexample,getoption,usage,keysrch:getdoe" >>${DATABASE}/tables/responses
		fi
		rm /tmp/$$.response
	
		if [ -f /tmp/d.${CMD}$$ ]
		then
			chmod 644 /tmp/d.${CMD}$$
			mv /tmp/d.${CMD}$$ /tmp/${CMD}
			${HELP}/usr/lib/help/replace ${DATABASE}/descriptions.a ${CMD}
			echo "Description Screen for ${CMD} Updated"
			rm /tmp/${CMD}
		fi
		if [ -f /tmp/o.${CMD}$$ ]
		then
			chmod 644 /tmp/o.${CMD}$$
			mv /tmp/o.${CMD}$$ /tmp/${CMD}
			${HELP}/usr/lib/help/replace ${DATABASE}/options.a ${CMD}
			echo "Options Screen for ${CMD} Updated"
			rm /tmp/${CMD}
		fi
		if [ -f /tmp/e.${CMD}$$ ]
		then
			chmod 644 /tmp/e.${CMD}$$
			mv /tmp/e.${CMD}$$ /tmp/${CMD}
			${HELP}/usr/lib/help/replace ${DATABASE}/examples.a ${CMD}
			echo "Examples Screen for ${CMD} Updated"
			rm /tmp/${CMD}
		fi
		if [ -f /tmp/k.${CMD}$$ ]
		then
			if [ `wc -l </tmp/k.${CMD}$$` -gt 1 ]
			then
				ed - /tmp/k.${CMD}$$ <<-!
				2,\$s/^/ /
				1,\$j
				w
				q
				!
			fi
	#WORDS will have all the keywords on one line separated by a blank.
			WORDS=`cat /tmp/k.${CMD}$$`
			cp ${DATABASE}/tables/keywords /tmp/keywords$$
			ed - /tmp/keywords$$ <<-!
			g/The \\\\\\\\S${CMD}\\\\\\\\S/d
			w
			q
			!
			for i in ${WORDS}
			do
			echo "key='${i}'	echo 'The \\\\\\\\S${CMD}\\\\\\\\S command'" >> /tmp/keywords$$
			done
			ed - ${DATABASE}/tables/keywords <<-!
			1,\$d
			.r /tmp/keywords$$
			w
			q
			!
			rm /tmp/keywords$$
			rm /tmp/k.${CMD}$$
			echo "Keyword List for ${CMD} Updated."
		fi
	echo "Modifications to help database complete."
	fi
	rm /tmp/$$.editcmd
fi
rm /tmp/*$$ 2>/dev/null
exit
