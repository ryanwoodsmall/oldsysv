#ident	"@(#)graf:src/whatis.d/floor.w	1.3"





floor [-_o_p_t_i_o_n] [_v_e_c_t_o_r(s)]; floor function

Output is a vector with each element being the largest
integer less than the corresponding element from the input
_v_e_c_t_o_r(_s).  If no _v_e_c_t_o_r is given, the standard input is
assumed.

Option:   c_n  _n is the number of output elements per line.

Example:   floor -c3 A
outputs the floor of each element of A, three per line.
