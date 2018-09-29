#ident	"@(#)graf:src/whatis.d/root.w	1.4"





root [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; root function

Output is a vector with each element being the _r_o_o_t root of
the corresponding element from the input _v_e_c_t_o_r(_s).  If no
_v_e_c_t_o_r is given, the standard input is assumed.

Options:

     c_n        _n is the number of output elements per line.
     r_n        _r_o_o_t:=_n.  If not given, _r_o_o_t:=2.

Example:   root -r3.5,c3 A
outputs the 3.5th root of each element of A, three per line.
