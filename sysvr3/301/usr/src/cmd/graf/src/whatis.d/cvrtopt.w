#ident	"@(#)graf:src/whatis.d/cvrtopt.w	1.3"





cvrtopt [=_o_p_t_i_o_n(s)] [_a_r_g(s)];  options converter

_C_v_r_t_o_p_t reformats _a_r_g_s (usually the command line arguments
of a calling shell procedure) to facilitate processing by
shell procedures.  An _a_r_g is either a filename (a string not
beginning with a '-', or a '-' by itself) or an option
string (a string of options beginning with a '-').  Output
is of the form:

     -option -option . . . filename(s)

All options appear singularly and preceding any filenames.
Option names that take values (e.g., -r1.1) or are two
letters long must be described through options to _c_v_r_t_o_p_t.
Output is to the standard output.

_C_v_r_t_o_p_t is usually used with _s_e_t in the following manner as
the first line of a _s_h_e_l_l procedure:

     set - `cvrtopt [=option(s)] $@`

_S_e_t will reset the command argument string ($1,$2,...) to
the output of _c_v_r_t_o_p_t.  The minus option to _s_e_t turns off
all flags so that the options produced by _c_v_r_t_o_p_t are not
interpreted as options to _s_e_t.

Options:

     s_s_t_r_i_n_g  _S_t_r_i_n_g accepts string values.
     f_s_t_r_i_n_g  _S_t_r_i_n_g accepts floating point numbers as
              values.
     i_s_t_r_i_n_g  _S_t_r_i_n_g accepts integers as values.
     t_s_t_r_i_n_g  _S_t_r_i_n_g is a two letter option name that takes
              no value.

_S_t_r_i_n_g is a one or two letter option name.

Example:  cvrtopt =ic,so,txi,fr -c1 -x,y,ofile A - B
-r1.1e3,xi
outputs -c1 -x -y -ofile -r1.1e3 -xi A - B

