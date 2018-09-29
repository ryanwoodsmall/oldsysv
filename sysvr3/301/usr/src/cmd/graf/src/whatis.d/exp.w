#ident	"@(#)graf:src/whatis.d/exp.w	1.3"





exp [-_o_p_t_i_o_n] [_v_e_c_t_o_r(s)]; exponential function

Output is a vector with elements e raised to the _x power,
where e is 2.71828, approximately, and _x are the elements
from the input _v_e_c_t_o_r(_s).  If no _v_e_c_t_o_r is given, the
standard input is assumed.

Option:   c_n  _n is the number of output elements per line.

Example:   exp -c3 A
outputs the exponential of each element of A, three per
line.
