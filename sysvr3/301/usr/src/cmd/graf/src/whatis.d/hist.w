#ident	"@(#)graf:src/whatis.d/hist.w	1.3"





hist [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; build a histogram

Output is a GPS that describes a histogram display.  Input
is a _v_e_c_t_o_r of odd rank, with odd elements being bucket
limits and even elements being bucket counts (see _b_u_c_k_e_t).
If no _v_e_c_t_o_r is given, the standard input is assumed.

Options:

     a         Suppress axes.
     b         Plot histogram with bold weight lines,
               otherwise use medium.
     f         Do not build a frame around plot area.
     g         Suppress background grid.
     r_n        Put the histogram in GPS region _n, where _n is
               between 1 and 25 inclusive.
     x_n (y_n)   Position the histogram in the GPS universe
               with x-origin (y-origin) at _n.
     xa (ya)   Do not label x-axis (y-axis).
     yl_n       _n is the y-axis low tick value.
     yh_n       _n is the y-axis high tick value.

Example:   hist -r5,ya A
outputs the histogram described by _v_e_c_t_o_r A and locates it
in region 5 of the GPS universe, with no y-axis labels.
