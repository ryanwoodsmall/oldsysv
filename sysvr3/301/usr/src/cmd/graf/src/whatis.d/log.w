#ident	"@(#)graf:src/whatis.d/log.w	1.3"





log [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; logarithm

Output is the logarithm for each element of the input
_v_e_c_t_o_r(_s).  If no _v_e_c_t_o_r is given, the standard input is
assumed.

Options:

     b_n   _n is the logarithm base.  If not given, 2.71828...
          is used.
     c_n   _n is the number of output elements per line.

Example:   log -b2,c3 A
outputs the logarithm base 2 of each element of A, three per
line.
