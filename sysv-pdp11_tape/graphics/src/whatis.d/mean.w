.\" @(#)mean.w	1.1
mean [-\fIoption\fR(s)] [\fIvector\fR(s)]; mean
.sp
Output is the mean of the elements in the input \fIvector(s)\fR.
The input may optionally be trimmed.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Options:
.VL 15 5 1
.LI f\fIn\fR
Trim (1/\fIn\fR)*r elements from each end, where r is the rank of the input vector.
.LI "p\fIn\fR"
Trim \fIn\fR*r elements from each end, \fIn\fR is between 0 and .5.
.LI "n\fIn\fR"
Trim \fIn\fR elements from each end.
.LE
.sp
Example:   mean -p.25 A
.br
outputs the
mean of the middle 50% of the elements of A,
i.e., A is trimmed by 25% of its elements from both ends.
