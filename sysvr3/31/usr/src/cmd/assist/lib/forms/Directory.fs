400DIRECTORY RELATED
00001220000DIRECTORY-RELATED COMMANDS
01703190101....... Change working directory
02704220101.... Change permissions of files and directories
03705220101.... Change owner of files and directories
04706190101....... Copy files
05707210101..... Copy file archives in and out of a file
06708230101... Compare directories
07709190101....... Link files or directories
08710190101....... List contents of directory
09711220101.... Make a directory
10712190101....... Move/rename files or directories
11713200101...... Print working directory name
12714190101....... Remove files and directories
13715220101.... Remove empty directories

The Directory-Related Commands menu lists commands
that create, remove, compare and change directories.

01"cd" allows you to change (that is, move to a different)
working directory.  This is the command to use for moving
around in the UNIX system hierarchical file system.  The "cd"
command form will allow you to change directories within
ASSIST, however, when you exit ASSIST, you will be in the
directory from which you invoked ASSIST.  While in ASSIST,
you may also change directories via the Pop-up menu (^F).
02"chmod" changes the permissions of files.  You can
use it in symbolic or absolute mode.  In the symbolic
mode, you specify the permission by using a string
that indicates which permission should change; what
type of permission is involved; and whether this
permission is to be added, deleted, or set.  In the
absolute mode, you specify the permission by a three-
or four-digit octal code.
03"chown" changes the owner of files and directories.
You name the new owner of file or directory using
the person's login name or numeric user ID.  You
must own the file or directory, or you must be
super user to use "chown".
04"cp" copies the named input file to the target
output file.  Under no circumstances can the input
file and the target file be the same.  If the target
is a directory, then one or more files are copied to
that directory.  If the target is an existing file,
its contents are destroyed.  If the target is not an
existing file, a new file is created.  The input file
cannot be a directory.  "cp" differs from "mv" because
"cp" duplicates the file in the new location without
removing it from its current location.  "mv" actually
removes the file/directory from its current location.
05"cpio" has three distinct uses.  First, "cpio -o"
can take a list of file names, and display their
content on standard output.  By redirecting output,
you can create an archive and write it to external
storage on to a single file.  Second, "cpio -i" can
take an archive file coming from standard input,
and unarchive it into its constituent files.  This
form is often used to read in a tape made with an
earlier "cpio" command.  Finally, "cpio -p" can copy
a set of files and directories from one location to
another.
06"dircmp" compares two directories and lists all
the files in them stating whether they are the
same or different.  Files are marked as the same
if they have the same name and identical contents.
07files by the same name in a different directory.
"ln" does not make copies of the files, it simply
enables you to access or change the contents of a
file by more than one name.  If you change the
file using either file name, the file will be
changed when you access it using either name.
You can delete the new file name, but the original
file will still exist.  If the new file name is an
existing file, its contents are destroyed.
08"ls" lists contents of a directory or directories,
and can give detailed information about each file
listed.

The "ls" menu lists two versions of the "ls" command
form; a short "typical" use of "ls", and a complete
version that incorporates all of the options.
09"mkdir" creates directories.  When it creates a
directory, it assigns read, write and execute
permissions to all those who have access to the
directory.  You must have write permission in a
directory to create sub-directories in it.
10"mv" moves an input file(s) or a directory to the
target or different file or directory.  If the input
is a directory, then one or more files in the direc-
tory are moved to that directory.  "mv" differs from
"cp" because it actually removes the file or directory
from its current location and puts it into the new
location.  "cp" copies the file in the new location
without removing it from its current location.  If you
"mv" a file to an existing file, the contents of the
existing file are destroyed.
11"pwd" prints the path name of the current working
directory.
12"rm" removes files.  To remove directories, and the
files and sub-directories in the directories, you
must use the "-r" option.
13"rmdir" is the remove directory command.  "rmdir"
will remove only empty directories.  To use "rmdir"
you must own a directory and have write permission
for it.  If you want to remove directories that
contain files and sub-directories, you must use
the "rm" command.

010316cd
020416chmod
030516chown
040616cp
050716cpio
060816dircmp
070916ln
081016ls
091116mkdir
101216mv
111316pwd
121416rm
131516rmdir

01cd
02chmod
03chown
04cp
05cpio
06dircmp
07ln
08ls
09mkdir
10mv
11pwd
12rm
13rmdir

