.\" @(#)point.w	1.1
point [-\fIoption\fR(s)] [\fIvector\fR(s)]; empirical cumulative density function point
.sp
Output is a linearly interpolated value from the empirical cumulative
density function (e.c.d.f) for the input \fIvector\fR.
By default, \fIpoint\fR returns the median (50% point).
If no \fIvector\fR is given, the standard input is assumed.
.sp
Options:
.VL 10 5 1
.LI f\fIn\fR
Return the (1/\fIn\fR)*100 percent point from the e.c.d.f.
.LI "p\fIn\fR"
Return the \fIn\fR*100 percent point.
.LI "n\fIn\fR"
Return the \fIn\fRth element.
.LI "s"
The input is assumed to be sorted.
.LE
.sp
Example:   point -s,p.25 A
.br
outputs the
25% point from the e.c.d.f.
\fIVector\fR A is assumed to be sorted.
