400EDIT
00001160000EDIT, CUT, AND PASTE FILES
01703110101............. Concatenate and show files
02704140101.......... Split a file into sections according to an expression
03705110101............. Cut out selected fields of each line of a file
04706100101.............. Edit files with a line oriented text editor
05707130101........... Paste files together by columns
06708110101............. Stream editor
07709130101........... Split a file into pieces by number of lines
08710100101.............. Edit files with a screen oriented text editor
09711210101... Brief introduction to the "vi" screen editor

The Edit, Cut and Paste Files menu lists commands
that edit the contents of files, divide files, join
files, and move chunks of text within and between
files.

01"cat" prints the content of files on your terminal.
If you specify multiple files, "cat" prints each
in turn, thus concatenating the multiple files.
02"csplit" splits a file into smaller files, based on
a set of splitting criteria specified by the user.
The first file created contains everything up to
(but not including) the first splitting criterion,
the second file contains everything from the first
criterion up to the second, and so forth.  In general,
the "csplit" command will try to create one more file
than the number of splitting criteria you specify.
03"cut" prints selected columns or fields from a file.
If the material you want appears in the same columns
on every line of the file, cut based on columns.  If
the material you want is separated by some character
(for example, <TAB>), cut by field.
04"ed" is a line-oriented text editor that allows you
to create and edit files.  Editing with "ed" is
line-by-line rather than full-screen (as in "vi").
You can use "ed" on both hard-copy and video display
terminals.
05"paste" concatenates corresponding lines of input
files.  It treats each file as a column or columns
of a table and pastes them together horizontally
or vertically.
06"sed" reads lines from a file, changes the lines
according to a script you supply, and puts out the
modified lines on standard output.  The actual
files are not changed.
07"split" divides a file into a number of smaller files.
The first "n" lines of the input file are placed in
the first output file, the second "n" lines are placed
in the second, etc.  By default, "n" is 1000 lines,
but you may specify a different value.  The input file
is not changed.
08"vi" is a full-screen text editor.  When you use
"vi", the changes you make are reflected on the
screen as you make them.  You can use "vi" to create
and edit any type of text file.  Examples of text
files include:  (1) mail messages, (2) documents
that will be formatted by a command such as "mm",
(3) programs in languages such as C, and (4) shell
scripts.  
09ASSIST's "vi" walkthru is designed to get you started
with a basic set of "vi" editing commands.

"vi" is a full-screen text editor.  When you use
"vi", the changes you make are reflected on the
screen as you make them.  You can use "vi" to create
and edit any type of text file.  Examples of text
files include:  (1) mail messages, (2) documents
that will be formatted by a command such as "mm",
(3) programs in languages such as C, and (4) shell
scripts.  

010307cat
020407csplit
030507cut
040607ed
050707paste
060807sed
070907split
081007vi
091107assist viwalk

01cat
02csplit
03cut
04ed
05paste
06sed
07split
08vi
09viwalk

