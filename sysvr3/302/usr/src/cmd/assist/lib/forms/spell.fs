00000spell
00701060030NAME OF INPUT FILE(S): 
01903060030NAME OF LOCAL FILE CONTAINING A SORTED LIST
02704080001OF ADDITIONAL CORRECT SPELLINGS FOR INPUT FILES: 
03706060001PRINT ALL WORDS NOT LITERALLY IN THE SPELLING LIST [-v] (y/n): 
04708060001CHECK BRITISH SPELLING [-b] (y/n): 
05710060001PRINT EVERY PLAUSIBLE STEM [-x] (y/n): 
06712060001FOLLOW CHAINS OF ALL INCLUDED FILES [-l] (y/n): 
07714060030ADDITIONS TO THE COMMAND LINE: 

00070
02060
03010
04030
05040
06050
07082

If you type a hyphen (-) instead of an input file name,
"spell" puts you into an interactive mode.  You must
type control-d (^D) to terminate this mode.00-

"spell" collects words from the input file(s) and looks them
up in a spelling list.  Words that are not on the spelling list
or are not derivable form words on the list, are printed on
the standard output.  If you do not specify an input file,
words are collected from the standard input.  "spell" ignores
most macros used with troff, tbl, and eqn.

00Enter the name(s) of file(s) you want to check for spelling
errors.  If you leave this field blank, or place a hyphen
in this field, "spell" looks for input from the standard
input.  In this case, you must type control-d (^D) to terminate
the input.
02This field specifies the name of of a user-provided
file that contains a sorted list of words, one per line.
If you have words that are specific to you file, you
may specify the name of that file in this field.  "spell"
will check your file in addition to its own dictionary and
will not flag words as incorrect, even though they may be
specific to your document.
03This option prints all words not literally in the spelling
list.  It also indicates plausible derivations from the
words in the spelling list. [-v]
04This option checks British spelling.  It looks for
spellings of the following kinds: centre, colour,
programme, travelled, etc.  These are the preferred
spellings and words such as the following
will be flagged:  center, color, program, traveled.
This option insists on -ise in words like
standardise. [-b]
05This option prints every plausible stem.
A stem is the part of the word that does
not change when prefixes or suffixes
are added or when verb tenses, number,
etc. are changed.
Each word is preceded by "=." [-x]
06By default, "spell" follows chains of included files,
unless the included files begin with /usr/lib.  This
option follows the chains of all included files. [-l]
07ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



030670n
040842n
051046n
061255n

0015-
003420001010
0202
023420001010
0304yYnN
0404yYnN
0504yYnN
0604yYnN


001110
011110
021010+
031011y-vY-vnN
041011y-bY-bnN
051011y-xY-xnN
061011y-lY-lnN
071110



