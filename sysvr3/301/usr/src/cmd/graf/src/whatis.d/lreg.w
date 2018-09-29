#ident	"@(#)graf:src/whatis.d/lreg.w	1.3"





lreg [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; linear regression

Output is the slope and intercept from a least squares
linear regression of each _v_e_c_t_o_r on a _b_a_s_e vector.  The _b_a_s_e
vector is specified using the F option.  If the _b_a_s_e is not
given, it is assumed to be ascending positive integers from
zero.

Options:

     F_v_e_c_t_o_r _v_e_c_t_o_r is the _b_a_s_e.
     i       Only output the intercept.
     o       Output the slope and intercept in "option" form
             (see _s_i_l_i_n_e).
     s       Only output the slope.

Example:   lreg -FA,i B
outputs the intercept from the linear regression of _v_e_c_t_o_r B
on _b_a_s_e vector A.
