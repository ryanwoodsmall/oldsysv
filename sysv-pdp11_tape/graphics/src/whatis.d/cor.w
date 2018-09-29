.\" @(#)cor.w	1.1
cor [-\fIoption\fR] [\fIvector\fR(s)]; ordinary correlation coefficient
.sp
Output is the ordinary correlation coefficient between a \fIbase\fR vector and another
\fIvector\fR.
The \fIbase\fR vector is specified using the F option.
If the \fIbase\fR or \fIvector\fR is not given, it is assumed to come from the standard input.
Each \fIvector\fR is compared to the \fIbase\fR.
Both \fIbase\fR and \fIvector\fR must be of the same length.
.sp
Option:   F\fIvector  vector\fR is the \fIbase\fR.
.sp
Example:   cor -FA B C
.br
outputs the ordinary correlation coefficients between \fIvectors\fR A and B,
and \fIvectors\fR A and C.
