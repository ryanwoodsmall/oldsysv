.\" @(#)siline.w	1.1
siline [-\fIoption\fR(s)] [\fIvector\fR(s)]; generate a line given slope and intercept
.sp
Output is a vector of values \fIslope*x+intercept\fR,
where \fIx\fR takes on values from \fIvector(s)\fR.
If the \fIn\fR option is given,
\fIvector\fR is the ascending positive integers.
If neither the \fIn\fR option nor a \fIvector\fR is given,
\fIvector\fR comes from the standard input.
.sp
Options:
.VL 10 5 1
.LI "c\fIn\fR"
\fIn\fR is the number of output elements per line.
.LI "i\fIn\fR"
\fIn\fR is the \fIintercept\fR, 0 if not given.
.LI "n\fIn\fR"
\fIn\fR is the number of positive integers to be used for \fIx\fR.
.LI "s\fIn\fR"
\fIn\fR is the \fIslope\fR, 1 if not given.
.LE 1
Example:   siline -`lreg -o,FA B` A
.br
outputs a simple linear fit of vector B on vector A.
(The o option of \fIlreg\fR outputs the
slope and intercept in option form of B regressed on A.)
