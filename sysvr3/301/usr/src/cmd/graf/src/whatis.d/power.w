#ident	"@(#)graf:src/whatis.d/power.w	1.3"





power [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; power function

Output is a vector with each element being a power of the
corresponding element from the input _v_e_c_t_o_r(_s).  If no
_v_e_c_t_o_r is given, the standard input is assumed.

Options:

     c_n        _n is the number of output elements per line.
     p_n        Input elements are raised to the _nth power.
               If not given, 2 is used.

Example:   power -p3.5,c3 A
outputs the 3.5th power of each element of A, three per
line.
