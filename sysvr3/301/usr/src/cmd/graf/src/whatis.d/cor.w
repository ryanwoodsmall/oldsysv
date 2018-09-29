#ident	"@(#)graf:src/whatis.d/cor.w	1.3"





cor [-_o_p_t_i_o_n] [_v_e_c_t_o_r(s)]; ordinary correlation coefficient

Output is the ordinary correlation coefficient between a
_b_a_s_e vector and another _v_e_c_t_o_r.  The _b_a_s_e vector is
specified using the F option.  If the _b_a_s_e or _v_e_c_t_o_r is not
given, it is assumed to come from the standard input.  Each
_v_e_c_t_o_r is compared to the _b_a_s_e.  Both _b_a_s_e and _v_e_c_t_o_r must
be of the same length.

Option:   F_v_e_c_t_o_r  _v_e_c_t_o_r is the _b_a_s_e.

Example:   cor -FA B C
outputs the ordinary correlation coefficients between
_v_e_c_t_o_r_s A and B, and _v_e_c_t_o_r_s A and C.
