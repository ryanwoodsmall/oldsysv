#ident	"@(#)graf:src/whatis.d/qsort.w	1.3"





qsort [-_o_p_t_i_o_n] [_v_e_c_t_o_r(s)]; quick sort

Output is a vector of the elements from the input _v_e_c_t_o_r in
ascending order.  If no _v_e_c_t_o_r is given, the standard input
is assumed.

Option:   c_n  _n is the number of output elements per line.

Example:   qsort -c3 A
outputs the the elements of A in ascending order.
