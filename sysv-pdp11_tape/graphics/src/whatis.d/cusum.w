.\" @(#)cusum.w	1.1
cusum [-\fIoption\fR] [\fIvector\fR(s)]; cumulative sum
.sp
Output is a vector with the \fIi\fRth element being the sum of
the first \fIi\fR elemtents from the input \fIvector\fR.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Option:   c\fIn  n\fR is the number of output elements per line.
.sp
Example:   cusum -c3 A
.br
outputs the
cumulative sum
of the elements of A, three per line.
