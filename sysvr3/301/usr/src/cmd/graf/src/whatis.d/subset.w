#ident	"@(#)graf:src/whatis.d/subset.w	1.3"





subset [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; generate a subset

Output are elements selected from the input based on a _k_e_y
and _o_p_t_i_o_n(_s).  If no _v_e_c_t_o_r is given, the standard input is
assumed.

Selection is as follows:

     If a _m_a_s_t_e_r vector is given, then the _k_e_y for the _ith
     element of the input is the _ith element of _m_a_s_t_e_r,
     otherwise the _k_e_y is the input element itself.  In
     either case _i goes from _s_t_a_r_t to _t_e_r_m_i_n_a_t_e.

     The input element is selected if the _k_e_y is either
     above _a_b_o_v_e, below _b_e_l_o_w, or equal to _p_i_c_k, and not
     equal to _l_e_a_v_e.  If neither _a_b_o_v_e, _b_e_l_o_w, nor _p_i_c_k is
     given, then the element is selected if it is not equal
     to _l_e_a_v_e.

Options:

     a_n        _a_b_o_v_e := _n.
     b_n        _b_e_l_o_w := _n.
     c_n        _n elements per output line.
     F_v_e_c_t_o_r   _v_e_c_t_o_r is the _m_a_s_t_e_r.
     i_n        _i_n_t_e_r_v_a_l := _n, default is 1.
     l_n        _l_e_a_v_e := _n.
     nl        leave elements whose index is given in
               _m_a_s_t_e_r.
     np        pick elements whose index is given in _m_a_s_t_e_r.
     p_n        _p_i_c_k := _n.
     s_n        _s_t_a_r_t := _n, default is 1.
     t_n        _t_e_r_m_i_n_a_t_e := _n, default is 32767.

Examples:

     subset -i2,s2 A   outputs the even elements of A.
     subset -FB,p1 A   for each element in B with a 1,
                       output the corresponding element of
                       A.
