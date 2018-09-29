.\" @(#)lreg.w	1.1
lreg [-\fIoption\fR(s)] [\fIvector\fR(s)]; linear regression
.sp
Output is the slope and intercept from a least squares linear regression
of each \fIvector\fR on a \fIbase\fR vector.
The \fIbase\fR vector is specified using the F option.
If the \fIbase\fR is not given,
it is assumed to be ascending positive integers from zero.
.sp
Options:
.VL 13 5 1
.LI "F\fIvector\fR"
\fIvector\fR is the \fIbase\fR.
.LI i
Only output the intercept.
.LI o
Output the slope and intercept in "option" form (see \fIsiline\fR).
.LI s
Only output the slope.
.LE 1
Example:   lreg -FA,i B
.br
outputs the
intercept from the linear regression of
\fIvector\fR B on \fIbase\fR vector A.
