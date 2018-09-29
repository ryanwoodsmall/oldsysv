.\" @(#)root.w	1.1
root [-\fIoption\fR(s)] [\fIvector\fR(s)]; root function
.sp
Output is a vector with each element being the \fIroot\fR root of the
corresponding element from the input \fIvector(s)\fR.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Options:
.VL 15 5 1
.LI "c\fIn\fR"
\fIn\fR is the number of output elements per line.
.LI "r\fIn\fR"
\fIroot\fR:=\fIn\fR.
If not given, \fIroot\fR:=2.
.LE 1
Example:   root -p3.5,c3 A
.br
outputs the
3.5th root
of each element of A, three per line.
