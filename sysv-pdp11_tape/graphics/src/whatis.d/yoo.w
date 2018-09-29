.\" @(#)yoo.w	1.1
yoo \fIfile\fR; pipe fitting
.sp
\fIYoo\fR is a piping primitive that deposits the output of a pipeline
into a \fIfile\fR used in the pipeline.
Note that, without \fIyoo\fR, this is not usually successful as it causes a read and write
on the same file simultaneously.
.sp
Example:   cat A B | yoo B
.br
stores the concatenation of files A and B in B.
