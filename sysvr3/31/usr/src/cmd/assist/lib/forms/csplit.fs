#ident	"@(#)forms.files:csplit.fs	1.6"
00000csplit
00702060101NAME OF INPUT FILE:
01704060130SPLIT FILE AT:
02706060001PREFIX FOR OUTPUT FILES [-f]:
03708060001PRINT COUNTS OF CHARACTERS [-s] (y/n):
04710060001DELETE OUTPUT FILES IF ERRORS [-k] (y/n):
05712060030ADDITIONS TO THE COMMAND LINE:

00040
01050
02030-f xx
03000s
04000k
05062

Since you did not enter the name of a file in the "NAME OF INPUT
FILE" field, the "csplit" command will expect input from
standard input.  Use ^D to indicate the end of input.00-

"csplit" splits a file into smaller files, based on
a set of splitting criteria specified by the user.
The first file created contains everything up to
(but not including) the first splitting criterion,
the second file contains everything from the first
criterion up to the second, and so forth.  In general,
the "csplit" command will try to create one more file
than the number of splitting criteria you specify.

00Enter here the name of the file to be split.  If you enter a hyphen,
the command will assume that input is coming from standard input.
If you do use standard input, type control-d (^D) to terminate input.
01You may split based on line numbers and/or patterns.  Examples:
CRITERIA:      ACTION:
 5 10          place lines 1-4 in one file, 5-9 in another, 
               lines 10 to end of input file in third file.
 "/the time/"  place all lines up to the first one containing "the time"
               in one file, and the rest of the input file in a second.
 %junk%        skip lines up to (but not including) the first with "junk"
               in it, place the rest of the input file in an output file.
 {3}           repeat the previous argument.  For example, 5 {10}
               would split the input file every four lines.

If you enter a string that contains white space, be sure to enclose
it in quotes.  You may enter any number of splitting criteria, and 
may inter-mix the different types.  See the manual page for details.
02By default, the newly created files will all begin with "xx,"
followed by 00 for the first file, 01 for the second, etc.
If you want to specify a prefix other than "xx" enter it here.
The prefix must be less than 13 characters.
03By default, the command prints the number of characters in
each file it creates.  Enter a "n" in this field if you want
to suppress this information.
04By default, all newly created files are deleted if an error occurs.
If you want to keep all newly created files regardless of errors, 
enter a "n" in this field.  In any case, the input file is not changed.
05ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



020636xx
030845y
041048y

0001
0015-
0013
003420001010
0132if [ ! "`echo "$F01" | /usr/bin/egrep \"^/.*/$|^%.*%$|^[0-9]+$|^{[0-9]+}$\"`" ]; then echo "LEGAL FORMATS: /string/ %string% integer {integer}"; elif [ "`echo "$F01" | /usr/bin/egrep \"^[0-9]+$\"`" -a "$F01" -le 1 -a "$F01" -ge 0 ]; then echo "LINE NUMBER MUST BE GREATER THAN 1";fi
020113
0304nNyY
0404nNyY


001110
011110
021010-f xx
031011n-sN-syY
041011n-kN-kyY
051110



