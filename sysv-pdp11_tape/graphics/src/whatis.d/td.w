.\" @(#)td.w	1.1
td [-\fIoption\fR(s)] [GPS \fIfiles\fR(s)];  display GPS on a Tektronix 4014
.sp
Output is scope code for a Tektronix 4014 terminal.
A viewing window is computed from the maximum and minimum points in
the first \fIfile\fR unless \fIoptions\fR are provided.
If no \fIfile\fR is given, the standard input is assumed.
.sp
Options:
.VL 15 5 1
.LI r\fIn\fR
Window on GPS region \fIn\fR,
\fIn\fR between 1 and 25 inclusive.
.LI u
Window on the entire GPS universe.
.LI e
Do not erase screen before initiating display.
.LE
.sp
Example:   td A.g B.g
.br
displays A.g and B.g.
The viewing window is built to include all of A.g.
