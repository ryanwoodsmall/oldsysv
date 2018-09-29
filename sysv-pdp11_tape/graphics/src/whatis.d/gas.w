.\" @(#)gas.w	1.2
.tr ~
gas [-\fIoption\fR(s)]; generate additive sequence
.sp
Output is a vector of \fInumber\fR elements determined by the parameters \fIstart, terminate, \fRand \fIinterval\fR,
and by the formula x[i]~=~\fIstart\fR~+~(i*\fIinterval\fR~MOD~(\fIterminate\fR-\fIstart\fR+\fIinterval\fR)).
The parameters are set by command options.
.sp
Options:
.VL 10 5 1
.LI c\fIn\fR
\fIn\fR elements per output line.
.LI i\fIn\fR  
\fIinterval\fR:=\fIn\fR.  If not given, \fIinterval\fR:=1.
.LI n\fIn\fR  
\fInumber\fR:=\fIn\fR.  If not given, \fInumber\fR:=10,
unless \fIterminate\fR is given, then \fInumber:=|terminate-start|/interval\fR.
.LI s\fIn\fR
\fIstart\fR:=\fIn\fR.  If not given, \fIstart\fR:=1.
.LI t\fIn\fR  
\fIterminate\fR:=\fIn\fR.  If not given, \fIterminate\fR:=positive infinity.
The default value of \fInumber\fR usually terminates generation before positive 
infinity is reached.
.LE 1
Examples:
.VL 27 5 1
.LI gas
generates the numbers 1 to 10
.LI "gas -s.01,t.05,i.01"
generates .01,.02,.03,.04,.05
.LI "gas -s3,t5,i2,n4"
generates 3,5,3,5
.LE
