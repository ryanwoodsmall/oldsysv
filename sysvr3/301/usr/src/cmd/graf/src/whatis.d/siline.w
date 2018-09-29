#ident	"@(#)graf:src/whatis.d/siline.w	1.3"





siline [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; generate a line given slope
and intercept

Output is a vector of values _s_l_o_p_e*_x+_i_n_t_e_r_c_e_p_t, where _x
takes on values from _v_e_c_t_o_r(_s).  If the _n option is given,
_v_e_c_t_o_r is the ascending positive integers.  If neither the _n
option nor a _v_e_c_t_o_r is given, _v_e_c_t_o_r comes from the standard
input.

Options:

     c_n   _n is the number of output elements per line.
     i_n   _n is the _i_n_t_e_r_c_e_p_t, 0 if not given.
     n_n   _n is the number of positive integers to be used
          for _x.
     s_n   _n is the _s_l_o_p_e, 1 if not given.

Example:   siline -`lreg -o,FA B` A
outputs a simple linear fit of vector B on vector A.  (The o
option of _l_r_e_g outputs the slope and intercept in option
form of B regressed on A.)
