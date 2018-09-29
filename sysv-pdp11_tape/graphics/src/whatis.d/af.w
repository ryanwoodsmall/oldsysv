.\" @(#)af.w	1.1
af [-\fIoption\fR(s)] \fIexpression\fR(s); arithmetic function
.sp
Output is the result of executing the \fIexpression\fR(s) once per complete set of input values.
Input comes from vectors specified in \fIexpression\fR.
.sp
Expression operands are:
.VL 21 5 1
.LI Vectors:   
filenames with the restriction that they
must begin with a letter and be composed only from letters,
digits, '_', and '.'.
The first unknown filename (one not in the current directory) references
the standard input.
.LI Functions:
the name of a command followed by the command arguments in parentheses.
Arguments are written as on the command line.
.LI Constants:    
floating point and integer (but not E notation).
.LE 1
Expression operators are, in order of decreasing precedence:
.VL 21 5 1
.LI \'\fIv\fR    
next value from vector \fIv\fR.
.LI "\fIx^y, -x\fR
\fIx\fR raised to the \fIy\fR power,
negation of \fIx\fR;
both associate right to left.
.LI "\fIx*y, x/y, x%y"
\fIx\fR multiplied by, divided by, modulo \fIy\fR, respectively;
all associate left to right.
.LI "\fIx+y, x-y"
\fIx\fR plus, minus \fIy\fR, respectively;
both associate left to right.
.LI "\fIx,y\fR"
output the value of \fIx\fR followed by the value of \fIy\fR;
associates from left to right
.LE 1
Parenthesis may be used to alter precedence.
Because many of the operator characters are special
to the shell it is good practice to surround \fIexpressions\fR
in quotes.
.sp
Options:
.VL 10 5 1
.LI "c\fIn\fR"
\fIn\fR elements per line in the output.
.LI t
output is titled from the vector on the standard input.
.LI v
verbose mode, function expansions are echoed.
.LE 1
Examples:
.VL 25 5
.tr Q"
.LI "af Q3+4*5Q"
yields 23
.LI "af QA,'A,A+'A,BQ"
yields a four column matrix with columns of
.br
  1) odd elements from A,
.br
  2) even elements from A,
.br
  3) sum of adjacent odd and even elements, and
.br
  4) elements from B.
.LI "af Qsin(A)^2Q"
yields the square of the sin of the elements of A.
.LE
