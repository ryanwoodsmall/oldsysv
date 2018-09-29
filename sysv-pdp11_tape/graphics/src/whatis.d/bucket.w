.\" @(#)bucket.w	1.1
bucket [-\fIoption\fR(s)] [sorted \fIvector\fR(s)]; generate buckets and counts
.sp
Output is a vector with odd values being bucket limits (in parentheses)
and even values being the number of elements from the input within the limits.
Input is assumed to be sorted.
If no input \fIvector(s)\fR are given, the standard input is assumed.
Unless otherwise specified, bucket limits are generated based on the
input data and the rule:  num buckets = 1 + log2(num elements).
.sp
Options:
.VL 15 5 1
.LI a\fIn\fR     
choose limits such that \fIn\fR is the average count per bucket.
.LI c\fIn\fR
\fIn\fR elements per line in the output.
.LI F\fIvector\fR    
take limit values from \fIvector\fR.
.LI h\fIn\fR
\fIn\fR is the highest limit.
.LI i\fIn\fR
\fIn\fR is the interval between limits.
.LI l\fIn\fR
\fIl\fR is the lowest limit.
.LI n\fIn\fR
\fIn\fR is the number of buckets.
.LE
.sp
Example:   bucket -a12,l-5 A
.br
outputs limits and counts for the elements of A, where
the lowest limit is -5 and the average bucket count is 12.
