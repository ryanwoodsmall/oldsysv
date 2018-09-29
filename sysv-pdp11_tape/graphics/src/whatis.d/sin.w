.\" @(#)sin.w	1.1
sin [-\fIoption\fR] [\fIvector\fR(s)]; sin
.sp
Output is the sin for each element of the input \fIvector(s)\fR.
Input is assumed to be in radians.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Option:   c\fIn  n\fR is the number of output elements per line.
.sp
Example:   sin -c3 A
.br
outputs the
sin
of each element of A, three per line.
