00000ls
00701060030NAME OF INPUT FILE(S)/DIRECTORY(S):
01703060001LIST NAMES BEGINNING WITH "." [-a] (y/n):
02705060001LIST DIRECTORY NAME ONLY, NOT ITS CONTENTS [-d] (y/n):
03707060001LIST IN LONG FORMAT [-l] (y/n):
04709060001MULTI-COLUMN WITH ENTRIES SORTED VERTICALLY [-C] (y/n):
05711060001MULTI-COLUMN WITH ENTRIES SORTED HORIZONTALLY [-x] (y/n):
06713060001APPEND "/" FOR DIRECTORIES, "*" FOR EXECUTABLE FILES [-F] (y/n):
07715060001APPEND "/" FOR DIRECTORIES [-p] (y/n):
08717060030ADDITIONS TO THE COMMAND LINE:

00030
01001a
02001d
03001l
04001C
05001x
06001F
07001p
08032-


ls lists contents of a directory or directories, and can give detailed
information about each listed file.  This is the command form for the
typical usage of 'ls.'  There is also a complete command form which
provides all of the 'ls' options.  To save the output in a file,
use the COMMAND LINE ADDITIONS field to redirect the output to a file.

00List any number of file or directory names here.
If you leave this blank, the current directory is listed.
01This option lists "." files.  For example, ".profile" is not
normally listed.  If you use the -a option, ".profile" will be
listed.  This option is overridden by the -d option.  You 
should not use them together. [-a]
02If an argument is a directory, this option lists
only its name, not its contents.  It is often
used with -l to get the status of a directory. 
This option overrides the -a option.  You should
not use -a if you are using this option. [-d]
03This option lists for each file:  access permissions, number of links,
owner, group, size in bytes, and time of last modification.  It is
incompatible with the -c and -x options and should not be used with
them.  [-l]
04This option lists output in columns regardless of whether
sorting was alphabetic or chronological.  It is incompatible
with the -l and -x options and should not be used with them.  [-C]
05This option lists output in rows regardless of whether
sorting was alphabetic or chronological.  It is incompatible
with the -C and -l options and should not be used with them.  [-x]
06This option provides a convenient way to obtain some
basic file information, such as which items are
directories or which files are executable, without
using the long format.  This option is incompatible
with the -p option and should not be used with it. [-F]
07This option provides a convenient way to find directories
without using the long format.  It is incompatible with
the -F option and should not be used with it. [-p]
08ASSIST appends anything you type here to the command line.  
You can use this field for "piping," "redirection," or to execute 
a command in the "background."  However, since ASSIST does not perform 
any error checking on this field, you should  use it with caution.

0102
0201
030405
040305
050403
0607
0706


010348n
020561n
030738n
040962n
051164n
061371n
071545n

0014
0104yYnN
0105
0204yYnN
0205
0304yYnN
0305
0404yYnN
0405
0504yYnN
0505
0604yYnN
0605
0704yYnN
0705


001110
010001y-aY-anN
020001y-dY-dnN
030001y-lY-lnN
040001y-CY-CnN
050001y-xY-xnN
060001y-FY-FnN
070001y-pY-pnN
081110



