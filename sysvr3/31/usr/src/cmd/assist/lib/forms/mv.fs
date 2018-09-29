00000mv
00701060030NAME OF INPUT FILES(S):
01702060001NAME OF OUTPUT FILE:
02704060001NAME OF INPUT DIRECTORY:
03705060001NAME OF OUTPUT DIRECTORY:
04707060001PROMPT FOR FILE/DIRECTORY WITH INCORRECT PERMISSIONS [-f] (y/n):
05709060030ADDITIONS TO THE COMMAND LINE:

00020
01040
02020
03040
04010-f
05062


"mv" moves the input files(s)/directory to the target output
file/directory.  Under not circumstances can the input name and
output name be the same.  If the target is a directory, the one or
more files are moved to that directory.  If the target is an
existing file, its contents are destroyed.  "mv" differs from "cp"
because it actually removes the file/directory from its current
location and puts it into the new location.  "cp" duplicates the
file in the new location without removing it from its current
location.

00List the names of files that you want to move.
When "mv" is executed, the file(s) name here
will be destroyed.  If you specify a filename
here, you cannot specify a directory in the
NAME OF INPUT DIRECTORY field.
01List the name of the file that is the destination for the file
you want to move.  If you specify more thatn one input file, then
you must type a directory name in the NAME OF OUTPUT DIRECTORY field.
If an existing file is given as the output file, then its contents
are destroyed.
02List the name of the directory you want to move.  You can only name one
directory at a time.  The input directory and the target directory must
have the same parent.  When "mv" is executed, the directory named here
will be destroyed.  If you specify a directory name here, you cannot
specify files in the NAME OF INPUT FILE(S) field.
03List the name of the output directory.  If you specify more than one
input file, then you must specify a directory as the output.  If you 
list a directory name here, you cannot specify a file name in the NAME 
OF OUTPUT FILE field.
04Normally, "mv" will ask questions before moving the file if the access
permissions are incorrect.  This option prevents those questions from
being asked. [-f]
05ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0002
010302
020001
0301


040771y

0013
0005
0031
003420001010
0113
0106
0105
0131
0107
0103
0210
0205
023401001010
0231
0307
0332if [ "`/bin/dirname $F02`" != "`/bin/dirname $F03`" ] ; then echo "INPUT AND TARGET DIRECTORIES MUST HAVE SAME PARENT"; fi;
0306
0305
0331
0312
0303
0404nNyY


001110
011110
021110
031110
041011n-fN-fyY
051110

0002
0103
0200
0301


