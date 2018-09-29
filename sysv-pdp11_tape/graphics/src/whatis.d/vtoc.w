.\" @(#)vtoc.w	1.2
vtoc [-\fIoption\fR(s)] [TTOC \fIfile\fR]; visual table of contents
.sp
Output is a GPS that describes a Visual Table of Contents
(VTOC or hierarchy chart) of the Textual Table of Contents (TTOC) entries from the input.
If no \fIfile\fR is given, the standard input is assumed.
TTOC entries have the form:
.sp
.ti 5
id [line weight,line style] "text" [mark]
.sp
where
.VL 17 5 1
.LI "id"
is an alternating sequence of numbers and dots.
.LI "line weight"
is either n for narrow, m for medium, or b for bold.
.LI "line style"
is either so for solid, do for dotted, dd for dot-dashed,
da for dashed, or ld for long-dashed.
.LI "text"
is a string of characters surrounded by quotes.
.LI "mark"
is a string of characters (surrounded by quotes if it contains spaces), with included dots being
escaped.
.sp
.LE
Entry example:  1.1 b,da "ABC" DEF
.sp
Options:
.VL 11 5 1
.LI "c"
Take the text as entered, (default is all upper case).
.LI "d"
Connect the boxes with diagonal lines.
.LI "h\fIn\fR"
Horizontal interbox space is \fIn\fR% of box width.
.LI "i"
Suppress the box id.
.LI "m"
Suppress the box mark.
.LI "s"
Do not compact boxes horizontially.
.LI "v\fIn\fR"
Vertical interbox space is \fIn\fR% of box height.
.LE 1
Example:  vtoc -i,v50 A.t
.br
Outputs a VTOC derived from the TTOC in \fIfile\fR A.t.
The VTOC boxes are without id fields,
the ratio of vertical box spacing to box height is 1/2.
