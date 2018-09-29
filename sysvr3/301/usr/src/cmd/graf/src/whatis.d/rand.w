#ident	"@(#)graf:src/whatis.d/rand.w	1.3"





rand [-_o_p_t_i_o_n(s)]; generate random sequence

Output is a vector of _n_u_m_b_e_r elements determined by the
parameters _l_o_w, _h_i_g_h, _m_u_l_t_i_p_l_i_e_r and _s_e_e_d.  Random numbers
are first generated in the range 0 to 1, initialized by the
seed.  Then if a _m_u_l_t_i_p_l_i_e_r is given, each number is
multiplied accordingly.  The parameters are set by command
options.

Options:

     c_n   _n elements per output line.
     h_n   _h_i_g_h:=_n.  If not given, _h_i_g_h:=1.
     l_n   _l_o_w:=_n.  If not given, _l_o_w:=0.
     m_n   _m_u_l_t_i_p_l_i_e_r:=_n.  If not given, _m_u_l_t_i_p_l_i_e_r is
          determined from _h_i_g_h and _l_o_w.
     n_n   _n_u_m_b_e_r:=_n.  If not given, _n_u_m_b_e_r:=10.
     s_n   _s_e_e_d:=_n.  If not given, _s_e_e_d:=1.

Examples:

     rand              generates ten random numbers between
                       0 and 1.
     rand -l10,m25,c3  generates ten random numbers between
                       10 and 35, three per line.
