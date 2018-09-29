.\" @(#)hilo.w	1.1
hilo [-\fIoption\fR(s)] [\fIvector\fR(s)]; find high and low values
.sp
Output is the high and low values across all of the input
\fIvector(s)\fR. If no \fIvector\fR is given, the standard input is assumed.
.sp
Options:
.VL 10 5 1
.LI h
Only output high value.
.LI l
Only output low value.
.LI o
Output high, low values in "option" form (see \fIplot\fR).
.LI ox
Output high, low values with x prepended.
.LI oy
Output high, low values with y prepended.
.LE 1
Example:   hilo -ox,l A B
.br
outputs the
lowest value in \fIvectors\fR A and B with
\fIxl\fR prepended, i.e. \fIxl\fRlowvalue.
