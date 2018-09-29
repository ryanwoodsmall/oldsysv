#ident	"@(#)graf:src/whatis.d/af.w	1.3"





af [-_o_p_t_i_o_n(s)] _e_x_p_r_e_s_s_i_o_n(s); arithmetic function

Output is the result of executing the _e_x_p_r_e_s_s_i_o_n(s) once per
complete set of input values.  Input comes from vectors
specified in _e_x_p_r_e_s_s_i_o_n.

Expression operands are:

     Vectors:        filenames with the restriction that
                     they must begin with a letter and be
                     composed only from letters, digits,
                     '_', and '.'.  The first unknown
                     filename (one not in the current
                     directory) references the standard
                     input.
     Functions:      the name of a command followed by the
                     command arguments in parentheses.
                     Arguments are written as on the command
                     line.
     Constants:      floating point and integer (but not E
                     notation).

Expression operators are, in order of decreasing precedence:

     '_v              next value from vector _v.
     _x^_y, -_x         _x raised to the _y power, negation of _x;
                     both associate right to left.
     _x*_y, _x/_y, _x%_y   _x multiplied by, divided by, modulo _y,
                     respectively; all associate left to
                     right.
     _x+_y, _x-_y        _x plus, minus _y, respectively; both
                     associate left to right.
     _x,_y             output the value of _x followed by the
                     value of _y; associates from left to
                     right

Parenthesis may be used to alter precedence.  Because many
of the operator characters are special to the shell it is
good practice to surround _e_x_p_r_e_s_s_i_o_n_s in quotes.

Options:

     c_n   _n elements per line in the output.
     t    output is titled from the vector on the standard
          input.
     v    verbose mode, function expansions are echoed.

Examples:

     af "3+4*5"          yields 23

     af "A,'A,A+'A,B"    yields a four column matrix with
                         columns of
                           1) odd elements from A,
                           2) even elements from A,
                           3) sum of adjacent odd and even
                         elements, and
                           4) elements from B.

     af "sin(A)^2"       yields the square of the sin of the
                         elements of A.
