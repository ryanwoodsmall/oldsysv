#ident	"@(#)graf:src/whatis.d/point.w	1.3"





point [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; empirical cumulative density
function point

Output is a linearly interpolated value from the empirical
cumulative density function (e.c.d.f) for the input _v_e_c_t_o_r.
By default, _p_o_i_n_t returns the median (50% point).  If no
_v_e_c_t_o_r is given, the standard input is assumed.

Options:

     f_n   Return the (1/_n)*100 percent point from the
          e.c.d.f.
     p_n   Return the _n*100 percent point.
     n_n   Return the _nth element.
     s    The input is assumed to be sorted.

Example:   point -s,p.25 A
outputs the 25% point from the e.c.d.f.  _V_e_c_t_o_r A is assumed
to be sorted.
