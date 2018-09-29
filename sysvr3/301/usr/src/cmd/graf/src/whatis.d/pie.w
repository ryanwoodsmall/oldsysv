#ident	"@(#)graf:src/whatis.d/pie.w	1.3"





pie [-_o_p_t_i_o_n(s)] [_f_i_l_e(s)]; build a pie chart

Output is a GPS that describes a pie chart.  Input is lines
of the form:

                [<control>] value [label]

The control field specifies the way that slice should be
handled.  Legal values are:

     i        The slice will not be drawn, though a space
              will be left for it.
     e        The slice is "exploded", or moved away from
              the pie.
     f        The slice is filled.  The angle of fill lines
              depends on the color of the slice.
     c_c_o_l_o_r   The slice is drawn in _c_o_l_o_r rather than the
              default black.  Legal values for _c_o_l_o_r are 'b'
              for black, 'r' for red, 'g' for green, and 'u'
              for blue.

The pie is drawn with the _v_a_l_u_e of each slice printed inside
and the _l_a_b_e_l printed outside.  If no _f_i_l_e is specified, the
standard input is assumed.

Options:

     b        Draw pie chart in bold weight lines, otherwise
              use medium.
     p        Output _v_a_l_u_e as a percentage of the total pie.
     pp_n      Only draw _n percent of a pie.
     pn_n      Output _v_a_l_u_e as a percentage, but total of
              percentages equals _n rather than 100.  pn100
              is equivalent to p.
     v        Do not output values.
     o        Output values around the outside of the pie.
     r_n       Put the pie chart in region _n, where _n is
              between 1 and 25 inclusive.
     x_n (y_n)  Position the pie chart in the GPS universe
              with x-origin (y-origin) at _n.

Example:  pie -pp80,pn80 A
will draw the pie chart specified by file A in 80% of a
circle and will output the _v_a_l_u_e_s as percentages that total
80.
