#ident	"@(#)graf:src/whatis.d/hpd.w	1.3"





hpd [-_o_p_t_i_o_n(s)] [GPS _f_i_l_e_s(s)];  display GPS on a a Hewlett
Packard 7221A Graphics Plotter

Output is scope code for a Hewlett Packard 7221A Plotter.  A
viewing window is computed from the maximum and minimum
points in the GPS _f_i_l_e(s) unless the _r or _u option is
provided.  If no _f_i_l_e is given, the standard input is
assumed.

Options:

     c_n        Select character set _n, _n between 0 and 5
               (see the HP 7221A Plotter Operating and
               Programming Manual, Apendix A).
     p_n        Select pen numbered _n, _n between 1 and 4
               inclusive.
     r_n        Window on GPS region _n, _n between 1 and 25
               inclusive.
     s_n        Slant characters _n degrees counterclockwise
               from the vertical.
     u         Window on the entire GPS universe.
     xd_n       Set x displacement of the viewport's lower
               left corner to _n inches.
     xv_n       Set width of viewport to _n inches.
     yd_n       Set y displacement of the viewport's lower
               left corner to _n inches.
     yv_n       Set height of viewport to _n inches.

Example:   hpd -p3,s-20,xv12.5,yv10 A.g B.g
displays A.g and B.g together using pen number three and
slanting characters forward by 20 degrees.  The viewing
window is built to include all of both files, and the device
viewport is 12.5 by 10 inches.
