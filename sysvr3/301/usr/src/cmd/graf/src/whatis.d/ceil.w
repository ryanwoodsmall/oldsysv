#ident	"@(#)graf:src/whatis.d/ceil.w	1.3"





ceil [-_o_p_t_i_o_n] [_v_e_c_t_o_r(s)]; ceiling function

Output is a vector with each element being the smallest
integer greater than the corresponding element from the
input _v_e_c_t_o_r(_s).  If no _v_e_c_t_o_r is given, the standard input
is assumed.

Options:   c_n  _n is the number of output elements per line.

Example:   ceil -c3 A
outputs the ceiling of each element of A, three per line.
