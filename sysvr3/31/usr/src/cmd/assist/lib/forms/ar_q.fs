#ident	"@(#)forms.files:ar_q.fs	1.1"
00000ar -q
00701060101ARCHIVE FILE:
01703060150FILE(S) TO BE APPENDED TO THE ARCHIVE FILE:
02706060101PROVIDE VERBOSE DESCRIPTION [v] (y/n):
03708060101SUPPRESS MESSAGE PRODUCED WHEN ARCHIVE FILE CREATED [c] (y/n):
04710060101PLACE TEMPORARY FILES IN CURRENT WORKING DIRECTORY [l] (y/n):
05712060030ADDITIONS TO THE COMMAND LINE:

00080
01090
02032
03040
04052
05992


"ar" with the "-q" option quickly appends a new file
to the end of an archive file.  "ar" with the "-q"
option does not check whether the added file is
currently in the archive file.  

00The archive file is the file that contains one or 
more files placed there by the "ar" command.  
01This field contains the names of files that you want to
append to the archive file.
02When this option is used, "ar" will give a verbose file-by-file 
description of the making of the new archive file from the old archive 
file and the constituent files.  [v]
03By default, "ar" prints a message when it creates an archive
file.  Say "y" in this field if you want to suppress this
message. [c]
04Saying yes here will cause "ar" to place its temporary files in the 
current directory rather than in the default temporary
directory. [l]
05ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



020645n
030869n
041068n

0032/bin/sh $ASSISTLIB/forms/ar.val $F00
013420001010
0204yYnN
0304yYnN
0404yYnN


001110
011110
020001yvYvnN
030001ycYcnN
040001ylYlnN
051110



