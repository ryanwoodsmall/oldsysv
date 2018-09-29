.\" @(#)subset.w	1.1
subset [-\fIoption\fR(s)] [\fIvector\fR(s)]; generate a subset
.sp
Output are elements selected from the input based on a \fIkey\fR
and \fIoption(s)\fR.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Selection is as follows:
.DS 1 1
If a \fImaster\fR vector is given,
then the \fIkey\fR for the \fIi\fRth element of the input
is the \fIi\fRth element of \fImaster\fR,
otherwise the \fIkey\fR is the input element itself.
In either case \fIi\fR goes from \fIstart\fR to \fIterminate\fR.
.sp
The input element is selected if the \fIkey\fR is
either above \fIabove\fR, below \fIbelow\fR, or equal to \fIpick\fR,
and not equal to \fIleave\fR.
If neither \fIabove, below\fR, nor \fIpick\fR is given,
then the element is selected if it is not equal to \fIleave\fR.
.sp
.in 0
Options:
.VL 15 5 1
.LI a\fIn\fR
\fIabove\fR := \fIn\fR.
.LI b\fIn\fR
\fIbelow\fR := \fIn\fR.
.LI c\fIn\fR
\fIn\fR elements per output line.
.LI F\fIvector\fR
\fIvector\fR is the \fImaster\fR.
.LI i\fIn\fR
\fIinterval := n\fR, default is 1.
.LI l\fIn\fR
\fIleave := n\fR.
.LI nl
leave elements whose index is given in \fImaster\fR.
.LI np
pick elements whose index is given in \fImaster\fR.
.LI p\fIn\fR
\fIpick := n\fR.
.LI s\fIn\fR
\fIstart := n\fR, default is 1.
.LI t\fIn\fR
\fIterminate := n\fR, default is 32767.
.LE 1
Examples:
.VL 23 5 1
.LI "subset -i2,s2 A"
outputs the even elements of A.
.LI "subset -FB,p1 A"
for each element in B with a 1,
output the corresponding element of A.
.LE
