#ident	"@(#)forms.files:sttyset.fs	1.2"
00000stty
00901020030LOCAL MODES
01702040001SET "erase" TO A SPECIFIED CHARACTER [erase]:
02703040001SET "kill"  TO A SPECIFIED CHARACTER [kill]:
03905040030SET "stty"  TO ERASE A CHARACTER FROM THE SCREEN WHEN
04706190001THE "erase" CHARACTER  IS ENTERED [echoe] (y/n):
05707040001SET "stty"  TO DISABLE "echoe" [-echoe] (y/n):
06908040030SET "stty"  TO ECHO A NL AFTER
07709190001A KILL CHARACTER IS ENTERED [echok] (y/n):
08710040001SET "stty"  TO DISABLE "echok" [-echok] (y/n):
09912020030COMBINATION MODES
10713040001SET "stty"  TO PRESERVE TABS WHEN PRINTING [tabs] (y/n):
11714040001SET "stty"  TO DISABLE "tabs" [-tabs] (y/n):
12715040001RESET ALL MODES TO SOME REASONABLE VALUES [sane] (y/n):
13917020030OUTPUT MODES
14718040001ENTER STYLE OF DELAY FOR: CARRIAGE RETURNS [cr]:
15719300001LINE FEEDS [nl]:
16720300001HORIZONTAL TABS [tab]:
17721300001VERTICAL TABS [vt]:
18722300001FORM FEEDS [ff]:
19724040030ADDITIONS TO THE COMMAND LINE:

01042
02042
04060
05060
07060
08060
10150
11160
12170
14100
15110
16120
17130
18140
19182


This is the form you will use in ASSIST to set your "stty" I/O options.
"stty" sets certain terminal I/O options for the device that is
the current standard input;  The ASSIST command form for "stty"
does not contain every option available, but contains the most
frequently used options.  Most .profiles contain "stty" commands
that will initialize your terminal settings when you log onto
the computer so many of the stty arguments will already be set.
Caution should be used when running  "stty" because the command 
does no sanity checking.  Some sanity checking is provided by
ASSIST.

01By default, the erase character when you login in to a UNIX
system is the # sign.  Use this field to change your erase
character.  People often change their erase character to the
^H (control-h) character because ^H is the backspace character. 
You can enter other control characters here if you wish
instead of ^H.  Be sure and put the caret (^) sign before
the character if you are entering a control character so you
will type the two characters ^H or you can enter a control
character in this field by typing the ASSIST control character,
^C, and then enter the actual control character, ^H.  [erase]
02Use this field to change your kill character.  The default value for the
kill character is a "@".  The "kill" option assigns to the character
that you enter the capability of killing a line of text.  For example,
if you type:

          mm -Tlp -t -c myfile@

then the "@" will kill your line (erase your entire line).  The "@" sign
is a good kill character because it is not used often for other purposes.
You can also enter control characters here by typing the caret (^) sign
before the character (ie. ^N), or you can enter a control character in
this field by typing the ASSIST control character, ^C, and then entering
the actual control character (ie. <CTRL>C and then <CTRL>N).   [kill]
04This field causes an echo of the ERASE character as a 
backspace-space-backspace string.   Note:  this mode 
will erase the ERASEed character on many CRT terminals;
however,  it does not keep track of column position and,
as a result, may be confusing on escaped characters, tabs,
and backspaces.   [echoe]
05You do not have to disable "echoe" unless it has
previously been enabled.  Choosing this field will
cause the terminal not to echo the ERASE character
as a backspace-space-backspace string.   [-echoe]
07Invoking this field causes the terminal to echo NL 
after KILL character.   [echok]
08You do not have to disable "echok" unless it has
previously been enabled.  Choosing this field will
cause the terminal not to echo NL after the KILL
character.  [-echok]
10Invoking this field will cause the terminal to preserve
(expand to spaces) tabs when printing.   [tabs]
11You do not have to disable "tabs" unless it has
previously been enabled.  Choosing this field will
cause the terminal not to preserve tabs.  [-tabs]
12Invoking this field, resets all modes to some
reasonable value.  The "sane" field is
incompatible with other fields since it is
resetting values and all the other fields are
setting values.   [sane]
14Select style of delay for carriage returns.  The delay field
specifies how long transmission stops to allow for mechanical
or other movement when the carriage return is sent to the
terminal.  Entering a value of 0 indicates no delay.  Entering
a value of 1 is dependent on the current column position.  
Entering a 2 causes a delay of about 0.10 seconds,  3 is 0.15
seconds, 4 is 0.20 seconds, and 5 is 0.25 seconds.  You must
enter 0, 1, 2, or 3.  [cr]
15Select style of delay for line feeds.  The delay field
specifies how long transmission stops to allow for mechanical
or other movement when the line feed is sent to the
terminal.  Entering a value of 0 indicates no delay.  Entering
a value of 1 causes a delay of about 0.10 seconds.
Select a 0 or 1 for this field.   [nl] 
16Select style of delay for horizontal tabs.  The delay field
specifies how long transmission stops to allow for mechanical
or other movement when the horizontal tab is sent to the
terminal.  Entering a value of 0 indicates no delay.  Entering
a value of 1 is dependent on the current column position.  
Entering a 2 causes a delay of about 0.10 seconds.  Entering
a 3 specifies that tabs are to be expanded into spaces.
Select type of delay for horizontal tabs.  0, 1, 2, or 3 are
the only acceptable entries for this field.   [tab]
17Select style of delay for vertical tabs.  The delay field
specifies how long transmission stops to allow for mechanical
or other movement when the vertical tab feed is sent to the
terminal.  Entering a value of 0 indicates no delay. Entering
a value of 1 causes a delay of about 2  seconds.
0 and 1 are the only acceptable entries for this field.   [vt]
18Select style of delay for form feeds.  The delay field
specifies how long transmission stops to allow for mechanical
or other movement when the form feed is sent to the terminal.  
Entering a value of 0 indicates no delay. Entering a value of 1 
causes a delay of about 2  seconds.  0 and 1 are the only 
acceptable entries for this field.   [ff]
19ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

0112
0212
040512
050412
070812
080712
101112
111012
1211101401040507081516171802
1412
1512
1612
1712
1812


040668n
050751n
070962n
081051n
101361n
111449n
121560n

0105
01012
0108^#	
0118\\
0205
02012
023410101010
0208^@	
0218/\
0404yYnN
0405
0504yYnN
0505
0704yYnN
0705
0804yYnN
0805
1004yYnN
1005
1104yYnN
1105
1204yYnN
1205
14040123
1405
150401
1505
16040123
1605
170401
1705
180401
1805


001110
011010erase ''erase  
021010kill ''kill  
031110
041011yechoeYechoenN
051011y-echoeY-echoenN
061110
071011yechokYechoknN
081011y-echokY-echoknN
091110
101011ytabsYtabsnN
111011y-tabsY-tabsnN
121011ysaneYsanenN
131110
141010cr
151010nl
161010tab
171010vt
181010ff
191110



