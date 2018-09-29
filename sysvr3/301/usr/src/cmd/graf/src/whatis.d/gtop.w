#ident	"@(#)graf:src/whatis.d/gtop.w	1.3"





gtop [-_o_p_t_i_o_n(s)] [GPS _f_i_l_e(s)]; GPS to _p_l_o_t(5) filter

_G_t_o_p transforms a GPS into _p_l_o_t(_5) commands displayable by
_p_l_o_t(_1) filters.  Input is taken from _f_i_l_e if given, else
from the standard input.  GPS objects are translated if they
fall within the window that circumscribes the first _f_i_l_e
unless an _o_p_t_i_o_n is given.  Output is to the standard
output.

Options:

     r_n   Translate objects in GPS region _n.
     u    Translate all objects in the GPS universe.

Example:  gtop A.g
translates the GPS in file A.g into _p_l_o_t(_5) commands.
