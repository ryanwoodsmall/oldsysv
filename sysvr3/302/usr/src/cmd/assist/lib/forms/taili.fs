#ident	"@(#)forms.files:taili.fs	1.4"
00000tail
00701060101FILE NAME:
01703060001INTERACTIVELY FOLLOW FILE CONTENTS [-f] (y/n):
02705060030ADDITIONS TO THE COMMAND LINE:

00050
01041
02062

Since you selected the interactive "-f" (follow) option, 
"tail" will continue to follow the file contents until 
you hit the BREAK key.01yY

The "tail" command is used to print out selected portions of a
file.  If you select the "-f" option, the interactive usage 
will be activated.  Be sure to read the item help message 
for that field before executing the command form.  The "follow"
option is useful when you want to check on the progress of
another program that is writing into a file.

00Enter here the name of the file whose contents you
wish to display.  
01By default, the "tail" command prints out the requested portions
of the file, and then exits.  At this point, you would be returned
to ASSIST (or your UNIX system prompt if you executed the command
from there).  If you select this option, the "tail" command will
not terminate after printing the appropriate portions of the file.
It will continue to check the file, and print out any new portions
of the file, as they are added to the file.  This option is useful
when some other command (or user) is adding to a file.  You can
use the "-f" option to see the effect of that other command or user.
If you select this option, "tail" will become an interactive command,
and you will have to terminate the command with ^D.  [-f]
02ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.


0101

010353n

0002
003420001010
0116
0119
0104yYnN


001110
010001y-fY-fnN
021110



