#ident	"@(#)forms.files:cpioi.fs	1.4"
00000cpio -i
00701060001REDIRECT INPUT FROM:
01703060030FILE NAME SELECTION PATTERN(S):
02705060001READ HEADER AS ASCII CHARACTERS [-c] (y/n):
03707060001USE 5,120 BYTE BLOCKS [-B] (y/n):
04709060001CREATE DIRECTORIES AS NEEDED [-d] (y/n):
05711060001PRINT TABLE OF CONTENTS ONLY [-t] (y/n):
06713060001PROVIDE LIST OF FILE NAMES [-v] (y/n):
07715060001RETAIN PREVIOUS FILE MODIFICATION TIMES [-m] (y/n):
08717060001INTERACTIVELY RENAME FILES [-r] (y/n):
09719060001COPY FILES UNCONDITIONALLY [-u] (y/n):
10721060001COPY ALL FILES EXCEPT THOSE IN "PATTERN" [-f] (y/n):
11723060001SWAP BYTES IN EACH HALF WORD [-s] (y/n):
12725060001SWAP HALFWORDS IN EACH WORD [-S] (y/n):
13727060001REVERSE BYTES IN EACH WORD [-b] (y/n):
14729060001FILES ARE UNIX SYSTEM SIXTH EDITION [-6] (y/n):
15731060030ADDITIONS TO THE COMMAND LINE:

00150
01140
02012c
03012B
04012d
05012t
06012v
07012m
08012r
09012u
10012f
11012s
12012S
13012b
140126
15162

You have not redirected input from standard input, so the
"cpio" command will expect input from your keyboard.  Use
^D to indicate to "cpio" when you have finished your input.00

"cpio -i" takes an archive file that was created
using "cpio -o", and recreates the constituent files.
"cpio -i" expects the contents of the archive file
to come from standard input, so you will almost
always be redirecting standard input from a file or
storage device.  The storage device or file should
contain the output that you created using "cpio -o".
In this way, you can transfer files from one system
or location to another.

00The "cpio -i" command expects input from standard input.
However, this command is rarely used in this way.  Most
of the time, input is redirected from a file or an external
storage device (a tape drive or disk, for example).  Enter
here the name of your file or device that contains the result
of your "cpio -o" command, that you now want to read in.
01By default, all files in the "cpio" module will
be created.  If you want to limit creation to certain
files, you may specify them here.  You may enter file
names, in which case only those files will be created.
You may also enter meta-characters (*, ?, [...]).
However, you should enclose any strings containing
meta-characters in double quotes (").  Since file
names may be stored in "cpio" files as full pathnames,
your patterns must take this into account (e.g. */fname
will match all files ending in "fname").
02Select this option if you used the "-c" option when
you issued the "cpio -o" command that is serving as
input this command.  By selecting the option,
you are indicating that headers were written in ASCII
characters, and instructing the "cpio" command that
the headers should be read in the same way. [-c]
03Select this option if you used the "-B" option when
you issued the "cpio -o" command that is serving as
input this command.  By selecting the option,
you are requesting that input is to be blocked 5120
bytes to the record. [-B]
04If the "cpio -o" command output that is being used
as input for this command contains sub-directories,
you should select this option.  By selecting this
option, you are instructing the command that sub-directories
should be created as needed.  Since you may not know
whether the input contains sub-directories, it might
be a good idea to always select this option. [-d]
05If you select this option, no input files will be created.
Instead, you will merely receive a list of file names in
the "cpio" module.  You can use this option to see what
files would be created.  If you select this option and the
"-v" option, you will receive a listing that is similar
to the "ls -l" command. [-t]
06By itself, this option produces a list of file names.
If used with the "-t" option, the command will produce 
a list of input files that looks similar to the output 
of the "ls -l" command. [-v]
07By default, the file modification time of created files
will be the current time.  If you want the created files
to have the same last modification time as the source
files, select this option. [-m]
08By default, created files are given the same name as
the source (e.g. "cpio -o") files.  If you want to
interactively rename files, select this option.  For
each file, you will be prompted for a name.  If you do
not enter a file name for a given file (i.e. just hit
the RETURN key), that file will be skipped, and not created. [-r]
09By default, a file will not be created if an existing
file with the same name is newer (i.e. modified more
recently).  You may override this default by selecting
this option, in which case, files will be created regardless
of modification times. [-u]
10By default, all of those files that match the pattern
you specify are copied.  You can reverse this by selecting
this option.  In this case, only those file names that do
not match the pattern will be created. [-f]
11Select this option if you wish to swap bytes within
each half word.  You may have to use this option when
reading in a "cpio" module from a machine that has
a different internal representation of words. [-s]
12Select this option to swap half words within each word.
You may have to use this option when reading in a "cpio"
module from a machine with a different internal representation
of words. [-S]
13Select this option to reverse the order of bytes in
each word.  You may have to use this option if your
"cpio" module was created on a machine that has a
different internal representation of a word. [-b]
14Select this option if the "cpio" module was created on
a machine running the Sixth Edition of the UNIX system. [-6]
15ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0305
0405
05040708091012130311
0705
0805
0905
1005
11051213
120511
130511


020550n
030740n
040947n
051147n
061345n
071558n
081745n
091945n
102159n
112347n
122546n
132745n
142954n

003420001010
0126
0204yYnN
0304yYnN
0305
0404yYnN
0405
0504yYnN
0505
0604yYnN
0704yYnN
0705
0804yYnN
0805
0904yYnN
0905
1004yYnN
1005
1104yYnN
1105
1204yYnN
1205
1304yYnN
1305
1404yYnN


001010< 
011110
020001ycYcnN
030001yBYBnN
040001ydYdnN
050001ytYtnN
060001yvYvnN
070001ymYmnN
080001yrYrnN
090001yuYunN
100001yfYfnN
110001ysYsnN
120001ySYSnN
130001ybYbnN
140001y6Y6nN
151110



