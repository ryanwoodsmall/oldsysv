#ident	"@(#)graf:src/whatis.d/td.w	1.3"





td [-_o_p_t_i_o_n(s)] [GPS _f_i_l_e_s(s)];  display GPS on a Tektronix
4014

Output is scope code for a Tektronix 4014 terminal.  A
viewing window is computed from the maximum and minimum
points in the first _f_i_l_e unless _o_p_t_i_o_n_s are provided.  If no
_f_i_l_e is given, the standard input is assumed.

Options:

     r_n        Window on GPS region _n, _n between 1 and 25
               inclusive.
     u         Window on the entire GPS universe.
     e         Do not erase screen before initiating
               display.

Example:   td A.g B.g
displays A.g and B.g.  The viewing window is built to
include all of A.g.
