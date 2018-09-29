.\" @(#)list.w	1.1
list [-\fIoption\fR(s)] [\fIvector\fR(s)]; list vector
.sp
Output is a listing of the elements of the input \fIvector\fR(s).
If no \fIvector\fR is given, the standard input is assumed.
.sp
Options:
.VL 14 5 1
.LI "c\fIn\fR"
\fIn\fR is the number of output elements per line.
Five is the default value.
.LI "d\fIstring\fR"
The characters in \fIstring\fR serve as delimiters.
Only elements that are delimited by these characters will be listed.
The whitespace characters space, tab, and newline are always delimiters.
.LE
.sp
Examples:
.VL 20 5 1
.LI "list -c3 A"
outputs
each element of A, three per line.
.LI "list -d\e\e, A"
outputs each element of A that is delimited by commas
or whitespace, five per line.
A comma requires two backslashes because it is a special character for \fIlist\fR.
.LE
