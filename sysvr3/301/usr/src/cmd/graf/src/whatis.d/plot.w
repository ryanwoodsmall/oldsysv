#ident	"@(#)graf:src/whatis.d/plot.w	1.4"





plot [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; plot an x-y graph

Output is a GPS that describes an x-y graph.  Input is one
or more _v_e_c_t_o_r(_s).  Y-axis values come from _v_e_c_t_o_r(_s), x-
axis values from the F option.  Axis scales are determined
from the first _v_e_c_t_o_r(_s) plotted.  If no _v_e_c_t_o_r is given,
the standard input is assumed.

Options:

     a         Suppress axes.
     b         Plot graph with bold weight lines, otherwise
               use medium.
     c_c_h_a_r(_s)  Use _c_h_a_r(_s) for plotting characters, implies
               option m.  The first character of _c_h_a_r(_s) is
               used to mark the first graph, the second is
               used to mark the second graph, etc.
     d         Do not connect plotted points, implies option
               m.
     F_v_e_c_t_o_r   Use _v_e_c_t_o_r for x-values, otherwise the
               positive integers are used.
     g         Suppress background grid.
     m         Mark the plotted points.
     r_n        Put the graph in GPS region _n, where _n is
               between 1 and 25 inclusive.
     x_n (y_n)   Position the graph in the GPS universe with
               x-origin (y-origin) at _n.
     xa (ya)   Omit x-axis (y-axis) labels.
     xi_n (yi_n) _n is the x-axis (y-axis) tick increment.
     xl_n (yl_n) _n is the x-axis (y-axis) low tick value.
     xh_n (yh_n) _n is the x-axis (y-axis) high tick value.
     xn_n (yn_n) _n is the approximate number of ticks on the
               x-axis (y-axis).
     xt (yt)   Omit x-axis (y-axis) title.

Examples:

     plot A                    plots _v_e_c_t_o_r A against the
                               positive integers.
     plot -r5,yl0,xa,FA B      plots _v_e_c_t_o_r B against _v_e_c_t_o_r
                               A, y-axis ticks begin at 0,
                               no x-axis labels are printed,
                               the plot is placed in region
                               5 of the GPS universe.
     plot -`hilo -oy A B` A B  plots _v_e_c_t_o_r_s A and B against
                               the positive integers, with
                               y-axis ticks going from the
                               lowest value in A and B to
                               the highest value in A and B.
     plot -FA,FB C D E         plots _v_e_c_t_o_r_s C against A, D
                               and E against B.  Y-axis
                               scale is determined from C,
                               x-axis from A.
