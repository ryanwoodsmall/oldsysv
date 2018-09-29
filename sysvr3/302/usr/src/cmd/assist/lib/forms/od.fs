00000od
00701060001NAME OF INPUT FILE:
01702060001REDIRECT OUTPUT TO:
02704060001INTERPRET BYTES IN OCTAL [-b] (y/n):
03705060001INTERPRET BYTES IN ASCII [-c] (y/n):
04706060001INTERPRET WORDS IN UNSIGNED DECIMAL [-d] (y/n):
05707060001INTERPRET WORDS IN OCTAL [-o] (y/n):
06708060001INTERPRET 16-BIT WORDS IN SIGNED DECIMAL [-s] (y/n):
07709060001INTERPRET WORDS IN HEX [-x] (y/n):
08011020000FILE OFFSET OPTIONS
09712060001NUMBER OF OCTAL BYTES TO OFFSET:
10713060001NUMBER OF BLOCKS OF 512 OCTAL BYTES TO OFFSET:
11714060001NUMBER OF DECIMAL BYTES TO OFFSET:
12715060001NUMBER OF BLOCKS OF 512 DECIMAL BYTES TO OFFSET:
13717060030ADDITIONS TO THE COMMAND LINE:

00020
01040>
02001b
03001c
04001d
05001o
06001s
07001x
09030+
10030+
11030+
12030+
13042

If you use the standard input, you must type ^D twice to indicate
the end of your input.00-

The od command dumps a file in one or more formats as indicated
by the first argument.  If the first argument is missing, octal
words [-o] is the default.  If you do not provide an input file
name, the standard input is used.  If you want to start reading your
file at some location other than the beginning of the file, the
FILE OFFSET OPTIONS allow you to advance your starting point.

00The file argument specifies which file is to be dumped.  If no
file argument is specified, the standard input is used.
01This is the name of the file which contains the output generated
by the od command.  You will probably want to save the output
from  this command in order to study it in detail.
02This option interprets the bytes in octal.  The default mode
of interpretation is octal words. [-b]
03This option interprets bytes in ASCII.  The following non-graphic
characters appear as C escapes:  null=\\0, backspace=\\b,
newline=\\n, return=\\r, tab=\\t; others appear as 3-digit
octal numbers.  The default mode of interpretation is octal
words. [-c]
04This option interprets words in unsigned decimal.  The default
mode of interpretation is octal words. [-d]
05This option interprets words in octal.  If no format for dumping
is specified, octal is the default. [-o]
06This option interprets 16-bit words in signed decimal.  The default
mode of interpretation is octal words. [-s]
07This option interprets words in hexidecimal.  The default
mode of interpretation is octal words. [-x]
09The offset specified will be interpreted in octal bytes.  For example,
if you type in '10', the file will be offset by 10 octal bytes and
dumping will begin.
10The argument is interpreted as the number of blocks of 512 octal
bytes.  For example, if you type in '10', the file will be offset
by 10 blocks of 512 bytes and then dumping begins.
11The argument is interpreted as decimal bytes.  For example, if you
type '10', the file will be offset by 10 decimal bytes and dumping
will begin.
12The argument is interpreted as blocks of 512 decimal bytes.  For example,
if you type '10', the file will be offset by 10 blocks of 512 decimal
bytes and dumping will begin.
13ASSIST appends anything you type here to the command
line.  You can use this field for "piping," "redirection,"
or to execute a command in the "background."  However,
since ASSIST does not perform any error checking on
this field, you should use it with caution.

09111012
10091112
11091012
12091011


020443n
030543n
040654n
050743n
060859n
070941n

003410001010
0112
0101
0103
0204yYnN
0304yYnN
0404yYnN
0504yYnN
0604yYnN
0704yYnN
0905
0908
1005
1008
1105
1108
1205
1208


001010
011010>
020001y-bY-bnN
030001y-cY-cnN
040001y-dY-dnN
050001y-oY-onN
060001y-sY-snN
070001y-xY-xnN
091010+
101010+b
111010+.
121010+.b
131110



