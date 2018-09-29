#ident	"@(#)graf:src/whatis.d/label.w	1.3"





label [-_o_p_t_i_o_n(s)] [GPS _f_i_l_e(s)]; label the axis of a data
plot

_L_a_b_e_l appends axis labels from a _l_a_b_e_l _f_i_l_e to a GPS of a
data plot (like that produced by _h_i_s_t, _b_a_r and _p_l_o_t).  Each
line of the _l_a_b_e_l _f_i_l_e is taken as one label.  For _p_l_o_t
labels, be sure to include _x_i1 on the _p_l_o_t command line (see
_p_l_o_t).  Blank lines yield null labels.  Either the GPS or
the _l_a_b_e_l _f_i_l_e, but not both, may come from the standard
input.

Options:

     b         Assume the input is a bar chart.
     c         Retain lower case letters in labels,
               otherwise all letters are upper case.
     F_f_i_l_e     _f_i_l_e is the _l_a_b_e_l _f_i_l_e.
     h         Assume the input is a histogram.
     p         Assume the input is an x-y plot.  This is the
               default.
     r_n        Labels are rotated _n degrees.  The pivot
               point is the first character.
     x         Label the x-axis.  This is the default.
     xu        Label the upper x-axis, i.e., the top of the
               plot.
     y         Label the y-axis.
     yr        Label the right y-axis, i.e., the right side
               of the plot.

Examples:

     label -Flabs A.g     The file A.g, assumed to be an x-y
                          plot, is labelled with labels from
                          the file labs.
     label -yr,r-45 A.g   The file A.g is labelled on the
                          right y-axis with labels from the
                          standard input.  The labels are
                          printed at 45 degrees below the
                          horizontal.
