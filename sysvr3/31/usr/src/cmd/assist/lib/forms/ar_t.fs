#ident	"@(#)forms.files:ar_t.fs	1.1"
00000ar -t
00701060101ARCHIVE FILE:
01903060030PRINT TABLE OF CONTENTS OF THE ARCHIVE FILE
02704080130INCLUDING THE FOLLOWING NAMES IN THE TABLE:
03706060001REDIRECT OUTPUT TO:
04708060001GIVE A LONG LISTING OF ALL INFORMATION ABOUT FILE(S) [v] (y/n):
05710060001PLACE TEMPORARY FILES IN CURRENT WORKING DIRECTORY [l] (y/n):
06712060001FORCE REGENERATION OF THE ARCHIVE SYMBOL TABLE [s] (y/n):
07714060030ADDITIONS TO THE COMMAND LINE:

00050
02060
03070
04022
05032
06042
07082


"ar" with the "-t" option prints a table of
contents of the archive file.

00The archive file is the file that contains one or 
more files placed there by the "ar" command.  
02This field contains the names of files (in the archive file) that
you want in the table of contents. If you leave this field empty,
then "ar" will include all the files in the table of contents. 
03If you type a file name here, the output of this command will go to the
file.  ASSIST will warn you if the file already exists.  If the named
file exists, re- directing output to it will destroy the previous
contents of the file.  (See the Unix system walkthru for a more complete
discussion of redirection.)
04When this argument is used, "ar" will provide a long listing of all
the information about the files. [v] 
05Saying "y" here will cause "ar" to place its temporary files in the 
current directory rather than in the default temporary
directory. [l] 
06"ar" with the s argument will force regeneration of the archive symbol
table.  This option is useful in restoring the archive symbol table
after the "strip" command has been used on the archive table. [s]
07ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



020452all_files_in_archive_file
040870n
051068n
061264n

003420001010
0032grepout=`file $F00 | grep "archive"`; if test -z "$grepout"; then echo "$F00 is not an archive file"; fi
0303
0332if test -d $F03; then echo "$F03 IS A DIRECTORY";fi
0404yYnN
0504yYnN
0604yYnN


001110
011110
021110all_files_in_archive_file
031010> 
040001yvYvnN
050001ylYlnN
060001ysYsnN
071110



