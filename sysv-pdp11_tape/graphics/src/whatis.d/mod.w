.\" @(#)mod.w	1.1
mod [-\fIoption\fR(s)] [\fIvector\fR(s)]; modulo function
.sp
Output is a vector with each element being the remainder of
dividing the corresponding element from the input \fIvector(s)\fR
by the \fImodulus\fR.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Options:
.VL 15 5 1
.LI "c\fIn\fR"
\fIn\fR is the number of output elements per line.
.LI "m\fIn\fR"
\fIn\fR is the \fImodulus\fR.  If not given, 2 is used.
.LE 1
Example:   mod -m8,c3 A
.br
outputs the
elements of A modulo 8, three per line.
