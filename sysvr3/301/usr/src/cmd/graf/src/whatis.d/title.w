#ident	"@(#)graf:src/whatis.d/title.w	1.3"





title [-_o_p_t_i_o_n(s)] [_f_i_l_e(s)]; title a vector a or GPS

_T_i_t_l_e prepends _t_i_t_l_e to a vector or appends _t_i_t_l_e to a GPS.
Input is taken from _f_i_l_e(_s) if given, else from the standard
input.

Options:

     b         Make the GPS _t_i_t_l_e bold.
     c         Retain lower case letters in _t_i_t_l_e, otherwise
               all letters are upper case.
     l_s_t_r_i_n_g   For a GPS, generate a lower _t_i_t_l_e:=_s_t_r_i_n_g.
     u_s_t_r_i_n_g   For a GPS, generate an upper _t_i_t_l_e:=_s_t_r_i_n_g.
     v_s_t_r_i_n_g   For a vector, _t_i_t_l_e:=_s_t_r_i_n_g.

Examples:

     title -c,v"Vector A" A   Assuming A is a vector, the
                              title `Vector A' is prepended
                              to A.
     title -lLower,uUpper A.g Assuming A.g is a GPS, `LOWER'
                              is appended as a lower title
                              and `UPPER' as an upper title.
