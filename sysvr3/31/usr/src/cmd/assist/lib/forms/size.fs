#ident	"@(#)forms.files:size.fs	1.5"
00000size
00701060130FILE NAME(S):
01703060001PRINT NUMBERS IN OCTAL [-o] (y/n):
02705060001PRINT NUMBERS IN HEXADECIMAL [-x] (y/n):
03707060001ALSO INCLUDE "NOLOAD" SECTIONS [-n] (y/n):
04709060001PROVIDE VERBOSE OUTPUT [-f] (y/n):
05711060001PRINT VERSION NUMBER OF "SIZE" COMMAND [-V] (y/n):
06713060030ADDITIONS TO THE COMMAND LINE:

00040-
01011-
02001
03000
04000
05021V
06042-


"size" prints size information for each section of
an object file.  The size of the text, data, and
bss (uninitialized data) sections are printed along
with the total size of the object file.  If you
use an archive file as input to the command, the
information for all archive members is displayed.

00Enter the names of files for which you want size information.
Separate multiple files by spaces.
01By default, all sizes are printed as decimal numbers.  Enter "y"
here if you want sizes printed as octal numbers. [-o]
02By default, all sizes are printed as decimal numbers.  Enter a "y"
here if you want sizes reported as hexadecimal numbers. [-x]
03If you select this option, the size information printed by
the "size" command will include NOLOAD sections.  By default,
these sections are not included.  [-n]
04Select this option if you want verbose output.   This
option will cause the "size" command to print the
size of every loaded section, along with the name of
the section.  [-f]
05Select this option if you want to know the version number
of the "size" command. [-V]
06ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0102
0201


010341n
020547n
030749n
040941n
051157n

0002
0013
0032if [ ! "`/bin/file $F00| /bin/grep executable`" ];then echo "$F00 IS NOT AN OBJECT FILE";fi
0104yYnN
0105
0205
0204yYnN
0304yYnN
0335
0404yYnN
0435
0504yYnN


001110
010001y-oY-onN
020001y-xY-xnN
031011y-nY-nnN
041011y-fY-fnN
050001y-VY-VnN
061110



