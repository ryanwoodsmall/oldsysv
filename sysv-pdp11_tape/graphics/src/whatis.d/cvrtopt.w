.\" @(#)cvrtopt.w	1.1
cvrtopt [=\fIoption\fR(s)] [\fIarg\fR(s)];  options converter
.sp
\fICvrtopt\fR reformats \fIargs\fR (usually
the command line arguments of a
calling shell procedure)
to facilitate processing by shell procedures.
An \fIarg\fR is either a filename
(a string not beginning with
a '-', or a '-' by itself)
or an option string
(a string of options beginning with a '-').
Output is of the form:
.sp
.ti 5
-option -option . . . filename(s)
.P
All options appear singularly and preceding
any filenames.
Option names that take values (e.g.,
-r1.1)
or are two letters long
must be described through options to \fIcvrtopt\fR.
Output is to the standard output.
.sp
\fICvrtopt\fR is usually used with \fIset\fR in the following manner
as the first line of a \fIshell\fR procedure:
.ti 5
.sp
set - `cvrtopt [=option(s)] $@`
.sp
\fISet\fR will reset the command argument string
($1,$2,...) to the output
of \fIcvrtopt\fR.
The minus option to \fIset\fR turns off all
flags so that the options produced by \fIcvrtopt\fR
are not interpreted as options to \fIset\fR.
.br
.sp
Options:
.VL 14 5 1
.LI "s\fIstring\fR"
\fIString\fR accepts string values.
.LI "f\fIstring\fR"
\fIString\fR accepts floating point numbers as values.
.LI "i\fIstring\fR"
\fIString\fR accepts integers as values.
.LI "t\fIstring\fR"
\fIString\fR is a two letter option name that takes no value.
.LE
.sp
\fIString\fR is a one or two letter option name.
.sp
Example:
.ti
cvrtopt =ic,so,txi,fr -c1 -x,y,ofile A - B -r1.1e3,xi
.br
outputs
.ti
-c1 -x -y -ofile -r1.1e3 -xi A - B
.sp
