#ident	"@(#)graf:src/whatis.d/ged.w	1.3"





ged [-_o_p_t_i_o_n(s)] [GPS _f_i_l_e_s(s)];  graphical editor

_G_e_d allows displaying and editing of GPS.

Options:

     R         Invoke the editor in a restricted _s_h_e_l_l
               environment.
     e         Do not erase screen before initial display.
     r_n        Window on GPS region _n, _n between 1 and 25
               inclusive.
     u         Window on the entire GPS universe.

Example:   ged A.g B.g
displays A.g and B.g and prepares them for editing.
