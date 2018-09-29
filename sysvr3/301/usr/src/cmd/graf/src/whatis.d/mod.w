#ident	"@(#)graf:src/whatis.d/mod.w	1.3"





mod [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; modulo function

Output is a vector with each element being the remainder of
dividing the corresponding element from the input _v_e_c_t_o_r(_s)
by the _m_o_d_u_l_u_s.  If no _v_e_c_t_o_r is given, the standard input
is assumed.

Options:

     c_n        _n is the number of output elements per line.
     m_n        _n is the _m_o_d_u_l_u_s.  If not given, 2 is used.

Example:   mod -m8,c3 A
outputs the elements of A modulo 8, three per line.
