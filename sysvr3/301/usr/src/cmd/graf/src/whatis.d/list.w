#ident	"@(#)graf:src/whatis.d/list.w	1.3"





list [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; list vector

Output is a listing of the elements of the input _v_e_c_t_o_r(s).
If no _v_e_c_t_o_r is given, the standard input is assumed.

Options:

     c_n       _n is the number of output elements per line.
              Five is the default value.
     d_s_t_r_i_n_g  The characters in _s_t_r_i_n_g serve as delimiters.
              Only elements that are delimited by these
              characters will be listed.  The whitespace
              characters space, tab, and newline are always
              delimiters.

Examples:

     list -c3 A     outputs each element of A, three per
                    line.
     list -d\\, A   outputs each element of A that is
                    delimited by commas or whitespace, five
                    per line.  A comma requires two
                    backslashes because it is a special
                    character for _l_i_s_t.
