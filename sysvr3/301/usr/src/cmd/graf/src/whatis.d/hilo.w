#ident	"@(#)graf:src/whatis.d/hilo.w	1.3"





hilo [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; find high and low values

Output is the high and low values across all of the input
_v_e_c_t_o_r(_s). If no _v_e_c_t_o_r is given, the standard input is
assumed.

Options:

     h    Only output high value.
     l    Only output low value.
     o    Output high, low values in "option" form (see
          _p_l_o_t).
     ox   Output high, low values with x prepended.
     oy   Output high, low values with y prepended.

Example:   hilo -ox,l A B
outputs the lowest value in _v_e_c_t_o_r_s A and B with _x_l
prepended, i.e. _x_llowvalue.
