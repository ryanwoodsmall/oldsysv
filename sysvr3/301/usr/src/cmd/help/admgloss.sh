#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)help:admgloss.sh	1.4

trap 'rm -f /tmp/$$.admgloss ; rm -f /tmp/$$.response ; rm -f /tmp/*$$ ; trap 0 ; exit' 0 1 2 3 15
if [ -f /tmp/*.admgloss ]
then
	echo "Someone else is using helpadm to update the glossary."
	echo "Try to update the glossary later."
else
	>/tmp/$$.admgloss
	echo "Enter the name of the glossary term to be added/modified/deleted> \c"
	read term
	grep "^${term}:glossary" ${DATABASE}/tables/responses >/tmp/gloss$$
	cd /tmp
	if [  -s /tmp/gloss$$ ]
	then
####code if term already exists
		echo "${term} is already included in the glossary."
		ed - /tmp/gloss$$ <<-!
		1s/${term}:glossary://
		w
		Q
		!
		filename=`cat /tmp/gloss$$ | line`
		rm /tmp/gloss$$
		${HELP}/usr/lib/help/extract ${DATABASE}/glossary.a ${filename} 
#if file is not writeable by that user, do not let him/her modify or
#delete it
		if [ ! -w  /tmp/${filename} ]
		then
			echo "You do not have permission to delete the definition of ${term}."
			echo "Exiting with no changes to the help facility database."
		else
			mv /tmp/${filename} /tmp/${filename}$$
			CHOICE=
			while [ \( "${CHOICE}" != "d" \) -a \( "${CHOICE}" != "m" \) ]
			do
				echo "Do you want to m(modify) its definition or d(delete) it from the glossary?"
				echo "Enter choice (m or d ) > \c"
				read CHOICE
			done
#deleting a glossary term
			if [ "${CHOICE}" = "d" ]
			then
				ANS=
				while [ \( "${ANS}" != "y" \) -a \( "${ANS}" != "n" \) ]
				do
					echo "Are you sure you want to delete ${term} from the glossary?"
					echo "Enter choice (y or n) > \c"
					read ANS
				done
				if [ "$ANS" = "y" ]
				then
#delete term from responses table
					table=responses action="deleted from" info=${term} place=glossary ${LIB}/locking
					>/tmp/$$.response
					ed - ${DATABASE}/tables/responses <<-!
					g/^${term}:glossary:define/d
					w
					q
					!
#check if more than one glossary term uses the same file for its definition.
#if so, the definition file is not removed.  
					grep ${filename} ${DATABASE}/tables/responses >/tmp/gloss$$
					rm /tmp/$$.response
					if [ ! -s /tmp/gloss$$ ]
					then
						${HELP}/usr/lib/help/delete ${DATABASE}/glossary.a ${filename} 2>/dev/null
					else
						rm /tmp/gloss$$
					fi
					echo "${term} deleted from glossary."
				else
					echo "Exiting with no changes to the help facility database."
				fi
#code for modifying an existing term definition
			else
				echo "Editing definition for ${term}"
				LEN=bad EDFILE=/tmp/${filename}$$ term=${term} ${LIB}/defnlen
#explicitely request modifications to database
				ANS=
				while [ \( "${ANS}" != "y" \) -a \( "${ANS}" != "n" \) ]
				do
					echo "Do you want the data you have entered to be added to the help"
					echo "facility database (y or n)?> \c"
					read ANS
				done
				if [ "${ANS}" = "n" ]
				then
					echo "Exiting with no changes to the help database."
				else
					mv /tmp/${filename}$$ /tmp/${filename}
					${HELP}/usr/lib/help/replace ${DATABASE}/glossary.a ${filename}
					echo "Modifications to the help glossary complete."
				fi
			fi
		fi
	else
#code for adding a new glossary term
####get a filename for the new term 
		start=`grep -c ":define[0-9]*" ${DATABASE}/tables/responses`
		${HELP}/usr/lib/help/extract ${DATABASE}/glossary.a define${start} 2>/dev/null
		while [ -f /tmp/define${start} ]
		do
			rm /tmp/define${start}
			start=`expr ${start} + 1`
			${HELP}/usr/lib/help/extract ${DATABASE}/glossary.a define${start} 2>/dev/null
		done
		filename=define${start}
		>/tmp/${filename}$$
		echo " \S ${term} \S " >>/tmp/${filename}$$
		chmod 644 /tmp/${filename}$$
		echo "Editing definition for ${term}"
		LEN=bad EDFILE=/tmp/${filename}$$ term=${term} ${LIB}/defnlen
#explicitely request changes to database
		ANS=
		while [ \( "${ANS}" != "y" \) -a \( "${ANS}" != "n" \) ]
		do
			echo "Do you want the data you have entered to be added to the help"
			echo "facility database (y or n)?> \c"
			read ANS
		done
		if [ "${ANS}" = "n" ]
		then
			rm /tmp/${filename}$$
			echo "Exiting with no changes to the help database."
		else
			table=responses action="added to" info=${term} place=glossary ${LIB}/locking
			>/tmp/$$.response
			echo "${term}:glossary:${filename}" >>${DATABASE}/tables/responses
			mv /tmp/${filename}$$ /tmp/${filename}
			${HELP}/usr/lib/help/replace ${DATABASE}/glossary.a ${filename}
			rm /tmp/$$.response
			echo "Modifications to the help glossary complete."
		fi
	fi
	rm /tmp/$$.admgloss
fi
rm -f /tmp/${filename} 2>/dev/null
exit
