.\" @(#)hpd.w	1.1
hpd [-\fIoption\fR(s)] [GPS \fIfiles\fR(s)];  display GPS on a a Hewlett Packard 7221A Graphics Plotter
.sp
Output is scope code for a Hewlett Packard 7221A Plotter.
A viewing window is computed from the maximum and minimum points in
the GPS \fIfile\fR(s) unless the \fIr\fR or \fIu\fR option is provided.
If no \fIfile\fR is given, the standard input is assumed.
.sp
Options:
.VL 15 5 1
.LI c\fIn\fR
Select character set \fIn\fR, \fIn\fR between 0 and 5 (see the HP 7221A Plotter
Operating and Programming Manual, Apendix A).
.LI p\fIn\fR
Select pen numbered \fIn\fR, \fIn\fR between 1 and 4 inclusive.
.LI r\fIn\fR
Window on GPS region \fIn\fR,
\fIn\fR between 1 and 25 inclusive.
.LI s\fIn\fR
Slant characters \fIn\fR degrees counterclockwise from the vertical.
.LI u
Window on the entire GPS universe.
.LI xd\fIn\fR
Set x displacement of the viewport's lower left corner to \fIn\fR inches.
.LI xv\fIn\fR
Set width of viewport to \fIn\fR inches.
.LI yd\fIn\fR
Set y displacement of the viewport's lower left corner to \fIn\fR inches.
.LI yv\fIn\fR
Set height of viewport to \fIn\fR inches.
.LE
.sp
Example:   hpd -p3,s-20,xv12.5,yv10 A.g B.g
.br
displays A.g and B.g together using pen number three and slanting characters forward by 20 degrees.
The viewing window is built to include all of both files, and the device viewport is 12.5 by 10 inches.
