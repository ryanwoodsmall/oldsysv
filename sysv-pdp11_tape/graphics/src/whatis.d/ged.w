.\" @(#)ged.w	1.1
ged [-\fIoption\fR(s)] [GPS \fIfiles\fR(s)];  graphical editor
.sp
\fIGed\fR allows displaying and editing of GPS.
.sp
Options:
.VL 15 5 1
.LI R
Invoke the editor in a restricted \fIshell\fR environment.
.LI e
Do not erase screen before initial display.
.LI r\fIn\fR
Window on GPS region \fIn\fR,
\fIn\fR between 1 and 25 inclusive.
.LI u
Window on the entire GPS universe.
.LE
.sp
Example:   ged A.g B.g
.br
displays A.g and B.g and prepares them for editing.
