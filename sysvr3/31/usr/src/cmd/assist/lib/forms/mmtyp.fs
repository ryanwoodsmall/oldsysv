00000mm
00703060130NAME(S) OF INPUT FILE(S):
01705060001REDIRECT OUTPUT TO:
02707060001SPECIFY TYPE OF OUTPUT TERMINAL [-T]:
03709060001FORMAT MATHEMATICAL TEXT FOR NROFF [-e] (y/n):
04711060001FORMAT TABLES FOR NROFF [-t] (y/n):
05713060030ADDITIONS TO THE COMMAND LINE:

00080
01100
02020
03050
04060
05112


mm is used to format documents using nroff and the mm text-formatting
macro package.  mm has option to specify preprocessing by tbl and/or
neqn and postprocessing by various terminal oriented output filters.

00Enter the name(s) of file(s) whose contents you want formatted.
If you enter multiple files, the command will format each in the
order specified.  mm reads standard input when you use a hyphen (-)
instead of any file names.
01If you type a file name here, the output of this command will go to the
file.  ASSIST will warn you if the file already exists.  If it exists,
redirecting output to it will destroy the previous contents of the file.
(See the UNIX walkthru for a more complete discussion of redirection.)
02The argument for -T is the desired terminal type.  If no -T option
is given then "$TERM" is used if it has a value otherwise the default
is lp.  ASSIST does not perform any error checking on this field,
you should use it with caution.  Any other argument that you provide
the option -T does not produce an error; it is equivalent to -Tlp.
These are the recognized terminal types:

    2631      300       300s-12    4000a     832     lp
    2631-c    300-12    37         450       8510    tn300
    2631-e    300s      382        450-12    X

(cat /usr/pub/terminals for a brief explanation of the terminal types.)
03neqn is a preprocessor that translates the control lines in your
file for nroff.  Thus, you may use neqn with other preprocessors
such as tbl, grap, pic, and macro packages without duplication of
function.  This option causes mm to invoke neqn; also causes neqn
to read the "/usr/pub/eqnchar" file.  [-e]
04This option causes mm to invoke tbl.  tbl is a preprocessor that prepares
tables for nroff.  tbl assumes that lines between the .TS and .TE 
commands describe tables.  [-t]
05ASSIST appends anything you type here to the command line.  You
can use this field for "piping," "redirection," or to execute a 
command in the background.  However, since ASSIST does not perform
any error checking on this field, you should use it with caution.



030953n
041142n

0015-
003420001010
0107
0103
0304yYnN
0404yYnN


001110
011010> 
021010-T
031011y-eY-enN
041011y-tY-tnN
051110



