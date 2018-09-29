#ident	"@(#)forms.files:cut.fs	1.5"
00000cut
00702060030FILE NAME(S):
01904020000CUT BY COLUMN
02705060001LIST OF CHARACTER POSITION(S) TO SELECT [-c]:
03907020000CUT BY FIELD
04708060001LIST OF FIELD(S) TO SELECT [-f]:
05709060030FIELD DELIMITER CHARACTER [-d]:
06710060001SELECT ONLY LINES WITH DELIMITER CHARACTERS [-s] (y/n):
07712060030ADDITIONS TO THE COMMAND LINE:

00040-
02010-c
04010-f
05022d
06030-s
07082

Since you did not enter the name of a file in the "FILE NAME(S)" field,
the "cut" command is going to expect input to come from you terminal.
Use ^D to indicate to "cut" that you have completed your input.00

"cut" prints selected columns or fields from a file.
If the material you want appears in the same columns
on every line of the file, cut based on columns.  If
the material you want is separated by some character
(for example, <TAB>), cut by field.

00Enter the name(s) of the file(s) for which you want to select
columns or fields.
02If you want to select certain character positions from your file,
you should use this option.  Enter here the character positions
in which you are interested.  Indicate ranges with a hyphen, and
use commas to separate lists.  For example, 1-3,5,7-9 would select
those characters in columns 1,2, 3, 5, 7, 8, and 9.  Column 1
refers to the first character on the line (including spaces), column
2 is the second, etc.   [-c]
04Use this option if you want to select certain fields.  Each field must
be separated by a "separator character" (default is <TAB>, see the
"FIELD DELIMITER CHARACTER" field for more information).  For example,
to select all of the text before the first separator, and the text
between the fourth and fifth delimiters, enter 1,4-5.  [-f]
05You may use this option only if you select text by fields.  Field 1
is the text from the beginning of the line up to the first delimiter.
The last field is the text following the last delimiter.
All other fields are defined as the text between field delimiters.
By default, this delimiter is a tab.  You may change it to any single
character. If you want this set to a space, type ^C and then hit
the space bar. [-d]
06By default, any input lines that have no delimiter characters
are passed through the command without change.  If you want
to suppress printing of such lines, enter a "y" here.  You may
use this option only when sorting by field, not column.  [-s]
07ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0204
0402
0502
0602

0504
0604

050938	
061062n

000114
0002
0013
003420001010
0205
0231
0232if [ "`echo $F02|grep \"[^0123456789,\-]\"`" ];then echo "ONLY DIGITS, HYPHEN, AND COMMA ALLOWED IN LIST";fi
0405
0431
0432if [ "`echo $F04|grep \"[^0123456789,\-]\"`" ];then echo "ONLY DIGITS, HYPHEN, AND COMMA ALLOWED IN LIST";fi
0505
0516
05011
0605
0616
0604yYnN


001110
021010-c
041010-f
051110-d''	
061011y-sY-snN
071110

0204
0402


