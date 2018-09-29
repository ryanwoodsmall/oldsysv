.\" @(#)power.w	1.1
power [-\fIoption\fR(s)] [\fIvector\fR(s)]; power function
.sp
Output is a vector with each element being a power of the
corresponding element from the input \fIvector(s)\fR.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Options:
.VL 15 5 1
.LI "c\fIn\fR"
\fIn\fR is the number of output elements per line.
.LI "p\fIn\fR"
Input elements are raised to the \fIn\fRth power.
If not given, 2 is used.
.LE 1
Example:   power -p3.5,c3 A
.br
outputs the
3.5th power
of each element of A, three per line.
