g/#ident	"@(#)graf:src/toc.d/ed.ttoc.t	1.1"/
/                            Table of Contents/d
?D R A F T?-1d
1,.d
g/^$/d
g/^[	 ][	 ]*/s///
g/^-/d
g/^D R A F T/d
g/^.$/s/^/  /
g/  [0-9][0-9]*$/s//"&/
g/^[0-9]/s/  */&"/
0a
0.  "Table of Contents"
.
1,$g/^[0-9][0-9]*\./ p
Q
