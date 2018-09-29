.\" @(#)label.w	1.1
label [-\fIoption\fR(s)] [GPS \fIfile\fR(s)]; label the axis of a data plot
.sp
\fILabel\fR appends axis labels from a \fIlabel file\fR to a GPS of a data plot
(like that produced by \fIhist\fR, \fIbar\fR and \fIplot\fR).
Each line of the \fIlabel file\fR is taken as one label.
For \fIplot\fR labels, be sure to include \fIxi\fR1 on the \fIplot\fR command line
(see \fIplot\fR).
Blank lines yield null labels.
Either the GPS or the \fIlabel file\fR, but not both,
may come from the standard input.
.sp
Options:
.VL 15 5 1
.LI b
Assume the input is a bar chart.
.LI c
Retain lower case letters in labels,
otherwise all letters are upper case.
.LI "F\fIfile\fR"
\fIfile\fR is the \fIlabel file\fR.
.LI h
Assume the input is a histogram.
.LI p
Assume the input is an x-y plot.  This is the default.
.LI r\fIn\fR
Labels are rotated \fIn\fR degrees.
The pivot point is the first character.
.LI x
Label the x-axis.  This is the default.
.LI xu
Label the upper x-axis, i.e., the top of the plot.
.LI y
Label the y-axis.
.LI yr
Label the right y-axis, i.e., the right side of the plot.
.LE
.sp
Examples:
.VL 26 5 1
.LI "label -Flabs A.g"
The file A.g, assumed to be an x-y plot, is labelled with
labels from the file labs.
.LI "label -yr,r-45 A.g"
The file A.g is labelled on the right y-axis with labels
from the standard input.
The labels are printed at 45 degrees below the horizontal.
.LE
