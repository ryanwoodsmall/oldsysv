.\" @(#)hist.w	1.1
hist [-\fIoption\fR(s)] [\fIvector\fR(s)]; build a histogram
.sp
Output is a GPS that describes a histogram display.
Input is a \fIvector\fR of odd rank, with odd elements being bucket limits
and even elements being bucket counts
(see \fIbucket\fR).
If no \fIvector\fR is given, the standard input is assumed.
.sp
Options:
.VL 15 5 1
.LI a
Suppress axes.
.LI b
Plot histogram with bold weight lines,
otherwise use medium.
.LI f
Do not build a frame around plot area.
.LI g
Suppress background grid.
.LI r\fIn\fR
Put the histogram in GPS region \fIn\fR,
where \fIn\fR is between 1 and 25 inclusive.
.LI "x\fIn\fR (y\fIn\fR)"
Position the histogram in the GPS universe with x-origin (y-origin) at \fIn\fR.
.LI "xa (ya)"
Do not label x-axis (y-axis).
.LI "yl\fIn\fR"
\fIn\fR is the y-axis low tick value.
.LI "yh\fIn\fR"
\fIn\fR is the y-axis high tick value.
.LE
.sp
Example:   hist -r5,ya A
.br
outputs the
histogram described by \fIvector\fR A
and locates it in region 5 of the GPS
universe, with no y-axis labels.
