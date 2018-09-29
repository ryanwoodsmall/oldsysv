00000mm
00703060130NAME(S) OF INPUT FILE(S):
01705060001REDIRECT OUTPUT TO:
02707060001SPECIFY TYPE OF OUTPUT TERMINAL [-T]:
03709060001PRODUCE THE DOCUMENT IN 12-pitch [-12] (y/n):
04711060001FILTER REVERSE LINE-FEEDS FOR CERTAIN TERMINAL TYPES [-c] (y/n):
05713060001FORMAT MATHEMATICAL TEXT FOR NROFF [-e] (y/n):
06715060001FORMAT TABLES FOR NROFF [-t] (y/n):
07717060001PRODUCE EQUALLY-SPACED WORDS [-E] (y/n):
08919030000PARAMETERS THAT CAN BE SET FROM THE COMMAND LINE
09721060001SET CONTROL FOR SECTION HEADING UNDERLINING [-rU1] (y/n):
10722060001SET REGISTER FOR THE .AF MACRO [-rA] (0-2):
11723060001SET TYPE OF COPY AND/OR SPACING [-rC] (0-5):
12724060101SET DEBUG MODE [-rD1] (y/n):
13725060001CHANGE FONT OF SUBJECT/DATE/FROM FIELDS TO BOLD [-rE0] (y/n):
14726060001SET LENGTH OF PHYSICAL PAGE [-rL]:
15727060001SET PAGE NUMBERING STYLE [-rN] (0-5):
16728060001SET PAGE OFFSET [-rO]:
17729060001SET STARTING PAGE NUMBER [-rP]:
18730060001SET REGISTER SETTING FOR CERTAIN DEVICES [-rT] (1-2):
19731060001SET PAGE WIDTH [-rW]:
20734060030ADDITIONS TO THE COMMAND LINE:

00200
01210
02030
03040
04050
05060
06070
07080
09090
10100
11110
12120
13130
14140
15150
16160
17170
18180
19190
20222
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

   2631     300       300s-12    4000a      832     lp
   2631-c   300-12    37         450        8510    tn300
   2631-e   300s      382        450-12     X

(cat /usr/pub/terminals for a brief explanation of the terminal types.)
03You may use this option when "$TERM" is set to one of 300,
300s, and 450.  (You must manually set the pitch switch on the 
DASI 300 and 300s terminals to 12 if you use this option.)  [-12]
04This option causes mm to invoke col(1); note col is invoked 
automatically by mm unless terminal type is one of 300, 300s,
450, 34, 4000a, 382, 4014, and X.  
Col performs the line overlays implied by reverse line feeds
and by forward and reverse half-line feeds.  [-c]
05neqn is a preprocessor that translates the control lines in your
file for nroff. Thus, you may use neqn with other preprocessors 
such as tbl, grap, pic, and macro packages without duplication of 
function.  This option causes mm to invoke neqn; also causes neqn 
to read the "/usr/pub/eqnchar" file.  [-e]
06This option causes mm to invoke tbl.  tbl is a preprocessor that prepares
tables for nroff.  tbl assumes that lines between the .TS and .TE 
commands describe tables.  [-t]
07This invokes the -e option of nroff.  The option produces 
equally-spaced words in adjusting lines, using the full 
resolution of the particular terminal. [-E]
09This flag causes only letters and digits to be underlined.
Otherwise, all characters including spaces are underlined.
[-rU1]
10This option provides an alternate first page format. [-rA]
The valid options are:
    0    is the default.
    1    has the effect of invoking the .AF macro without an argument.
    2    allows for usage of the logo, if available, on a printing device.
11This option sets the type of copy to be printed at the bottom 
of each page and sets spacing.  [-rC]

   value
     0  is the default 
     1  for OFFICIAL FILE COPY
     2  for DATE FILE COPY
     3  for DRAFT with single spacing and default paragraph style
     4  for DRAFT with double spacing and 10-space paragraph indent
     5  for double spacing 
12This flag reguest the formatter to continue processing even if 
mm detects errors that would otherwise cause termination.  It 
also includes some debugging information in the default page
header.  [-rD1]
13The roman font is the regular text-default for the nroff formatter.
If this option is selected, the Subject/Date/From fields will be 
printed in bold font.  [-rE0]
14Set length of physical page to the desired number of lines.
If there is no value given, the default page length is 66
lines per page.  [-rL]
15This option allows you to specify the page numbering style 
from the command line.  The default value does not print 
anything on the command line.  [-rN]

   value
     0    (default), all pages get the prevailing header
     1    page header replaces footer on page 1 only
     2    page header is omitted from page 1 
     3    "section-page" numbering occurs
     4    default page header is suppressed; however a user-specified
          header is not affected
     5    "section-page" and "section-figure" numbering occurs
16This argument is in character positions.  The output is offset 
the specified amount to the right.  If this register is not 
set from the command line, the offset is 0.75 inches.   [-rO]
17The document will start with the page number specified.  Automatic 
numbering will be performed if no argument is given.  [-rP]
18This option provides the register setting for certain devices.  [-rT]
   values
     1    line length is 80, page offset is 3.

     2    changes page length to 84 lines per page and inhibits
          underlining; it is meant for output to the Versatec printer.
19This option changes the page width from the default of six inches.
The argument is a number representing character positions.  [-rW]
20ASSIST appends anything you type here to the command line.  You
can use this field for "piping," "redirection," or to execute a 
command in the background.  However, since ASSIST does not perform
any error checking on this field, you should use it with caution.



030952n
041171n
051353n
061542n
071747n
092164n
1022500
1123510
122435n
132568n
14264166
1527440
1729381
19312860

0015-
003420001010
0107
0103
0304yYnN
0404yYnN
0504yYnN
0604yYnN
0704yYnN
0904yYnN
1004012
1104012345
1204yYnN
1304yYnN
1408123456789
14180
1504012345
1608
1708
180412
1908
19240


001110
011010> 
021010-T
031011y-12Y-12nN
041011y-cY-cnN
051011y-eY-enN
061011y-tY-tnN
071011y-EY-EnN
091011y-rU1Y-rU1nN
101010-rA0
111010-rC0
121011y-rD1Y-rD1nN
131011y-rE0Y-rE0nN
141010-rL66
151010-rN0
161010-rO
171010-rP1
181010-rT
191010-rW60
201110



