#ident	"@(#)pkg.cc:INSTALL.cc	1.21"
#
# Multiple Floppy INSTALL Script
#
#
# Save mount point of floppy (INSPATH), package name (PKGNAME) and
# information for the mount & umount commands (mntdev & mntname).
#	Note that \fILAST\fR should be set equal to the number of
#	floppy diskettes in the set.  In this example, there are 3 floppies.
#
#	This script has been specially tailored for the cc package.

# Before the actual installation starts, see if this is a pre 3.0 system.
#	If it is, then continue with installation.
#	If this is a 3.0 or later system, notify the customer that
#	the sys header package that came with the core package will
#	be required for installation to be considered complete and give
#	the customer the option of stopping the installation if the
#	package is not available.

INSPATH=/install/new
PKGNAME=`cat ${INSPATH}/usr/options/cc.name`
PKGNAME1=/usr/options/cc.name #the name on the machine
LAST=2
if
	test -b "$1"
then
	mntdev=$1
elif
	test -b /dev/diskette
then
	mntdev=/dev/diskette
elif
	test -b /dev/install
then
	mntdev=/dev/install
else
	echo "**ERROR**   Can't find mount device"
	exit
fi
mntname=${2:-/install}
echo "Installing the ${PKGNAME}."
echo "Copyright (c) 1985 AT&T Technologies, Inc."
echo "All Rights Reserved"

# Check that an old Issue and Version of the package will not accidentally
#	 install over a later Issue and Version
#

if
	test -r ${PKGNAME1}
then		 #package is already on machine - check for Issue and Version
	ISS=`expr "\`cat ${PKGNAME1}\`" : '.*Issue[ 	]*\([^ 	]\).*'`
	VER=`expr "\`cat ${PKGNAME1}\`" : '.*Version[ 	]*\([^ 	]\).*'`
	if
		test -n "$ISS"
	then
		FISS=`expr "${PKGNAME}" : '.*Issue[ 	]*\([^ 	]\).*'`
		FVER=`expr "${PKGNAME}" : '.*Version[ 	]*\([^ 	]\).*'`

		if
			test $ISS -gt $FISS
		then
			echo "**WARNING**  This will install an older"
			echo "Issue/Version ($FISS/$FVER) over the current Issue/Version ($ISS/$VER)"
			answer=""
			while 
				[ "$answer" != y -a "$answer" != n ]
			do
				echo "Type 'y' to install over current Issue or\c"
				echo " 'n' to stop installation:  \c"
				read answer
				case $answer in
					n)	exit
						;;
				
					y)	;; #continue installation
				esac
			done
		elif
			test $ISS -eq $FISS
		then
			if
				test $VER -gt $FVER
			then
				echo "**WARNING**  This will install an older"
				echo "Issue/Version ($FISS/$FVER) over the current Issue/Version ($ISS/$VER)"
				answer=""
				while 
					[ "$answer" != y -a "$answer" != n ]
				do
					echo "Type 'y' to install over current Issue or\c"
					echo " 'n' to stop installation:  \c"
					read answer
					case $answer in
						n)	exit
							;;
					
						y)	;; #continue installation
					esac
				done
			fi
		fi
	fi
fi


# Decide whether this is a 1.1, 1.2, or 2.1 system
# if not one of the above systems then the sys headers will
#     be installed from a "sys header" package provided with the O/S core

NEEDSYS=no
case `uname -r` in
	2.0)	VARPATH=/install/new1.1		# 1.1 system
		;;
	2.0.?)	VARPATH=/install/new1.2		# 1.2 system
		;;
	2.1.?)	VARPATH=/install/new2.1		# 2.1 system
		;;
	*)	NEEDSYS=yes
		VARPATH=""
		;;
esac


#
# Check that the necessary packages have already been installed.
#	If this package has no dependencies, then the following code
#	is deleted.
#

if
	test ! -s /usr/options/sgs.name
then
	echo "**ERROR**   ${PKGNAME} cannot be installed --"
	echo "It requires the Software Generation Utilities to be installed first."
	exit
fi

#
# If the sys header package is required warn the customer and allow
#	an exit if the customer does not have the package at this time.

if [ ${NEEDSYS} = "yes" ]
then
	echo "\n\n             NOTE NOTE NOTE \n"
	echo "After you have installed the ${last} floppies that"
	echo "make up the ${PKGNAME} you will"
	echo "be asked to install the System Header Files floppy that"
	echo "was included in the set of Essential Utilities Floppies"
	echo "that came with your Operating System package.\n"
	echo "If that floppy is not available at this time type 'n'"
	echo "when you are asked if you want to continue.  If you"
	echo "install the ${PKGNAME}"
	echo "and do not install the System Header"
	echo "Files the installation will not be complete and you may"
	echo "have problems compiling code\n"
	answer=""
	while [ "$answer" != y -a "$answer" != n ]
	do
		echo "Type 'y' to continue installation of ${PKGNAME}"
		echo "or 'n' to stop installation because the System Header \c"
		echo "Files floppy"
		echo "is not available at this time."
		echo "Continue ?  \c"
		read answer
		case $answer in
			n)	exit
				;;
			y)	;;	#continue installation
		esac
	done
fi
	
#
# Begin loop for multiple floppies
#

expect=1
while
	[ $expect -le $LAST ]
do
	if
		[ $expect -gt 1 ]
	then
		cd /
		/etc/umount $mntdev 2>/dev/null
		echo "Remove floppy and insert floppy number $expect"
		echo "Type <return> when ready:  \c"
		read answer
		/etc/mount $mntdev $mntname -r 2>/dev/null
	fi

	#
	# Check to make sure this is the correct floppy of the set.
	#

	answer=""
	while
		[ `cat /install/install/ORDER` != $expect -a "$answer" != y ]
	do
		echo "**WARNING**   Floppy out of sequence"
		echo "Expecting floppy diskette number $expect"
		while
			[ "$answer" != y -a "$answer" != n ]
		do
			echo "Type 'y' to continue or \c"
			echo "'n' to try another floppy:  \c"
			read answer
		done
		case $answer in
			n)	cd /
				/etc/umount $mntdev 2>/dev/null
				echo "Remove floppy and insert correct floppy"
				echo "Type <return> when ready:  \c"
				read answer

				#
				# Reset answer to ""
				# check to make sure that this
				# is now the correct floppy
				#

				answer=""
				/etc/mount $mntdev $mntname -r 2>/dev/null
				;;

			y)	expect=`cat /install/install/ORDER`
				;;
		esac
	done

	#
	# How much space is needed by this floppy?
	#	If the floppy contains packed or cpio files,
	#	the space needs should be hard coded and verified
	#	each time there is a new delivery of the source code.
	#	e.g. The following three lines would be replaced by:
	#	     USRneeds=250
	#	     ROOTneeds=100
	#
	# For cc package: space depends on the common stuff in INSPATH,
	# plus the release-specific stuff in VARPATH.

	USRCneeds=`expr "\`du -s ${INSPATH}/usr\`" : "\([0-9]*\).*"`
	if [ "${VARPATH}" != "" ]
	then
	USRVneeds=`expr "\`du -s ${VARPATH}/usr\`" : "\([0-9]*\).*"`
	USRneeds=`expr ${USRCneeds} + ${USRVneeds}`
	else
	USRVneeds=0
	USRneeds=${USRCneeds}
	fi
	ROOTCneeds=`expr "\`du -s ${INSPATH}\`" : "\([0-9]*\).*"`
	if [ "${VARPATH}" != "" ]
	then
	ROOTVneeds=`expr "\`du -s ${VARPATH}\`" : "\([0-9]*\).*"`
	ROOTneeds=`expr ${ROOTCneeds} + ${ROOTVneeds} - ${USRneeds}`
	else
	ROOTVneeds=0
	ROOTneeds=`expr ${ROOTCneeds} - ${USRneeds}`
	fi

	#
	# How much space is available in the necessary filesystems?
	#

	USRspace=`expr "\`df /usr 2>/dev/null\`" : '.*: *\([0-9]*\)'`
	ROOTspace=`expr "\`df / 2>/dev/null\`" : '.*: *\([0-9]*\)'`

	#
	# Now see if the floppy fits!
	# Inform the user and exit immediately, if not.
	#

	if
		[ "$USRspace" -lt "$USRneeds" ]
	then
		echo "**ERROR**   ${PKGNAME} cannot be installed --"
		echo "Not enough space on the hard disk."
		echo "There are $USRspace blocks available"
		echo "on the /usr file system --"
		echo "$USRneeds blocks are needed."
		exit
	elif
		[ "$ROOTspace" -lt "$ROOTneeds" ]  
	then
		echo "**ERROR**   ${PKGNAME} cannot be installed --"
		echo "Not enough space on the hard disk."
		echo "There are $ROOTspace blocks available"
		echo "on the / (root) file system --"
		echo "$ROOTneeds blocks are needed."
		exit
	fi

	cd ${INSPATH}
	echo "The following files are being installed:"
	find . -print | cpio -pduvm /
	if [ "${VARPATH}" != "" ]
	then
		cd ${VARPATH}
		if [ `ls -a | wc -l` -ne 2 ] #if not an empty directory
		then
			find . -print | cpio -pduvm /
		fi
	fi

	#
	# Increment number for next expected floppy.
	#

	echo "Floppy diskette number $expect is complete"
	expect=`expr $expect + 1`
done

#
# Check to see if we still need to install the sys header package
#

if [ "${NEEDSYS}" = "yes" ]
then
	GOTPKG=no
	REL=`uname -r`
	while [ "${GOTPKG}" = "no" ]
	do
		cd /
		/etc/umount ${mntdev} 2>/dev/null
		echo "Remove floppy and insert the System Header Floppy for Release ${REL}"
		echo "Type <return> when ready:  \c"
		read answer
		/etc/mount ${mntdev} ${mntname} -r 2>/dev/null

		#
		# Check to make sure system header file package was inserted
		#
		while [ ! -r ${INSPATH}/usr/options/sys.name ]
		do
			echo "\n** ERROR **  Wrong Package Inserted"
			echo "Expecting the System Header Package"
			cd /
			/etc/umount ${mntdev} 2>/dev/null
			echo "Remove floppy and insert the System Header Floppy for Release ${REL}"
			echo "Type <return> when ready:  \c"
			read answer
			/etc/mount ${mntdev} ${mntname} -r 2>/dev/null
		done

		#
		# We now have a system header package -- check to make
		#	sure it matches the current release
		#
		PREL=`expr "\`cat ${INSPATH}/usr/options/sys.name\`" : '.*Release[ 	]*\([^ 	]*\).*'`
		if [ "${REL}" != "${PREL}" ]
		then
			answer=""
			while [ "${answer}" != y -a "${answer}" != n ]
			do
				echo "\n** ERROR ** Expecting the package for Release ${REL}"
				echo "and got the headers for Release ${PREL}"
				echo "Type 'y' if you want to continue installing the ${PREL} header files."
				echo "Type 'n' is you do not want to continue installation of this package"
				echo "NOTE: If you type 'y' you will STILL be prompted"
				echo "    to install the correct (${REL}) headers"
				echo "\ncontinue?   \c"
				read answer
				case ${answer} in
					n)	DIFFREL=no
						;;
					y)	DIFFREL=yes
						;;
				esac
			done
			if [ "${DIFFREL}" = "yes" ] #install sys headers that
			then                        # don't match the Release
				cp ${INSPATH}/../install/INSTALL /tmp/$$INStall
				chmod +x /tmp/$$INStall
				/tmp/$$INStall "OTHER"
				rm /tmp/$$INStall
			fi
				
		else
			cp ${INSPATH}/../install/INSTALL /tmp/$$INStall
			chmod +x /tmp/$$INStall
			/tmp/$$INStall "OTHER"
			rm /tmp/$$INStall
			GOTPKG=yes
		fi
	done
fi


#
# Give user closing message
#

echo "Installation of the ${PKGNAME} is complete."
exit
