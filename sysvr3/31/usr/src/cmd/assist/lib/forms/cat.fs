#ident	"@(#)forms.files:cat.fs	1.4"
00000cat
00701060030NAME OF FILE(S):
01703060001REDIRECT OUTPUT TO:
02705060001MAKE NON-PRINTING CHARACTERS VISIBLE [-v] (y/n):
03707060001PRINT TABS AS ^I'S [-t] (y/n):
04709060001PLACE $ AT END OF EACH LINE [-e] (y/n):
05711060001BUFFER OUTPUT [-u] (y/n):
06713060001REPORT NON-EXISTENT FILES [-s] (y/n):
07715060030ADDITIONS TO THE COMMAND LINE:

00060
01070> 
02001v
03001t
04001e
05001
06001
07082

By not entering a file name, you have requested that the "cat" command
take input from your terminal.  To terminate input, type ^D.00-

"cat" prints the content of files on your terminal.
If you specify multiple files, "cat" prints each
in turn, thus concatenating the multiple files.

00Enter the name(s) of file(s) whose contents you want displayed.  
If you enter multiple files, the command will print each in the order
specified.  
01If you type a file name here, the output of this command will go to the 
file.  MUSE will warn you if the file you already exists.  If it exists, 
redirecting output to it will destroy the previous contents of the file.  
(See the UNIX walkthrough for a more complete discussion of redirection.) 
02By default, non-printing characters (e.g. control characters)
will not be visible when you print a file.  Enter a "y" in
this field if you want to make such characters visible. [-v]
03By default, tabs will appear as blank space on your terminal.
Select this option if you want tabs to be printed as ^I.  You 
must also select "MAKE NON-PRINTING CHARACTERS VISIBLE." [-t]
04If you want a dollar sign ($) to appear at the end of each output
line, enter a "y" in this field.  This option is useful to
see how many blanks follow the last visible character on a line.
You must also select "MAKE NON-PRINTING CHARACTERS VISIBLE." [-e]
05By default, "cat" buffers output, and for the vast majority of cases,
this is what you want.  You might select unbuffered output if you
wanted to merge the contents of a file with other text.  An example:
    (cat -u file_name; echo $HOME) | some_command        [-u]
06By default, "cat" will return an error message for any files
that it cannot find.  If you select this option, the command will
silently ignore non-existent files.  Since ASSIST checks that the 
files which you enter exist, you should never get such error 
messages in any case. [-s]
07ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.


0302
0402

020555n
030737n
040946n
051132y
061344y

0015-
0013
0103
0113
0219
0204YyNn
0316
0304yYnN
0416
0404yYnN
0504nNyY
0604nNyY


001110
011010> 
020001y-vY-vnN
030001y-tY-tnN
040001y-eY-enN
050001n-uN-uyY
060001n-sN-syY
071110



