#ident	"@(#)graf:src/whatis.d/bar.w	1.3"





bar [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; build a bar chart

Output is a GPS that describes a bar chart display.  Input
is a _v_e_c_t_o_r of counts that describes the y-axis. By default,
x-axis will be labelled with integers beginning at 1; for
other labels, see _l_a_b_e_l.  If no _v_e_c_t_o_r is given, the
standard input is assumed.

Options:

     a         Suppress axes.
     b         Plot bar chart with bold weight lines,
               otherwise use medium.
     f         Do not build a frame around plot area.
     g         Suppress background grid.
     r_n        Put the bar chart in GPS region _n, where _n is
               between 1 and 25 inclusive.
     w_n        _n is the ratio of the bar width to center-
               to-center spacing expressed as a percentage.
               Default is 50, giving equal bar width and bar
               space.
     x_n (y_n)   Position the bar chart in the GPS universe
               with x-origin (y-origin) at _n.
     xa (ya)   Do not label x-axis (y-axis).
     yl_n       _n is the y-axis low tick value.
     yh_n       _n is the y-axis high tick value.

Example:   bar -r10,xa,w75 A
outputs the bar chart described by vector A, located in
region 10 of the GPS universe, with no x-axis labels.  Bar
width is 75% of the center-to-center spacing.
