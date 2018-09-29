.\" @(#)rand.w	1.1
.tr ~
rand [-\fIoption\fR(s)]; generate random sequence
.sp
Output is a vector of \fInumber\fR elements determined by the parameters \fIlow, high, multiplier \fRand \fIseed\fR.
Random numbers are first generated in the range 0 to 1, initialized by the seed.
Then if a \fImultiplier\fR is given, each number is multiplied accordingly.
The parameters are set by command options.
.sp
Options:
.VL 10 5 1
.LI c\fIn\fR
\fIn\fR elements per output line.
.LI h\fIn\fR  
\fIhigh\fR:=\fIn\fR.  If not given, \fIhigh\fR:=1.
.LI l\fIn\fR
\fIlow\fR:=\fIn\fR.  If not given, \fIlow\fR:=0.
.LI m\fIn\fR
\fImultiplier\fR:=\fIn\fR.  If not given, \fImultiplier\fR
is determined from \fIhigh\fR and \fIlow\fR.
.LI n\fIn\fR  
\fInumber\fR:=\fIn\fR.  If not given, \fInumber\fR:=10.
.LI s\fIn\fR  
\fIseed\fR:=\fIn\fR.  If not given, \fIseed\fR:=1.
.LE 1
Examples:
.VL 23 5 1
.LI rand
generates ten random numbers between 0 and 1.
.LI "rand -l10,m25,c3"
generates ten random numbers between 10 and 35, three per line.
.LE
