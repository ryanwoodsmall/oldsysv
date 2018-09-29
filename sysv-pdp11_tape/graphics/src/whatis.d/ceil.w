.\" @(#)ceil.w	1.1
ceil [-\fIoption\fR] [\fIvector\fR(s)]; ceiling function
.sp
Output is a vector with each element being the smallest integer
greater than the corresponding element from the input \fIvector(s)\fR.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Options:   c\fIn  n\fR is the number of output elements per line.
.sp
Example:   ceil -c3 A
.br
outputs the
ceiling
of each element of A, three per line.
