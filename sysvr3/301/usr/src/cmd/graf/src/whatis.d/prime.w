#ident	"@(#)graf:src/whatis.d/prime.w	1.3"





prime [-_o_p_t_i_o_n(s)]; generate prime numbers

Output is a vector of _n_u_m_b_e_r elements determined by the
parameters _l_o_w and _h_i_g_h.  The parameters are set by command
options.

Options:

     c_n   _n elements per output line.
     h_n   _h_i_g_h:=_n.
     l_n   _l_o_w:=_n.  If not given, _l_o_w:=2.
     n_n   _n_u_m_b_e_r:=_n.  If not given, _n_u_m_b_e_r:=10.

Example:        prime -l200,h300
generates all prime numbers between 200 and 300.
