.\" @(#)qsort.w	1.1
qsort [-\fIoption\fR] [\fIvector\fR(s)]; quick sort
.sp
Output is a vector of the elements from the input \fIvector\fR in ascending order.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Option:   c\fIn  n\fR is the number of output elements per line.
.sp
Example:   qsort -c3 A
.br
outputs the
the elements of A in ascending order.
