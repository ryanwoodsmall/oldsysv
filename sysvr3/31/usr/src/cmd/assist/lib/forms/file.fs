00000file
00701060030NAME OF FILE(S) OR DIRECTORY(S):
01703060001REDIRECT OUTPUT TO:
02705060001NAME OF FILE THAT CONTAINS A LIST OF INPUT FILES [-f]:
03707060001NAME OF FILE THAT CONTAINS DESCRIPTIONS OF FILE TYPES [-m]:
04009060000PRINT THE FILE THAT CONTAINS FILE
05710080001TYPES AND PRINT COLUMN HEADINGS [-c] (y/n):
06712060030ADDITIONS TO THE COMMAND LINE:

00050
01060> 
02040-f 
03010-m 
05021c
06072


"file" gives you a file type for every file name that
you list on the command line.  An example of a file 
type is an ascii character file.  "file" identifies the
file type by comparing the contents of the file to 
descriptions of different types of files in the 
/etc/magic file on your system. 

00Name the file that you would like "file" to type.
If you do not give a file name here, you must 
name a file for the -f option, or respond "y"
to the -c option.
01If you type a file name here, the output of this command will go to the 
file.  ASSIST will warn you if the file you type already exists.  
If it exists, redirecting output to it will destroy the previous 
contents of the file.  (See the UNIX system walkthru for a discussion
of redirection.) 
02Type the name of a file that contains a list of
file names.  "file" with this option will print a file type for 
each file. You must type a file name here, or use the -c
option, or give the name of a file that you want to have 
typed.  [-f].
03"file" normally uses the /etc/magic file to
type your file. With this option you can specify 
your own file for "file" to use for a description
of file types. You cannot use this option with
the -c option. [-m]
05If you select this option, "file" will print the file that 
contains a list of the possible file types.  "file"  will not type 
any files but instead it will print the file that is used for 
typing with column headings. This enables you to inspect the file
for formatting errors. You must select this option, -f, or name a file 
that you want typed.  You cannot use this option with any of the others.
06ASSIST appends anything that you type here to the command line.
You can use this field for "piping," "redirection" or to
execute a command in the "background."  However, since ASSIST 
does not perform any error checking on this field, you should
use it with caution.

0005
0205
0305
05030200


051052n

0005
003411011010
0031
0103
0107
0132if test -d $F01; then echo "$F01 IS A DIRECTORY";fi
0205
023420001010
0231
0305
033420001010
0505
0531
0504yYnN


001110
011010>
021010-f 
031010-m 
050001y-cY-cnN
061110

000502
020500
050200


