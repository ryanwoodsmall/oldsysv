.\" @(#)log.w	1.1
log [-\fIoption\fR(s)] [\fIvector\fR(s)]; logarithm
.sp
Output is the logarithm for each element of the input \fIvector(s)\fR.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Options:
.VL 10 5 1
.LI "b\fIn\fR"
\fIn\fR is the logarithm base.  If not given, 2.71828... is used.
.LI "c\fIn\fR"
\fIn\fR is the number of output elements per line.
.LE 1
Example:   log -b2,c3 A
.br
outputs the
logarithm base 2
of each element of A, three per line.
