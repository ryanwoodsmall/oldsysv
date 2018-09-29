.\" @(#)abs.w	1.1
abs [-\fIoption\fR] [\fIvector\fR(s)]; absolute value
.sp
Output is the absolute value for each element of the input \fIvector(s)\fR.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Option:   c\fIn  n\fR is the number of output elements per line.
.sp
Example:   abs -c3 A
.br
outputs the
absolute value
of each element of A, three per line.
