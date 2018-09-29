.\" @(#)round.w	1.1
round [-\fIoption\fR(s)] [\fIvector\fR(s)]; rounded value
.sp
Output is the rounded value for each element of the input \fIvector(s)\fR.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Options:
.VL 10 5 1
.LI "c\fIn\fR"
\fIn\fR is the number of output elements per line.
.LI "p\fIn\fR"
\fIn\fR is the number of places following the decimal point rounded to.
\fIn\fR is in the range 0 to 9, 0 by default.
.LI "s\fIn\fR"
\fIn\fR is the number of significant digits rounded to.
\fIn\fR is in the range 0 to 9, 9 by default.
.LE
.sp
Example:   round -s2,c3 A
.br
outputs the
value of each element of A rounded to two significant digits,
three per line.
