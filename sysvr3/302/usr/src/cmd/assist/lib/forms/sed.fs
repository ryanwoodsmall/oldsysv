#ident	"@(#)forms.files:sed.fs	1.6"
00000sed
00702060050INPUT FILE NAME(S):
01704060001REDIRECT OUTPUT TO:
02706060001SED COMMAND LINE SCRIPT [-e]:
03708060030NAME(S) OF FILE CONTAINING SED SCRIPT [-f]:
04710060001SUPPRESS DEFAULT OUTPUT [-n] (y/n):
05712060030ADDITIONS TO THE COMMAND LINE:

00040-
01050
02020-e 
03030-f 
04001n
05062

You have not entered a file for "sed" to act on.  The command will
expect you to enter input from standard input.  After executing this
command, enter ^D to indicate to the command that you are done
entering input. 00-

"sed" reads lines from a file, changes the lines
according to a script you supply, and puts out the
modified lines on standard output.  The actual
files are not changed.

00Enter the name(s) of your input file(s) here.  The "sed" command 
will act on each line in the file(s) as specified by your script, 
and then print the line on standard output.  The files are not changed.
01If you type a file name here, the output of this command will go to the 
file.  ASSIST will warn you if the file already exists.  If it exists, 
redirecting output to the file will destroy any previous contents.  
(See the UNIX system walkthru for a more complete discussion of 
redirection.) 
02Enter here your sed script(s) to modify input lines.  You should
enclose the script in quotes, to protect it from the shell.  ASSIST
will not validate this script in any way.  Several examples:
   's/junk/whatever/g'      substitutes whatever for junk
   '3,5p'                   prints lines 3, 4, and 5    [-e]
03Instead of (or in addition to) specifying your editing script on
the command line, you may create a file that contains the script.
If you have such a file that you wish to use, enter the file name here.
ASSIST will verify the existence of the file, but not the correctness of
the "sed" script it contains. [-f]
04By default, "sed" prints every input line it reads, regardless of the
script you provide.  If you want control over what gets printed, you
should select this option. The command will not print any lines,
unless your script includes the instruction to print a line. [-n]
05ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



041042n

0002
0013
000114
0103
0107
0231
0226
0302
0313
0331
0404yYnN


001110
011010> 
021010-e
031110-f
040001y-nY-nnN
051110

0203
0302


