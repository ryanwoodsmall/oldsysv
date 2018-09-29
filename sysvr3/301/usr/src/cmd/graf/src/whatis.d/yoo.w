#ident	"@(#)graf:src/whatis.d/yoo.w	1.3"





yoo _f_i_l_e; pipe fitting

_Y_o_o is a piping primitive that deposits the output of a
pipeline into a _f_i_l_e used in the pipeline.  Note that,
without _y_o_o, this is not usually successful as it causes a
read and write on the same file simultaneously.

Example:   cat A B | yoo B
stores the concatenation of files A and B in B.
