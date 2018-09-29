#ident	"@(#)graf:src/whatis.d/pair.w	1.3"





pair [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; pair element groups

Output is a vector with elements taken alternately from a
_b_a_s_e vector and from a _v_e_c_t_o_r.  The _b_a_s_e vector is specified
either with the F option, or else it comes from the standard
input.  _V_e_c_t_o_r(_s) are specified either on the command line
or else one may come from the standard input.  If both the
_b_a_s_e and _v_e_c_t_o_r come from the standard input, _b_a_s_e precedes
_v_e_c_t_o_r.

Options:

     c_n        _n is the number of output elements per line.
     F_v_e_c_t_o_r   _v_e_c_t_o_r is the _b_a_s_e.
     x_n        _n is the number of elements taken from the
               _b_a_s_e for each one element taken from _v_e_c_t_o_r.

Example:   pair -x3,FA B
Outputs a vector with three elements from A, then one from
B, then three from A, one from B, and so on.
