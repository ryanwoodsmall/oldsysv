00000ls
00701060030NAME OF INPUT FILE(S)/DIRECTORY(S):
01702060001LIST NAMES BEGINNING WITH "." [-a] (y/n):
02703060001RECURSIVELY LIST SUBDIRECTORIES ENCOUNTERED [-R] (y/n):
03704060001LIST DIRECTORY NAME ONLY, NOT ITS CONTENTS [-d] (y/n):
04006030000ATTRIBUTE LIST	
05707060001LIST IN LONG FORMAT [-l] (y/n):
06708060001LIST IN LONG FORMAT, PRINT UID AND GID NUMBERS [-n] (y/n):
07709060001LIST IN LONG FORMAT, BUT DO NOT PRINT GROUP [-o] (y/n):
08710060001LIST IN LONG FORMAT, BUT DO NOT PRINT OWNER [-g] (y/n):
09012020000SORTING OPTIONS
10713060001REVERSE ALPHABETIC OR OLDEST FIRST [-r] (y/n):
11714060001TIME MODIFIED [-t] (y/n):
12715060001TIME OF LAST ACCESS [-u] (y/n):
13716060001TIME OF LAST i-NODE MODIFICATION [-c] (y/n):
14018030000OUTPUT FORMAT
15719060001MULTI-COLUMN WITH ENTRIES SORTED VERTICALLY [-C] (y/n):
16720060001MULTI-COLUMN WITH ENTRIES SORTED HORIZONTALLY [-x] (y/n):
17721060001STREAM OUTPUT [-m] (y/n):
18723060001APPEND "/" FOR DIRECTORIES, "*" FOR EXECUTABLE FILES [-F] (y/n):
19724060001APPEND "/" FOR DIRECTORIES [-p] (y/n):
20725060001PRINT NON-GRAPHIC CHARACTERS IN OCTAL/DDD NOTATION [-b] (y/n):
21726060001PRINT NON-GRAPHIC FILE NAME CHARACERS AS "?" [-q] (y/n):
22727060001PRINT i-NUMBER OF FILE IN FIRST COLUMN [-i] (y/n):
23728060001GIVE SIZE IN BLOCKS FOR EACH ENTRY [-s] (y/n):
24729060001TREAT EACH ARGUMENT AS A DIRECTORY [-f] (y/n):
25731060030ADDITIONS TO THE COMMAND LINE:

00030
01001a
02001R
03001d
05001l
06001n
07001o
08001g
10001r
11001t
12001u
13001c
15001C
16001x
17001m
18001F
19001p
20001b
21001q
22001i
23001s
24001f
25032-


'ls' lists contents of a directory or directories, and can give detailed
information about each listed file.  This is the complete command form
which presents all of the 'ls' options.  There is also a typical
'ls' command form which provides the options that are typically used.
The ATTRIBUTE LIST provides you with choices for information about the
files.  Choose an option from the SORTING OPTIONS group if you want to
order the list chronologically or reversed alphabetically.  Choose an
the OUTPUT FORMAT group if you want to arrange your output in columns
columns or in stream format.  To save the output in a file, use the
ADDITIONS TO THE COMMAND LINE field to redirect the output to a file.

00List any number of file or directory names here.
If you leave this blank, the current directory is listed.
01This option lists "." files.  For example, ".profile" is not normally 
listed.  If you use the -a option, ".profile" will be listed.  This option
is overridden by the -d option.  You should not use -a and -d 
together. [-a]
02This option lists files in directories given, but also in
their sub-directories, directories within sub-directories, etc. [-R]
03If an argument is a directory, this option lists
only its name, not its contents.  It is often
used with -l to get the status of a directory.  This option overrides the
-a option.  You should not use -a if you are using this option. [-d]
05This option lists for each file: access permissions, number of links, 
owner, group, size in bytes, and time of last modification.  It is 
incompatible with the -o, -g, -C, -x, and -f options.  It should not be 
used with them.  [-l]
06This option lists for each file: access permissions, number of links, 
owner's identification number instead of character string, group 
identification number instead of character string, size in bytes, and 
time of last modification.  It is incompatible with the -o, -g, -C, -x 
options and should not be used with them. [-n]
07This option lists for each file:  access permissions, number of links,
owner, size in bytes, and time of last modification.  The output is the 
same as for the -l option except that the group is not printed.  This 
option is incompatible with the -C, -x, -l, and -n options and should
not be used with them. [-o]
08This option lists for each file:  access permissions, number of links, 
group, size in bytes, and time of last modification.  The output is the 
same as for the -l option except that the owner is not printed.  This 
option is incompatible with the -C, -x, -l, and -n options and should 
not be used with them. [-g]
10This option reverses the order of sort to get reverse
alphabetic or oldest first, whichever is appropriate.
It is incompatible with the -c, -t, -u, and -f options
and should not be used with them. [-r]
11This option lists the files in the order in which they were
modified, with the most recently modifies files
listed first.  It is incompatible with the -r and -f options
and should not be used with them. [-t]
12This option lists the files in the order in which they were
accessed, with the most recently accessed files
listed first.  If you use this option, you should also
use the -l or -t option.  It is incompatible with the
-r and -c options. [-u] 
13This option lists the files in the order in which their
i-nodes were changed, with the most recently
changed files listed first.  If you use this option,
you should also use the -l or -t option.  It is
incompatible with -u and -r.  [-c]
15This option lists output in columns regardless of whether
sorting was alphabetic or chronological.  It is incompatible 
with the -l, -n, -o -g, -x, and -m options. [-C]
16This option lists output in rows regardless of whether
sorting was alphabetic or chronological.  It is incompatible
with the -C, -l, -n, -o, -g, and -m options and should not
be used with these options.  [-x]
17This option lists output as a constant stream, separated
by commas, across the page/screen.  It is incompatible with
the -C and -x options and should not be used with them.  [-m]
18This option provides a convenient way to obtain some
basic file information, such as which items
are directories or which files are
executable, without using the long
format.  This option is incompatible with the -p
option and should not be used with it. [-F]
19This option provides a convenient way to find directories
without using the long format.  It is incompatible with
the -F option and should not be used with it. [-p]
20This option forces printing of non-graphic characters to be
in the octal/ddd notation.  A non-graphic character is one
that does not appear printed on the screen.  For example,
a control character such as ^Z.  It is incompatible with
the -q option and should not be used with it.  [-b]
21This option forces printing of non-graphic characters in file
names as the character"?".  A non-graphic character is one
that does not appear printed on the screen.  For example,
a control character such as ^Z.  It is incompatible with
the -b option and should not be used with it.  [-q]
22For each file, print the i-number in the first column
of the report.  The i-number is the system's internal
name for the file.  [-i]
23This option gives the size in blocks, including indirect blocks.
Blocks are stored hierarchically.  A given block has a certain
number of other blocks at the same "level".  It also "points" to
blocks at the next level down; these are indirect blocks.  Since a
given object may be stored across blocks at different levels, all
blocks should be included in a size count.  This option is
incompatible with the -f option and should not be used with it. [-s]
24Force each argument to be interpreted as a
directory and list the name found in each
slot.  This option turns off -l, -t, -s,
and -r, and turns on -a: the order is the
order in which entries appear in the directory.  [-f]
25ASSIST appends anything you type here to the command line.  You can
use this field for "piping," "redirection," or to execute a command 
in the "background."  However, since ASSIST does not perform any error
checking on this field, you should  use it with caution.

0103
0203
030102
0507081516240617
060708151605
0715160506
0815160506
10121324
1124
121310
131210
15070816050617
16150708050617
17151605
1819
1918
2021
2120
2324
2423051011


010248n
020362n
030461n
050738n
060865n
070962n
081062n
101353n
111432n
121538n
131651n
151962n
162064n
172132n
182371n
192445n
202569n
212663n
222757n
232853n
242953n

0014
0104yYnN
0105
0204yYnN
0205
0304yYnN
0305
0504yYnN
0505
0604yYnN
0605
0704yYnN
0705
0804yYnN
0805
1004yYnN
1005
1104yYnN
1105
1204yYnN
1205
1304yYnN
1305
1504yYnN
1505
1604yYnN
1605
1704yYnN
1705
1804yYnN
1805
1904yYnN
1905
2004yYnN
2005
2104yYnN
2105
2204yYnN
2304yYnN
2305
2404yYnN
2405


001110
010001y-aY-anN
020001y-RY-RnN
030001y-dY-dnN
050001y-lY-lnN
060001y-nY-nnN
070001y-oY-onN
080001y-gY-gnN
100001y-rY-rnN
110001y-tY-tnN
120001y-uY-unN
130001y-cY-cnN
150001y-CY-CnN
160001y-xY-xnN
170001y-mY-mnN
180001y-FY-FnN
190001y-pY-pnN
200001y-bY-bnN
210001y-qY-qnN
220001y-iY-inN
230001y-sY-snN
240001y-fY-fnN
251110



