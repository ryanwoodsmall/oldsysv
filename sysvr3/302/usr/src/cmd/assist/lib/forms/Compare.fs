400COMPARE FILES
00001200000COMPARE FILES
01703100101..... Show byte and line numbers of differences
02704110101.... Select/reject common lines
03705110101.... Show all lines that differ
04007130000MORE COMMANDS FOR COMPARING FILES
05709120101... Show differences between files too large for "diff"
06710120101... Compare three files
07711130101.. Create new file with "nroff" commands to mark differences
08712120101... Show files side-by-side, marking differences

The Compare Files menu lists commands that compare
files and show the differences between files.

01"cmp" is used to compare two files.  By default,
"cmp" produces no output if the files are identical.
If they differ, it lists the byte and line number of
the first difference.
02"comm" prints lines that are either unique or common
to two files.  The files must be sorted in ASCII
collating order.  Use the "sort" command to do this.
By default, three columns of output are produced.
The left column contains those lines in the first
file that are not in the second.  The middle column
contains those lines that are in the second file,
but not the first.  The right column contains those
lines common to both files.
03"diff" tells you what lines you must change to
make two files identical.  "diff" can also create
scripts that you can use with "ed" to change the
files.
05"bdiff" shows the changes that you must make to
one file to make it identical to a second file.
It produces the same output as the "diff" command;
the difference between the two commands is that
"bdiff" will work on larger files than will "diff."
Also, "diff" provides some options that are not
available with "bdiff".
06"diff3" is similar to the "diff" command, but it
compares three files and lists the differences
between them rather than comparing two files.
"diff3" prints disagreeing text segments, indica-
ting what is different between the files.  "diff3"
can also create scripts that you can use with "ed"
to change the files.
07"diffmk" compares two versions of a file and creates
a third file that includes "change mark" macros.
These macros identify lines that have been changed
between the two versions of the files.  When you
format the file with "change marks" using "nroff"
or "troff", the changed lines will be marked with
dashes (-) or asterisks (**).
08"sdiff" produces a side-by-side listing of two files,
indicating those lines that are different.  Each
line of the two files is printed with a blank between
them if the lines  are identical, a < between them if
the line only exists in the first file named, a >
between them if the line only exists in the second
file named, and a | if the lines are different.

010306cmp
020406comm
030506diff
050906bdiff
061006diff3
071106diffmk
081206sdiff

01cmp
02comm
03diff
05
06
07
08

