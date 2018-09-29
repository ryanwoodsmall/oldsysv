.\" @(#)title.w	1.1
title [\fI-option\fR(s)] [\fIfile\fR(s)]; title a vector a or GPS
.sp
\fITitle\fR prepends \fItitle\fR to a vector or appends \fItitle\fR to a GPS.
Input is taken from \fIfile(s)\fR if given, else from the standard input.
.sp
Options:
.VL 15 5 1
.LI b
Make the GPS \fItitle\fR bold.
.LI c
Retain lower case letters in \fItitle\fR,
otherwise all letters are upper case.
.LI "l\fIstring\fR"
For a GPS, generate a lower \fItitle:=string\fR.
.LI "u\fIstring\fR"
For a GPS, generate an upper \fItitle:=string\fR.
.LI "v\fIstring\fR"
For a vector, \fItitle:=string\fR.
.LE
.sp
Examples:
.VL 30 5 1
.tr Q"
.LI "title -c,vQVector AQ A"
Assuming A is a vector,
the title `Vector A' is prepended to A.
.LI "title -lLower,uUpper A.g"
Assuming A.g is a GPS,
`LOWER' is appended as a lower title and `UPPER' as an upper title.
.LE
