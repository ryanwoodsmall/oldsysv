#ident	"@(#)forms.files:strip.fs	1.4"
00000strip
00701060130NAME OF FILE(S) TO BE STRIPPED:
01703060101LEAVE SYMBOL TABLE INFORMATION INTACT [-l] (y/n):
02705060001LEAVE STATIC AND EXTERNAL SYMBOL INFORMATION INTACT [-x] (y/n):
03007060030LEAVE STATIC AND EXTERNAL SYMBOL INFORMATION AND
04708080001SCOPING INFORMATION INTACT [-b] (y/n):
05010060030LEAVE STATIC AND EXTERNAL SYMBOL INFORMATION AND
06711080001RELOCATION INFORMATION INTACT [-r] (y/n):
07013060030PRINT THE VERSION OF THE STRIP COMMAND
08714090001EXECUTING ON THE STANDARD ERROR OUTPUT [-V] (y/n):

00010
01021
02031
04001
06041
08051


"strip" removes the symbol table and line number
information from common object files, including
archives.  Once this has been done, no symbolic 
debugging access will be available for that file [see 
the ASSIST command form for "sdb"]. Therefore, this 
command is normally run only on production modules 
that have been debugged and tested.

The options control the amount and type of information 
to be stripped.

00Your file(s) must have read and write permission.
In addition, they must have the right "magic number".
This usually means that they have been compiled
on the current machine, but may also have other
meanings.

Note: the "strip" command form does not validate
the magic number.  Thus, the command may not
execute even when the command form allows you to.
When this happens, the command gives you a "cannot 
open" or "bad magic" error message.
01Strip line number information only; do not 
strip any symbol table information [-l].
02Do not strip static or external symbol information [-x].
04Same as the -x option, but also do not strip scoping
information (e.g., beginning and end of block delimiters) [-b].
06Do not strip static or external symbol information,
or relocation information [-r].
08Prints version and date of version [-V].

01020406
02060401
040201
060201


010356n
020570n
040847n
061150n
081460n

003420000001
0032if test -n "`file $F00 | /bin/grep executable`" ;then exit; fi ; echo "NOT AN APPROPRIATE COMMON OBJECT FILE"
0104yYnN
0105
0204yYnN
0205
0404yYnN
0405
0604yYnN
0605
0804yYnN


001110
010001y-lY-lnN
020001y-xY-xnN
031110
040001y-bY-bnN
051110
060001y-rY-rnN
071110
080001y-VY-VnN



