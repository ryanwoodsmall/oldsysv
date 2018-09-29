.\" @(#)plot.w	1.2
plot [-\fIoption\fR(s)] [\fIvector\fR(s)]; plot an x-y graph
.sp
Output is a GPS that describes an x-y graph.
Input is one or more \fIvector(s)\fR.
Y-axis values come from \fIvector(s)\fR,
x-axis values from the F option.
Axis scales are determined from the first \fIvector(s)\fR plotted.
If no \fIvector\fR is given,
the standard input is assumed.
.sp
Options:
.VL 15 5 1
.LI a
Suppress axes.
.LI b
Plot graph with bold weight lines,
otherwise use medium.
.LI c\fIchar(s)\fR
Use \fIchar(s)\fR for plotting characters, implies option m.
The first character of \fIchar(s)\fR 
is used to mark the first graph, the second is used to
mark the second graph, etc.
.LI d
Do not connect plotted points, implies option m.
.LI F\fIvector\fR
Use \fIvector\fR for x-values,
otherwise the positive integers are used.
.LI g
Suppress background grid.
.LI m
Mark the plotted points.
.LI r\fIn\fR
Put the graph in GPS region \fIn\fR,
where \fIn\fR is between 1 and 25 inclusive.
.LI "x\fIn\fR (y\fIn\fR)"
Position the graph in the GPS universe with x-origin (y-origin) at \fIn\fR.
.LI "xa (ya)"
Omit x-axis (y-axis) labels.
.LI "xi\fIn\fR (yi\fIn\fR)"
\fIn\fR is the x-axis (y-axis) tick increment.
.LI "xl\fIn\fR (yl\fIn\fR)"
\fIn\fR is the x-axis (y-axis) low tick value.
.LI "xh\fIn\fR (yh\fIn\fR)"
\fIn\fR is the x-axis (y-axis) high tick value.
.LI "xn\fIn\fR (yn\fIn\fR)"
\fIn\fR is the approximate number of ticks on the
x-axis (y-axis).
.LI "xt (yt)"
Omit x-axis (y-axis) title.
.LE
.sp
Examples:
.VL 31 5 1
.LI "plot A"
plots \fIvector\fR A against the positive integers.
.LI "plot -r5,yl0,xa,FA B"
plots \fIvector\fR B against \fIvector\fR A,
y-axis ticks begin at 0,
no x-axis labels are printed,
the plot is placed in region 5 of the GPS universe.
.LI "plot -`hilo -oy A B` A B"
plots \fIvectors\fR A and B against the positive
integers, with y-axis ticks going from the lowest
value in A and B to the highest value in A and B.
.LI "plot -FA,FB C D E"
plots \fIvectors\fR C against A, D and E against B.
Y-axis scale is determined from C,
x-axis from A.
.LE
