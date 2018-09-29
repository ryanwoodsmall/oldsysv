.\" @(#)pair.w	1.1
pair [-\fIoption\fR(s)] [\fIvector\fR(s)]; pair element groups
.sp
Output is a vector with elements taken alternately from
a \fIbase\fR vector and from a \fIvector\fR.
The \fIbase\fR vector is specified either with the F option,
or else it comes from the standard input.
\fIVector(s)\fR are specified either on the command line
or else one may come from the standard input.
If both the \fIbase\fR and \fIvector\fR come from the standard input,
\fIbase\fR precedes \fIvector\fR.
.sp
Options:
.VL 15 5 1
.LI c\fIn\fR
\fIn\fR is the number of output elements per line.
.LI F\fIvector\fR
\fIvector\fR is the \fIbase\fR.
.LI x\fIn\fR
\fIn\fR is the number of elements taken from the \fIbase\fR
for each one element taken from \fIvector\fR.
.LE
.sp
Example:   pair -x3,FA B
.br
Outputs a vector with three elements from A,
then one from B, then three from A, one from B,
and so on.
