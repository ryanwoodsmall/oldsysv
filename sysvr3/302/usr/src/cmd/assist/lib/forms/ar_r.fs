#ident	"@(#)forms.files:ar_r.fs	1.2"
00000ar -r
00701060101ARCHIVE FILE:
01703060150FILE(S) TO BE ADDED OR REPLACED:
02005060030POSITION ADDED OR REPLACED
03706080001FILE(S) BEFORE FILE NAMED [b]:
04007060030POSITION ADDED OR REPLACED
05708080001FILE(S) AFTER FILE NAMED [a]:
06010060030REPLACE ONLY THOSE FILES WITH DATES OF
07711080001MODIFICATION LATER THAN THE ARCHIVE FILE [u] (y/n):
08713060001PROVIDE VERBOSE DESCRIPTION [v] (y/n):
09714060001SUPRESS MESSAGE PRODUCED WHEN ARCHIVE FILE CREATED [c] (y/n):
10715060001PLACE TEMPORARY FILES IN CURRENT WORKING DIRECTORY [l] (y/n):
11717060030ADDITIONS TO THE COMMAND LINE:

00080
01090
03060
05070
07022
08032
09042
10052
11102


"ar" with the "-r" option adds new files to an
archive file and replaces one or more files in
an archive file.  "ar" with the "-r" option will
create the named archive file if it does not exist.  

00The archive file is the file that contains one or 
more files placed there by the "ar" command.  
01This field contains the names of files that you want to
add to the archive file or replace in the archive file.
03
By default, "ar" will place the file(s) you adding or replacing
at the end of the archive file.  Use this field if you want to
specify that those files should be positioned in your archive file
immediately before the file you name in this field. [b]
05By default, "ar" will place the file(s) you adding or replacing
at the end of the archive file.  Use this field if you want to
specify that those files should be positioned in your archive file
immediately after the file you name in this field. [a]
07If you say "y" here, then only those files that have been modified
since the last modification of the archive file will be replaced. [u]
08When this option is used, "ar" will give a verbose file-by-file 
description of the making of the new (modified) archive file from the old
archive file and the constituent files. [v] 
09By default, "ar" prints a message when it creates an archive
file.  Say "y" in this field if you want to suppress this
message. [c]
10Saying "y" here will cause "ar" to place its temporary files in the 
current directory rather than in the default temporary
directory. [l]
11ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0305
0503


071160n
081345n
091468n
101568n

0032/bin/sh $ASSISTLIB/forms/ar.val $F00
013420001010
030114
0305
0505
050114
0704yYnN
0804yYnN
0904yYnN
1004yYnN


001110
011110
021110
030010b 
041110
050010a 
061110
070001yuYunN
080001yvYvnN
090001ycYcnN
100001ylYlnN
111110



