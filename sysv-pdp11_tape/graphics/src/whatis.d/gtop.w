.\" @(#)gtop.w	1.1
gtop [-\fIoption\fR(s)] [GPS \fIfile\fR(s)]; GPS to \fIplot\fR(5) filter
.sp
\fIGtop\fR transforms a GPS into \fIplot(5)\fR commands displayable by \fIplot(1)\fR filters.
Input is taken from \fIfile\fR if given, else from the standard input.
GPS objects are translated if they fall within the window
that circumscribes the first \fIfile\fR unless an
\fIoption\fR is given.
Output is to the standard output.
.sp
Options:
.VL 10 5 1
.LI r\fIn\fR
Translate objects in GPS region \fIn\fR.
.LI u
Translate all objects in the GPS universe.
.LE 1
Example:  gtop A.g
.br
translates the GPS in file A.g into
\fIplot(5)\fR commands.
