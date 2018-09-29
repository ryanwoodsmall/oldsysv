00000pr
00701070050NAME OF INPUT FILE(S):
01903030000GENERAL FORMAT
02704070001DOUBLE-SPACE THE OUTPUT [-d] (y/n):
03705070001NUMBER OF CHARACTERS TO INDENT EACH LINE [-o]:
04706070001NUMBER OF LINES PER PAGE [-l]:
05707070030PAGE HEADER STRING TO APPEAR WITH DATE AND TIME [-h]:
06708070001SUPPRESS HEADER AND TRAILER LINES FOR EACH PAGE [-t] (y/n):
07709070001BEGIN PRINTING WITH PAGE NUMBER:
08911030000LINE NUMBERED OUTPUT
09712070001NUMBER THE OUTPUT LINES [-n] (y/n):
10713070001SEPARATOR CHARACTER BETWEEN LINE NUMBERS AND FILE CONTENTS:
11714070001MAXIMUM NUMBER OF CHARACTERS ALLOWED FOR LINE NUMBERS:
12916030000PRINTING CONTROLS
13717070001IGNORE DIAGNOSTIC MESSAGES [-r] (y/n):
14718070001PAUSE BEFORE PRINTING EACH PAGE ON TERMINAL [-p] (y/n):
15719070001USE FORM-FEED FOR NEW PAGES, PAUSE BETWEEN PAGES [-f] (y/n):
16921030030INTERPRETATION OF TABS FOR INPUT AND OUTPUT FILES
17722070001INPUT FILE TAB WIDTH [-e]:
18723070001INPUT FILE TAB CHARACTER [-e]:
19724070001OUTPUT FILE TAB WIDTH [-i]:
20725070001OUTPUT FILE TAB CHARACTER [-i]:
21927030000MULTI-COLUMN OUTPUT
22728070001PRINT MULTIPLE INPUT FILES, ONE PER COLUMN [-m] (y/n):
23729070001NUMBER OF COLUMNS TO PRINT INPUT FILES [-]:
24730070001NUMBER OF CHARACTERS PER LINE [-w]:
25731070001CHARACTER THAT SEPARATES COLUMNS [-s]:
26732070001PRINT OUTPUT ACROSS PAGE, 1 LINE PER COLUMN [-a] (y/n):
27734030030ADDITIONS TO THE COMMAND LINE:

00220
02001
03180
04190
05210
06001
07140
09150
10162
11170
13001
14001
15001
17070
18062
19090
20082
22100
23100
24120
25202
26130
27232

If you typed "-" as your input or didn't provide an
input filename, "pr" puts you into an interactive mode.
To exit this mode and execute the command, you must
type control-d (^D).00-

"pr" prints files on your terminal.  By default,
the file is printed separated into pages.  Each
page is headed by a page number, the date and
time, and the name of the file.  This is the
complete command form for "pr."  It contains all
of the options available for "pr."  There is
also a short, "typical" command form which
contains the most frequently used options.

00List here the name of the file(s) you want to print.  If you
specify "-" or don't list any file name(s), "pr" assumes the
standard input.
02This option double-spaces the output.  Blank lines that result
from double-spacing are dropped when they occur at the top of
a page. [-d]
03This option indents the output.  The indent is added
to the standard left margin.  Specify how many spaces
you want your output indented.  The default is 0. [-o]
04This option sets the page length.  If you specify a page length
of less than 10 lines, the header and trailer are suppressed.
Normally, "pr" uses a header of 5 lines and a trailer of 5 lines.
If you specify a page length of greater than 10 lines, "pr"
subtracts 10 lines to allow for header and trailer.  This leaves
you (length-10) for your text.  The default page length is 66
lines. [-l]
05This option allows you to specify text which replaces the file name
in the header printed at the top of each page.  It is incompatible
with -t and with -l when the page length is 10 or less. [-h] 
06This option suppresses the header and trailer normally printed
on each page.  It is incompatible with -h. [-t]
07Type the page number at which you want to start
printing.  The default is page one.
09If you want to number the lines of your output, type "y".
The line numbers appear at the beginning of each line. [-n]
10If you want to separate the line numbers from your text,
enter the single, non-digit character you would line to
use.  The default character is the TAB character.  (There
will be approximately 8 spaces between your numbers and
text in the default case.)  If you choose this option, you
must also choose -n.
11This option specifies the maximum number of digits your line
number can contain.  The default is 5 (99999 is the
highest line number).  If you choose this option,
you must also choose -n.
13NAME OF INPUT FILE(S):
14This option pauses before beginning each page if
the output is directed to a terminal.  "pr" will
ring the bell at the terminal and wait for a
carriage return. [-p]
15NAME OF INPUT FILE(S):
17If you want to expand the number of character positions between
input tabs, type in this field the number of positions you
would like between tab settings.  The default is 8. [-e]
18If you want to separate your input file columns, enter the single,
non-digit character you would like to use.  The default separator
character is the TAB character.  [-e]
19If you want to replace white space in your output with tab settings,
choose this option.  Type in this field the number of character
positions you want between tab settings.  The default is 8. [-i]
20If you want to separate your output columns, enter the single
non-digit character that you would like to use.  The default
character is the TAB character.  [-i]
22This option merges all the files you want to print and prints
them.  The files are printed in columns across the page,
one file per column.  You can specify a maximum of 8 files.
If you chose the option that specifies the number of columns
across the page, you cannot use this -m option. [-m]
23If you want more than one column of output, this option
lets you specify the number of columns you want across
the page.  This option is incompatible with -m.
24This option specifies the number of characters per line.
You must also select -m and the option that specifies the
number of columns when you choose this option.  The default
number of characters is 72.  This option is incompatible
with -s. [-w]
25Columns are normally separated by tabs.  This option allows
you to separate columns with the single character you specify
here.  It also prevents truncation of lines on multi-column
output.  This option is incompatible with -w. [-s]
26This option prints multi-column output across the page.
Each column across the page represents one line.  If
the line is too long to fit in a column, it is truncated.
If you choose this option, you must also choose the option
that specifies the number of columns across the page.
You must have more than one column. [-a]
27ASSIST appends anything you type here to the command
line.  You can use this field for "piping,"
"redirection," or to execute a command in the
"background."  However, since ASSIST does not perform
any error checking on this field, you should use it
with caution.

0506
0605
2223
2322
2425
2524

1009
1109
2623

020443n
0305540
04063866
060867n
0709401
091243n
101367	
1114625
131746n
141863n
151968n
1722348
182338	
1924358
202539	
2329511
24304372
253146	
263263n

0015-
003420001010
0204yYnN
0308
0408
04240
0505
0605
0604yYnN
0708
07240
0904yYnN
0919
10011
10240123456789
1016
1108
11240
1116
1304yYnN
1404yYnN
1504yYnN
1708
18011
1908
20011
2205
2204yYnN
2305
2319
23240
2308
2405
2408
24240
2505
25011
2616
2604yYnN


001110
020001y-dY-dnN
031010-o0
041010-l66
051110-h 
060001y-tY-tnN
071010+1
091001y-nY-nnN
100000''	
1100105
130001y-rY-rnN
140001y-pY-pnN
150001y-fY-fnN
161110
171010-e8
181010-e''	
191010-i8
201010-i''	
221011y-mY-mnN
231010-1
241010-w72
251010-s''	
261011y-aY-anN
271110



