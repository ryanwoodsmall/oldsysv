#ident	"@(#)graf:src/whatis.d/round.w	1.3"





round [-_o_p_t_i_o_n(s)] [_v_e_c_t_o_r(s)]; rounded value

Output is the rounded value for each element of the input
_v_e_c_t_o_r(_s).  If no _v_e_c_t_o_r is given, the standard input is
assumed.

Options:

     c_n   _n is the number of output elements per line.
     p_n   _n is the number of places following the decimal
          point rounded to.  _n is in the range 0 to 9, 0 by
          default.
     s_n   _n is the number of significant digits rounded to.
          _n is in the range 0 to 9, 9 by default.

Example:   round -s2,c3 A
outputs the value of each element of A rounded to two
significant digits, three per line.
