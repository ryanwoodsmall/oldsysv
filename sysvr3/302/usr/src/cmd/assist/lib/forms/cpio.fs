#ident	"@(#)forms.files:cpio.fs	1.1"
400cpio menu
00902210000CPIO FUNCTIONS
01705150101 .... create a new "cpio" file
02707150101 .... reconstitute an existing "cpio" file
03709150101 .... copy files to a directory

"cpio" has three distinct uses.  First, "cpio -o"
can take a list of file names, and display their
content on standard output.  By redirecting output,
you can create an archive and write it to external
storage or to a single file.  Second, "cpio -i" can
take an archive file coming from standard input,
and unarchive it into its constituent files.  This
form is often used to read in a tape made with an
earlier "cpio" command.  Finally, "cpio -p" can copy
a set of files and directories from one location to
another.

01Select this item if you want to create a new "cpio" file.
You might do this in order to transfer files to an external
storage device, or in order to mail the module to another
machine.  A "cpio" file is a single file that contains other files,
along with certain header information that is used by "cpio."
02Select this item if you have an existing "cpio" file, and
want to recreate the files in that "cpio" file.  The file should
either exist on your system, or by loaded on an external storage 
device (like a magnetic tape).
03Select this item to copy a group of files to a directory
on your system.

010507cpio -o
020707cpio -i
030907cpio -p

01cpioo
02cpioi
03cpiop

