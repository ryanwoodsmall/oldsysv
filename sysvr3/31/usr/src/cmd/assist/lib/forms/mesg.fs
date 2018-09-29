#ident	"@(#)forms.files:mesg.fs	1.6"
00000mesg
00701060001ALLOW WRITING TO TERMINAL [-y] (y/n):
01703060001PREVENT WRITING TO TERMINAL [-n] (y/n):
02705060030ADDITIONS TO THE COMMAND LINE:

00010/bin/mesg|/usr/bin/cut -c4
01000
02032


"mesg" can be used in two ways.  First, you can
use it to prevent or allow users to "write" to
your terminal.  Second, it can tell you whether
users are currently able to "write" to your
terminal.  If you want users to be able to write to
your terminal, enter a "y" in the first field.  If 
you want to prevent such writing, enter a "y" in the 
second field.  You may not enter a "y" in both fields.  
If you want to know the current state of your terminal, 
do not enter a value in either of the first two fields.

00If you want to allow users to write to your terminal, enter a "y"
in this field.  If you want to know the current state of your 
terminal, do not enter a value for either this field or the next. [-y]
01Enter a "y" here if you want to prevent users from writing
to your terminal.  If you merely want to find out whether
users are currently able to write to your terminal, do not
enter a value for this or the previous field. [-n]
02ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0001
0100


000144n
010346n

0005
0004yYnN
0105
0104yYnN


001011y-yY-ynN
011011y-nY-nnN
021110



