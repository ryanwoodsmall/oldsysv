00000pg
00701060030NAME OF INPUT FILE(S):
01702060001NUMBER OF LINES PER SCREEN:
02703060001START AT LINE NUMBER:
03704060001START AT FOLLOWING PATTERN:
04705060001STRING TO BE USED AS PROMPT INSTEAD OF DEFAULT PROMPT [-p]:
05706060001CLEAR SCREEN BEFORE DISPLAYING EACH PAGE [-c] (y/n):
06707060001PAUSE AT END OF EACH FILE [-e] (y/n):
07708060001SPLIT LINES THAT ARE LONGER THAN SCREEN WIDTH [-f] (y/n):
08709060001EXECUTE A COMMAND AS SOON AS A COMMAND LETTER IS ENTERED [-n] (y/n):
09710060001PRINT ALL MESSAGES AND PROMPTS IN STANDOUT MODE [-s] (y/n):
10712060030ADDITIONS TO THE COMMAND LINE:

00060
01010-24
02040+1
03052+
04020-p
05001c
06001e
07001f
08001n
09001s
10072-

If you use the standard input, you must type a
^D to exit the interactive mode.00-

pg allows examination of a file one screenful
at a time on a soft-copy terminal.  Each
screenful is followed by a prompt.  If you 
type a carriage return after the prompt ":",
 another page is displayed.  You can either type
in a file name, '-' to indicate standard input,
or omit the file name altogether.

00This is the name of the file to be looked at.
01This integer specifies the size of the window
that pg should use.  On a terminal containing
24 lines, the default window size is 23 lines.
02Start reading the file at this line number.
The default is line number 1.
03Start looking at the file at the line containing
the regular expression pattern specified by
this option.  A regular expression gives special meanings to
certain characters, like the '*' used by the shell.  These
characters are metacharacters.  When they are included in
a string, the string must be quoted.  If you want to specify
a multi-word pattern, you must escape each space by preceding
it with the ASSIST control character ^C.
04Use this string as a prompt instead of the
default string.  If the prompt string
contains a "%d", the first occurrence of
"%d" in the prompt will be replaced by the
current page number when the prompt is used.  The
default prompt string is ":". [-p]
05This option  is ignored if the parameter that
clears the screen is not defined for this
terminal type in the terminfo(4) database.
For detailed information about the terminfor(4)
database, refer to the AT&T 3B2 Computer
UNIX System V Release 2 Programmer's Reference
Manual. [-c]
06Normally, pg pauses at the end of each file.
This option causes pg not to pause at the end
of each file.  [-e]
07Normally, pg splits lines longer than the
screen width but some sequences of characters
in the text being displayed generate undesirable
results.  This option inhibits pg from splitting lines.  [-f]
08pg has its own set of commands which allow
further perusal, perform searches and modify
the perusal environment.  You type in these
commands when pg pauses after each screenful.
Normally, commands must be terminated by a
<cr> character.  This option causes the command
to execute as soon as a command letter is entered. [-n]
09This option causes pg to print all messages and
prompts in standout mode, usually inverse video.  [-s]
10Anything typed here will be appended to the command line.
This field can be used for "piping", "redirection", or to
execute a command in the "background."  However, since no
error checking is performed on this field, it should be
used with caution.

0203
0302


01023424
040566:
050659n
060744y
070864y
080975n
091066n

0013
0015-
0108
01240
0205
0208
02240
0305
0326
0504yYnN
0604nNyY
0704nNyY
0804yYnN
0904yYnN


001110
011010-24
021010+
031010+//
041010-p:
050001y-cY-cnN
060001n-eN-eyY
070001n-fN-fyY
080001y-nY-nnN
090001y-sY-snN
101110



