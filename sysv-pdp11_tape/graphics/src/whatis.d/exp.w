.\" @(#)exp.w	1.1
exp [-\fIoption\fR] [\fIvector\fR(s)]; exponential function
.sp
Output is a vector with elements e raised to the \fIx\fR power,
where e is 2.71828, approximately,
and \fIx\fR are the elements from the input \fIvector(s)\fR.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Option:   c\fIn  n\fR is the number of output elements per line.
.sp
Example:   exp -c3 A
.br
outputs the
exponential
of each element of A, three per line.
