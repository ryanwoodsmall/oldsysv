#ident	"@(#)graf:src/whatis.d/gas.w	1.4"





gas [-_o_p_t_i_o_n(s)]; generate additive sequence

Output is a vector of _n_u_m_b_e_r elements determined by the
parameters _s_t_a_r_t, _t_e_r_m_i_n_a_t_e, and _i_n_t_e_r_v_a_l, and by the
formula x[i] = _s_t_a_r_t + (i*_i_n_t_e_r_v_a_l MOD (_t_e_r_m_i_n_a_t_e-
_s_t_a_r_t+_i_n_t_e_r_v_a_l)).  The parameters are set by command
options.

Options:

     c_n   _n elements per output line.
     i_n   _i_n_t_e_r_v_a_l:=_n.  If not given, _i_n_t_e_r_v_a_l:=1.
     n_n   _n_u_m_b_e_r:=_n.  If not given, _n_u_m_b_e_r:=10, unless
          _t_e_r_m_i_n_a_t_e is given, then _n_u_m_b_e_r:=|_t_e_r_m_i_n_a_t_e-
          _s_t_a_r_t|/_i_n_t_e_r_v_a_l.
     s_n   _s_t_a_r_t:=_n.  If not given, _s_t_a_r_t:=1.
     t_n   _t_e_r_m_i_n_a_t_e:=_n.  If not given, _t_e_r_m_i_n_a_t_e:=positive
          infinity.  The default value of _n_u_m_b_e_r usually
          terminates generation before positive infinity is
          reached.

Examples:

     gas                   generates the numbers 1 to 10
     gas -s.01,t.05,i.01   generates .01,.02,.03,.04,.05
     gas -s3,t5,i2,n4      generates 3,5,3,5
