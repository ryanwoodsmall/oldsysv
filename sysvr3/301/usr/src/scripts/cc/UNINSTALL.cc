##ident	"@(#)pkg.cc:UNINSTALL.cc	1.12"
#
# UNINSTALL a 3B2 Package
#
# Save mount point of floppy (INSPATH) and package name (PKGNAME)
#

INSPATH=/install/new
CCMESG=/install/install/cc.mesg
PKGNAME=`cat ${INSPATH}/usr/options/cc.name`

# For the CC package: Only want to remove the files that we installed
# for this release.
# So, we must decide whether this is a 1.1, 1.2, or 2.1 system.
# If not one of the above systems then the sys headers were added from
# a seperate package and should be uninstalled using that package.

case `uname -r` in
	2.0)	VRlist=/install/install/Rlist1.1
		;;
	2.0.?)	VRlist=/install/install/Rlist1.2
		;;
	2.1.?)	VRlist=/install/install/Rlist2.1
		;;
	*)	VRlist=""
		;;
esac

#
# Remove the files found in the Rlist file from the hard disk.
#

echo "Removing the ${PKGNAME}."
echo "The following files are being removed:"
for i in `cat /install/install/Rlist`
do
	echo $i
	rm -fr $i
done
if [ -n "${VRlist}" ]
then
	for i in `cat ${VRlist}`
	do
		echo $i
		rm -fr $i
	done
fi

echo "The ${PKGNAME} has been removed."
