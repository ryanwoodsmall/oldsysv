#ident	"@(#)forms.files:du.fs	1.3"
00000du
00701060030DIRECTORY AND/OR FILE NAME(S):
01703060001GRAND TOTALS (SHORT REPORT) ONLY [-s] (y/n):
02705060001ALSO LIST FILES WITHIN DIRECTORIES [-a] (y/n):
03707060001REPORT UNREADABLE FILES AND DIRECTORIES [-r] (y/n):
04709060030ADDITIONS TO THE COMMAND LINE:

00040-
01021s
02011a
03031r
04052-


"du" is used to obtain the number of disk blocks
used by files and directories.  If you specify no
directories or files, "du" will list the usage,
the current directory, and any sub-directories.

00Enter any specific directories and/or files of interest.  If you
leave this field blank, the current directory is used.  If you
enter the names of any files in this field, you must select the
"ALSO LIST FILES WITHIN DIRECTORIES" option.
01By default, sub-directories are also listed.  If you want to limit
output to the specified directories (or the current directory if
none is specified), enter a "y" in this field. [-s]
02By default, disk usage for files within directories are not listed.
If you want to also see usage for each file, enter a "y" here.
You must also select this option if you specified any files on the
"DIRECTORY AND/OR FILE NAME(S)" field. [-a]
03By default, any non-existent or unreadable files or directories 
are silently ignored.  If you want a message when the directory/file 
is missing or unreadable, select this option. [-r]
04ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0102
0201


010351n
020553n
030758n

000114
0002
0104yYnN
0105
0204yYnN
0205
0304yYnN


001110
010001y-sY-snN
020001y-aY-anN
030001y-rY-rnN
041110



