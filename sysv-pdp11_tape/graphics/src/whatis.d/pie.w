.\" @(#)pie.w	1.1
pie [-\fIoption\fR(s)] [\fIfile\fR(s)]; build a pie chart
.sp
Output is a GPS that describes a pie chart.
Input is lines of the form:
.sp
		[<control>] value [label]
.sp
The control field specifies the way that slice should be handled.
Legal values are:
.VL 14 5 1
.LI "i"
The slice will not be drawn, though a space will be left for it.
.LI "e"
The slice is "exploded", or moved away from the pie.
.LI "f"
The slice is filled.
The angle of fill lines depends on the color of the slice.
.LI "c\fIcolor\fR"
The slice is drawn in \fIcolor\fR rather than the default black.
Legal values for \fIcolor\fR are 'b' for black, 'r' for red, 'g' for green, and 'u' for blue.
.LE
.sp
The pie is drawn with the \fIvalue\fR of each slice printed inside and the \fIlabel\fR
printed outside.
If no \fIfile\fR is specified, the standard input is assumed.
.br
.sp
Options:
.VL 14 5 1
.LI "b"
Draw pie chart in bold weight lines, otherwise use medium.
.LI "p"
Output \fIvalue\fR as a percentage of the total pie.
.LI "pp\fIn\fR"
Only draw \fIn\fR percent of a pie.
.LI "pn\fIn\fR"
Output \fIvalue\fR as a percentage, but total of percentages equals \fIn\fR rather
than 100.  pn100 is equivalent to p.
.LI "v"
Do not output values.
.LI "o"
Output values around the outside of the pie.
.LI "r\fIn\fR"
Put the pie chart in region \fIn\fR, where \fIn\fR is between 1 and 25 inclusive.
.LI "x\fIn\fR (y\fIn\fR)"
Position the pie chart in the GPS universe with x-origin (y-origin) at \fIn\fR.
.LE
.sp
Example:
.ti
pie -pp80,pn80 A
.br
will draw the pie chart specified by file A in 80% of a circle 
and will output the \fIvalues\fR as percentages that total 80.
