#ident	"@(#)graf:src/whatis.d/cusum.w	1.3"





cusum [-_o_p_t_i_o_n] [_v_e_c_t_o_r(s)]; cumulative sum

Output is a vector with the _ith element being the sum of the
first _i elemtents from the input _v_e_c_t_o_r.  If no _v_e_c_t_o_r is
given, the standard input is assumed.

Option:   c_n  _n is the number of output elements per line.

Example:   cusum -c3 A
outputs the cumulative sum of the elements of A, three per
line.
