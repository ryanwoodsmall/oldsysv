#ident	"@(#)forms.files:cpioo.fs	1.2"
00000cpio -o
00701060001REDIRECT INPUT FROM:
01703060001REDIRECT OUTPUT TO:
02705060001WRITE HEADER IN ASCII CHARACTERS [-c] (y/n):
03707060001USE 5,120 BYTE BLOCKS [-B] (y/n):
04709060001PROVIDE LIST OF FILE NAMES [-v] (y/n):
05711060001RESET ACCESS TIMES [-a] (y/n):
06713060030ADDITIONS TO THE COMMAND LINE:

00050
01060
02012c
03012B
04012v
05012a
06072

Since you did not redirect input, the "cpio" command
will expect you to provide a list of file names.
Hit ^D to indicate to "cpio" when you have completed
your input.00

"cpio -o" expects a list of file names from standard
input.  "cpio -o" places the contents of the named
files on standard output.  The usual way you would
use this command would be to redirect output to a
storage device or file.  This redirected output is
called a "cpio archive file".  You could then use
the "cpio -i ..." command to recreate the original
files on a different system or location.

00This command expects a list of path names on standard input.
If you have a file containing the list of path names, enter
that file name here.  If you don't enter a file name in
this field, the "cpio" command will expect you to enter
the list from your keyboard.
01By default, the output of this command is placed on standard
output.  Most of the time, you will want to redirect output
to a file or external storage device (e.g. magnetic tape).
If so, use this field to enter the file or device name.
02This option should always be selected when you plan to
move the cpio module from one machine to another.  It
causes the header information for each file to be in
ASCII character form, which permits portability. [-c]
03This option places 5120 bytes to the record on the output
device.  This option only makes sense when directing output
to a character-special device (e.g. tape drives). [-B]
04This option will cause a list of file names to be printed
on your terminal. [-v]
05By default, the time of last access will be updated for each
file copied by the "cpio" command.  If you do not want the
access time to be updated, select this option.  You may use
the "-lu" option of the "ls" command to see the last access
time of a file. [-a]
06ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



020551n
030740n
040945n
051137n

0002
003420001010
0107
0103
013410101010
0204yYnN
0304yYnN
0404yYnN
0504yYnN


001010< 
011010> 
020001ycYcnN
030001yBYBnN
040001yvYvnN
050001yaYanN
061110



