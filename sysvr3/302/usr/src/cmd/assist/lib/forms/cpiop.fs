#ident	"@(#)forms.files:cpiop.fs	1.3"
00000cpio -p
00701060101DESTINATION DIRECTORY:
01703060001REDIRECT INPUT FROM:
02705060001CREATE DIRECTORIES AS NEEDED [-d] (y/n):
03707060001PROVIDE LIST OF FILE NAMES [-v] (y/n):
04709060001LINK FILES WHEN POSSIBLE [-l] (y/n):
05711060001RESET ACCESS TIMES OF INPUT FILES [-a] (y/n):
06713060001RETAIN PREVIOUS MODIFICATION TIME [-m] (y/n):
07715060001COPY FILES UNCONDITIONALLY [-u] (y/n):
08717060030ADDITIONS TO THE COMMAND LINE:

00080
01090
02002d
03002v
04002l
05002a
06002m
07002u
08102

You have not redirected input from standard input, so
the "cpio" command will expect you to provide a list
of path names from your keyboard.  Use ^D to signal
"cpio" when you have no more path names to enter.01

"cpio -p" command takes a list of file names or
entire directories and its sub-directories from
standard input, and places them in a new directory
you specify.  Unlike "cpio -o", which creates a
single archive file, "cpio -p" passes entire
directories and files to a new directory without
creating an archive file.

00Enter here the name of the directory in which the files
are to be copied.  You must have write and execute
permissions in the directory.
01By default, the "cpio -p" command expects a list
of path names on standard input.  You may redirect
input from a file, by entering the file name here.
The file should contain a list of path names, one
to a line.
02By default, sub-directories are not created.  If you
want sub-directories to be created as needed, select
this option.  It is probably a good idea to always
select this option, since you may not always know whether
sub-directories are needed. [-d]
03This option provides you with a list of files names
as they are created. [-v]
04By default, the files are copied to the new directory.
If you want the files to be linked instead, select this
option.  Files that cannot be linked (because they are
on a different file system) will be copied. [-l]
05By default, the last access time of the input files will
be updated (to the time of the copy).  If you do not want 
the access time of the original input files to be changed,
select this option. [-a]
06By default, when new files are created, their modification
time is the time at which they were created.  If you want
the new files to have the same "last modification time" as
the input files, select this option. [-m]
07By default, a file will not be copied if a file with the
same name exists in the destination directory, and that
file is newer than the file to be copied.  If you want the
copy to take place regardless of the relative newness of
the files, select this option. [-u]
08ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



020547n
030745n
040943n
051152n
061352n
071545n

003400000000
0002
0102
013420001010
010114
0204yYnN
0304yYnN
0404yYnN
0504yYnN
0604yYnN
0704yYnN


001110
011010< 
020001ydYdnN
030001yvYvnN
040001ylYlnN
050001yaYanN
060001ymYmnN
070001yuYunN
081110



