#ident	"@(#)forms.files:Perm.fs	1.3"
400PERMISSIONS
00001180000PERMISSIONS OF FILES AND DIRECTORIES
01703160101.... Change permissions of files and directories
02704160101.... Change owner of files or directories
03705160101.... Encode or decode a file
04706150101..... Find a file
05707130101....... List contents of directory with file permissions options
06708160101.... Set code that changes permissions of a file when created

The Permissions of Files and Directories menu lists
commands that change or list the execute permissions
for your files or directories and encrypt files for
security purposes.

01
"chmod" changes the permissions of files.  You can
use it in symbolic or absolute mode.  In the symbolic
mode, you specify the permission by using a string
that indicates which permission should change; what
type of permission is involved; and whether this
permission is to be added, deleted, or set.  In the
absolute mode, you specify the permission by a three-
or four-digit octal code.
02
"chown" changes the owner of files and directories.
You name the new owner of file or directory using
the person's login name or numeric user ID.  You
must own the file or directory, or you must be
super user to use "chown".
03
"crypt" converts readable text into encrypted text
or unencrypts text that you encrypted using "crypt".
By default, input comes from standard input, and
output goes to standard output.
04
The purpose of "find" is to locate one or more files that satisfy 
conditions that the user specifies.  In the simplest case, the 
condition is that of having a specified name (e.g., assist.c).  
More complicated cases include, for example: the file must be
newer than a specified file, must have a specific permission
pattern, or must have been modified prior to a specified date.
05
"ls" lists contents of a directory or directories,
and can give detailed information about each file
listed.

The "ls" menu lists two versions of the "ls" command
form; a short "typical" use of "ls", and a complete
version that incorporates all of the options.
06
"umask" is used to remove permissions when a new
file is created.  It uses the same "absolute" mode
as the "chmod" command.  Thus, 022 will remove
write permission for "group" and "others".

010310chmod
020410chown
030510crypt
040610find
050710ls
060810umask

01chmod
02chown
03crypt
04find
05ls
06umask

