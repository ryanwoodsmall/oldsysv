#ident	"@(#)forms.files:ar_p.fs	1.2"
00000ar -p
00701060101ARCHIVE FILE:
01703060050FILE(S) TO BE PRINTED:
02705060001REDIRECT OUTPUT TO:
03707060001PROVIDE VERBOSE DESCRIPTION [v] (y/n):
04709060001PLACE TEMPORARY FILES IN THE CURRENT WORKING DIRECTORY [l] (y/n):
05711060001FORCE REGENERATION OF THE ARCHIVE SYMBOL TABLE [s] (y/n):
06713060030ADDITIONS TO THE COMMAND LINE:

00040
01050
02060
03012
04022
05032
06072


"ar" with the "-p" option prints one or more files
that are stored in the archive file.

00The archive file is the file that contains one or 
more files placed there by the "ar" command.  
01This field contains the names of files that you want to
print from the archive file.  If you leave this field empty,
then "ar" will print all the files in the archive file.
02If you type a file name here, the output of this command will go to the
file.  ASSIST will warn you if the file already exists.  If the named
file exists, re- directing output to it will destroy the previous
contents of the file.  (See the Unix system walkthru for a more complete
discussion of redirection.)
03When this argument is used, "ar" will give a verbose file-by-file 
description of the altering of the archive file. [v]
04Saying "y" here will cause "ar" to place its temporary files in the 
current directory rather than in the default temporary directory. [l]
05"ar" with the s argument will force regeneration of the archive symbol
table.  This option is useful in restoring the archive symbol table
after the "strip" command has been used on the archive table. [s]
06ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



030745n
040972n
051164n

003420001010
0032grepout=`/bin/file $F00 | /bin/grep "archive"`; if test -z "$grepout"; then /bin/echo "$F00 is not an archive file"; fi
010114
0203
0232if test -d $F02; then /bin/echo "$F02 IS A DIRECTORY";fi
0304yYnN
0404yYnN
0504yYnN


001110
011110
021010> 
030001yvYvnN
040001ylYlnN
050001ysYsnN
061110



