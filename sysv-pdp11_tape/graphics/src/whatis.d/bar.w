.\" @(#)bar.w	1.1
bar [-\fIoption\fR(s)] [\fIvector\fR(s)]; build a bar chart
.sp
Output is a GPS that describes a bar chart display.
Input is a \fIvector\fR of counts that describes the y-axis. 
By default, x-axis will be labelled with integers beginning at 1;
for other labels, see \fIlabel\fR.
If no \fIvector\fR is given, the standard input is assumed.
.sp
Options:
.VL 15 5 1
.LI a
Suppress axes.
.LI b
Plot bar chart with bold weight lines,
otherwise use medium.
.LI f
Do not build a frame around plot area.
.LI g
Suppress background grid.
.LI r\fIn\fR
Put the bar chart in GPS region \fIn\fR,
where \fIn\fR is between 1 and 25 inclusive.
.LI w\fIn\fR
\fIn\fR is the ratio of the bar width to center-to-center spacing expressed as a percentage.
Default is 50, giving equal bar width and bar space.
.LI "x\fIn\fR (y\fIn\fR)"
Position the bar chart in the GPS universe with x-origin (y-origin) at \fIn\fR.
.LI "xa (ya)"
Do not label x-axis (y-axis).
.LI "yl\fIn\fR"
\fIn\fR is the y-axis low tick value.
.LI "yh\fIn\fR"
\fIn\fR is the y-axis high tick value.
.LE
.sp
Example:   bar -r10,xa,w75 A
.br
outputs the
bar chart described by vector A, located in region 10 of the GPS
universe, with no x-axis labels.  Bar width is 75% of the center-to-center spacing.
