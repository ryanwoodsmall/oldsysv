.\" @(#)floor.w	1.1
floor [-\fIoption\fR] [\fIvector\fR(s)]; floor function
.sp
Output is a vector with each element being the largest integer
less than the corresponding element from the input \fIvector(s)\fR.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Option:   c\fIn  n\fR is the number of output elements per line.
.sp
Example:   floor -c3 A
.br
outputs the
floor
of each element of A, three per line.
