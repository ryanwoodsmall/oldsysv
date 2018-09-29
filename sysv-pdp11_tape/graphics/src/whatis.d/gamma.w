.\" @(#)gamma.w	1.1
gamma [-\fIoption\fR] [\fIvector\fR(s)]; gamma function
.sp
Output is the gamma value for each element of the input \fIvector(s)\fR.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Option:   c\fIn  n\fR is the number of output elements per line.
.sp
Example:   gamma -c3 A
.br
outputs the
gamma value
for each element of A, three per line.
