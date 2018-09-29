#ident	"@(#)forms.files:ar_x.fs	1.1"
00000ar -x
00701060101ARCHIVE FILE:
01703060130FILE(S) TO BE EXTRACTED:
02705060101PROVIDE VERBOSE DESCRIPTION [v] (y/n):
03707060101PLACE TEMPORARY FILES IN THE CURRENT WORKING DIRECTORY [l] (y/n):
04709060101FORCE REGENERATION OF THE ARCHIVE SYMBOL TABLE [s] (y/n):
05711060030ADDITIONS TO THE COMMAND LINE:

00040
01050
02012
03022
04032
05072


"ar" with the "-x" option extracts one or more files
that are stored in an archive file. "ar" makes a copy,
in the current directory, of each extracted file.  The
extracted file also remains in the archive file.

00The archive file is the file that contains one or 
more files placed there by the "ar" command.  
01This field contains the names of files that you want to
extract from the archive file. A copy of each file you extract
will be put in the current directory.  A copy of the file
will also remain in the archive file.
02When this argument is used, "ar" will provide a verbose
description. [v]
03Saying "y" here will cause "ar" to place its temporary files in the 
current directory rather than in the default temporary
directory. [l]
04"ar" with the s argument will force regeneration of the archive symbol
table.  This option is useful in restoring the archive symbol table
after the "strip" command has been used on the archive table. [s]
05ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



010331all_files_in_archive_file
020545n
030772n
040964n

003420001010
0032grepout=`file $F00 | grep "archive"`; if test -z "$grepout"; then echo "$F00 is not an archive file"; fi
0204yYnN
0304yYnN
0404yYnN


001110
011110all_files_in_archive_file
020001yvYvnN
030001ylYlnN
040001ysYsnN
051110



