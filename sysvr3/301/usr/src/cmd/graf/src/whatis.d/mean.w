#ident	"@(#)graf:src/whatis.d/mean.w	1.3"





mean [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; mean

Output is the mean of the elements in the input _v_e_c_t_o_r(_s).
The input may optionally be trimmed.  If no _v_e_c_t_o_r is given,
the standard input is assumed.

Options:

     f_n        Trim (1/_n)*r elements from each end, where r
               is the rank of the input vector.
     p_n        Trim _n*r elements from each end, _n is between
               0 and .5.
     n_n        Trim _n elements from each end.

Example:   mean -p.25 A
outputs the mean of the middle 50% of the elements of A,
i.e., A is trimmed by 25% of its elements from both ends.
