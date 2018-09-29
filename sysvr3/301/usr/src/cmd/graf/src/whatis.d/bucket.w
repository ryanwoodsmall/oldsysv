#ident	"@(#)graf:src/whatis.d/bucket.w	1.3"





bucket [-_o_p_t_i_o_n(s)] [sorted _v_e_c_t_o_r(s)]; generate buckets and
counts

Output is a vector with odd values being bucket limits (in
parentheses) and even values being the number of elements
from the input within the limits.  Input is assumed to be
sorted.  If no input _v_e_c_t_o_r(_s) are given, the standard input
is assumed.  Unless otherwise specified, bucket limits are
generated based on the input data and the rule:  num buckets
= 1 + log2(num elements).

Options:

     a_n        choose limits such that _n is the average
               count per bucket.
     c_n        _n elements per line in the output.
     F_v_e_c_t_o_r   take limit values from _v_e_c_t_o_r.
     h_n        _n is the highest limit.
     i_n        _n is the interval between limits.
     l_n        _l is the lowest limit.
     n_n        _n is the number of buckets.

Example:   bucket -a12,l-5 A
outputs limits and counts for the elements of A, where the
lowest limit is -5 and the average bucket count is 12.
