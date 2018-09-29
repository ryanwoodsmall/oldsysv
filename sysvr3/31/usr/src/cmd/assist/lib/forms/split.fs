#ident	"@(#)forms.files:split.fs	1.5"
00000split
00702060001INPUT FILE NAME:
01704060001OUTPUT FILE PREFIX:
02706060001NUMBER OF LINES PER FILE [-]:
03708060030ADDITIONS TO THE COMMAND LINE:

00020
01030x
02010-1000
03042-

Since you have not entered an input file name (or entered
a hyphen), the "split" command will expect input to come 
from your terminal.  Use ^D to indicate to the command 
that you have completed your input.00-

"split" divides a file into a number of smaller files.
The first "n" lines of the input file are placed in
the first output file, the second "n" lines are placed
in the second, etc.  By default, "n" is 1000 lines,
but you may specify a different value.  The input file
is not changed.

00Enter the name of the file which you want to split.  If you leave
this field blank, or enter a hyphen, then input is read from
standard input.  In this case, you will have to type ^D to terminate
the command.  Input is required here only if you specify an output
file prefix.  The input file is not altered by this command.
01The input file will be split into one or more output files.  The first
output file will end with "aa," the second will end with "ab," etc.
By default, the file prefix will be "x."  Therefore, the first file will
be called "xaa," the second will be "xab," etc.  If you want to specify
a prefix other than "x," enter it here (must be less than 13 characters.)
If you enter a value for this field, you must also supply an input
file name.
02By default, the input file is split into 1000 line output files (except
the last which may be less than 1000 lines).  If you want the size of
the output files to be other than 1000, enter the number of lines per
file here. [-]
03ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.


0100

010426x
0206361000

000114
0015-
003420001010
011601
010112
0208
0232if [ $F02 -eq 0 ];then echo "NUMBER OF LINES MUST BE GREATER THAN ZERO";fi


001110
011010x
021010-1000
031110



