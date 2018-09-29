00000ln
00701060130NAME OF INPUT FILE(S):
01703060101NAME OF OUTPUT FILE/DIRECTORY:
02705060001PROMPT FOR FILE/DIRECTORY WITH INCORRECT PERMISSIONS [-f] (y/n):
03707060030ADDITIONS TO THE COMMAND LINE:

00020
01030
02010-f
03042


"ln" links the named input file(s) to the target output
file/directory.  Under no circumstances can the input name and output
name be the same.  If the target is a directory, then one or more files are
linked to that directory.  If the target is an existing file, its contents are
destroyed.  "ln" will not link across file systems.  

00List the names of files or the name of the directory
that you want to link.
01List the name of the file or directory that you want linked to your 
file(s).  If you specify more than one input file, then you must type
a directory name as the output.  If an existing file is given as the 
output, then its contents are destroyed.
02Normally, "ln" will ask questions before linking the file if the access 
permissions are incorrect.  This option prevents those questions from
being asked. [-f]
03ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



020571y

0013
003420001010
0107
0106
0204yYnN


001110
011010
021011n-fN-fyY
031110



