#ident	"@(#)forms.files:ar_d.fs	1.1"
00000ar -d
00701060101ARCHIVE FILE:
01703060150FILE(S) TO BE DELETED FROM THE ARCHIVE FILE:
02706060001PROVIDE VERBOSE DESCRIPTION [v] (y/n):
03708060001PLACE TEMPORARY FILES IN CURRENT WORKING DIRECTORY [l] (y/n):
04710060030ADDITIONS TO THE COMMAND LINE:

00030
01040
02012
03022
04052


"ar" with the "-d" (delete) option removes files from an
archive file.

00The archive file is the file that contains one or 
more files placed there by the "ar" command.  
01This field contains the names of files that you want to
delete from the archive file.  ASSIST does no validation
of this field.
02When this argument is used, "ar" will give a verbose file-by-file 
description of the altering of the archive file. [v]
03Saying "y" here will cause "ar" to place its temporary files in the 
current directory rather than in the default temporary
directory. [l]
04ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



020645n
030868n

0002
0032/bin/sh $ASSISTLIB/forms/ar.val $F00
0204yYnN
0304yYnN


001110
011110
020001yvYvnN
030001ylYlnN
041110



