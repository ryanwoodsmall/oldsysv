00000lex
00702100030NAMES OF INPUT FILES:
01704100001PRINT SUMMARY STATISTICS [-v] (y/n):
02706100001SUPPRESS SUMMARY STATISTICS [-n] (y/n):
03708100001WRITE OUTPUT TO STANDARD OUTPUT [-t] (y/n):
04710100001WRITE ACTIONS IN RATFOR [-r] (y/n):
05712100030ADDITIONS TO THE COMMAND LINE:

00050
01011
02011
03011
04011
05062

By not entering a file name, you have requested that the "lex" command
take input from your terminal.  Type ^D to end input.00

"lex" generates programs for simple lexical analysis 
of text.  "lex" produces a C code subroutine as output
that you compile and combine with other programs.  You 
can use the processing done by "lex" as the first step 
in creating a compiler-type program.  In addition, you
can use output from "lex" as a preprocessing tool for
many different software generation functions.  

00Enter the names of the files that contain the
description of the program to be generated by
"lex."  If you do not supply a filename "lex"
will read from standard input.
01If you enter a "y" in this field "lex" will
produce a summary of statistics of the machine
generated. 
02If you enter a "y" for this field, "lex" will
not print summary statistics of the machine
generated.
03If you enter a "y" for this field, "lex" will
write the output to standard out instead of
using the default for output.  By default
output is written to the file "lex.yy.c" in your
current directory.
04Typically actions to "lex" are written in 
C, if you enter a "y" for this field, the
actions are written in RATFOR.
NOTE: This option is not fully operational
and you may experience some problems.
05ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since 
ASSIST does not perform any error checking on this field,
you should use it with caution.

0102
0201


010447n
020650n
030854n
041046n

0015-
003420001010
0104yYnN
0105
0204yYnN
0205
0304yYnN
0404yYnN


001110
010001y-vY-vnN
020001y-nY-nnN
030001y-tY-tnN
040001y-rY-rnN
051110



