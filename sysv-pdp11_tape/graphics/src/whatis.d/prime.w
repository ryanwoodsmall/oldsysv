.\" @(#)prime.w	1.1
prime [-\fIoption\fR(s)]; generate prime numbers
.sp
Output is a vector of \fInumber\fR elements determined by the parameters \fIlow\fR and \fIhigh\fR.
The parameters are set by command options.
.sp
Options:
.VL 10 5 1
.LI c\fIn\fR
\fIn\fR elements per output line.
.LI h\fIn\fR  
\fIhigh\fR:=\fIn\fR.
.LI l\fIn\fR
\fIlow\fR:=\fIn\fR.  If not given, \fIlow\fR:=2.
.LI n\fIn\fR  
\fInumber\fR:=\fIn\fR.  If not given, \fInumber\fR:=10.
.LE 1
Example:	prime -l200,h300
.br
generates all prime numbers between 200 and 300.
