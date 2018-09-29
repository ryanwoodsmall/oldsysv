#ident	"@(#)forms.files:ar_m.fs	1.2"
00000ar -m
00701060101ARCHIVE FILE:
01703060150FILE(S) TO BE MOVED IN THE ARCHIVE FILE:
02705060001POSITION MOVED FILE(S) BEFORE FILE NAMED [b]:
03707060001POSITION MOVED FILE(S) AFTER FILE NAMED [a]:
04709060001PROVIDE VERBOSE DESCRIPTION [v] (y/n):
05711060001PLACE TEMPORARY FILES IN CURRENT WORKING DIRECTORY [l] (y/n):
06713060030ADDITIONS TO THE COMMAND LINE:

00060
01070
02040
03050
04012
05032
06082


"ar" with the "-m" option moves files to different
positions within the archive file.

00The archive file is the file that contains one or 
more files placed there by the "ar" command.  
01This field contains the names of files that you want to
reposition within the archive file.
02By default, "ar" will move the file(s) you have named to the end
of the archive file.  Use this field if you want to specify
that the moved files should be positioned in your archive file
immediately before the file you name in this field. [b] 
03By default, "ar" will move the file(s) you have named to the end
of the archive file.  Use this field if you want to specify
that the moved files should be positioned in your archive file
immediately after the file you name in this field. [a]
04When this option is used, "ar" will give a verbose file-by-file 
description of the making of the new (rearranged) archive file from
the old archive file and the constituent files. [v]
05Saying "y" here will cause "ar" to place its temporary files in the 
current directory rather than in the default temporary
directory. [l]
06ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0203
0302


040945n
051168n

0002
0032/bin/sh $ASSISTLIB/forms/ar.val $F00
020114
0205
0305
030114
0404yYnN
0504yYnN


001110
011110
020010b 
030010a 
040001yvYvnN
050001ylYlnN
061110



