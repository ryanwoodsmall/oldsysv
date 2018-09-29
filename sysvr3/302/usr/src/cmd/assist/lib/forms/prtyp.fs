00000pr
00701070030NAME OF INPUT FILE(S):
01703070001DOUBLE-SPACE THE OUTPUT [-d] (y/n):
02705070001NUMBER OF LINES PER PAGE [-l]:
03707070001BEGIN PRINTING WITH PAGE NUMBER [+]:
04709070001PRINT MULTIPLE INPUT FILES, ONE PER COLUMN [-m] (y/n):
05711070001NUMBER OF COLUMNS TO PRINT INPUT FILES [-]:
06713070001NUMBER OF CHARACTERS PER LINE [-w]:
07715070030ADDITIONS TO THE COMMAND LINE:

00060
01030
02050
03040
04010
05010
06020
07072



"pr" prints files on your terminal.  By default,
the file is printed separated into pages.  Each
page is headed by a page number, the date and
time, and the name of the file.  This is the
"typical" command form for "pr."  It contains
the frequently used options.  There is also
a complete command form for "pr" which
contains all of the options available.

00List here the name of the file(s) you want to print.  If you
specify "-" or don't list any file name(s), "pr" assumes the
standard input.
01This option double-spaces the output.  Blank lines that result
from double-spacing are dropped when they occur at the top of
a page. [-d]
02This option sets the page length.  If you specify a page length
of less than 10 lines, the header and trailer are suppressed.
Normally, "pr" uses a header of 5 lines and a trailer of 5 lines.
If you specify a page length of greater than 10 lines, "pr"
subtracts 10 lines to allow for header and trailer.  This leaves
you (length-10) for your text.  The default page length is 66
lines. [-l]
03Type the page number at which you want to start
printing.  "pr" will start printing at the
page number you specify and print that page
number on the top of the page.  For example,
if you specify page 6, "pr" will start at
page 6 and the page numbers will be printed
starting with page 6.  The default is page one. [+]
04This option merges all the files you want to print and prints
them.  The files are printed in columns across the page,
one file per column.  You can specify a maximum of 8 files.
If you chose the option that specifies the number of columns
across the page, you cannot use this -m option. [-m]
05If you want more than one column of output, this option
lets you specify the number of columns you want across
the page.  This option is incompatible with -m. [-]
06This option specifies the number of characters per line.
You must also select -m and the option that specifies the
number of columns when you choose this option.  The default
number of characters is 72.  This option is incompatible
with -s. [-w]
07ASSIST appends anything you type here to the
command line.  You can use this field for
"piping," "redirection," or to execute
a command in the "background."  However,
since ASSIST does not perform any error checking
on this field, you should use it with caution.

0405
0504


010343n
02053866
0307441
040962n
0511511
06134372

0015-
0104yYnN
0208
0308
0405
0404yYnN
0505
0508
0631
0608


001110
011011y-dY-dnN
021010-l66
031010+1
041011y-mY-mnN
051010-1
061010-w72
071110

060405


