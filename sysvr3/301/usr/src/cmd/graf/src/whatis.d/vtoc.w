#ident	"@(#)graf:src/whatis.d/vtoc.w	1.4"





vtoc [-_o_p_t_i_o_n(s)] [TTOC _f_i_l_e]; visual table of contents

Output is a GPS that describes a Visual Table of Contents
(VTOC or hierarchy chart) of the Textual Table of Contents
(TTOC) entries from the input.  If no _f_i_l_e is given, the
standard input is assumed.  TTOC entries have the form:

     id [line weight,line style] "text" [mark]

where

     id          is an alternating sequence of numbers and
                 dots.
     line weight is either n for narrow, m for medium, or b
                 for bold.
     line style  is either so for solid, do for dotted, dd
                 for dot-dashed, da for dashed, or ld for
                 long-dashed.
     text        is a string of characters surrounded by
                 quotes.
     mark        is a string of characters (surrounded by
                 quotes if it contains spaces), with
                 included dots being escaped.

Entry example:  1.1 b,da "ABC" DEF

Options:

     c     Take the text as entered, (default is all upper
           case).
     d     Connect the boxes with diagonal lines.
     h_n    Horizontal interbox space is _n% of box width.
     i     Suppress the box id.
     m     Suppress the box mark.
     s     Do not compact boxes horizontially.
     v_n    Vertical interbox space is _n% of box height.

Example:  vtoc -i,v50 A.t
Outputs a VTOC derived from the TTOC in _f_i_l_e A.t.  The VTOC
boxes are without id fields, the ratio of vertical box
spacing to box height is 1/2.
