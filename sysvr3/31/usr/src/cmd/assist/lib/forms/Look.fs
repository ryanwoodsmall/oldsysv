400LOOK INSIDE FILES 
00001150000LOOK INSIDE FILES AND DIRECTORIES
01703110101............ Scan a big file
02704110101............ Concatenate and show files
03705100101............. Edit files with a line oriented text editor
04706130101.......... Search files for a full regular expression
05707130101.......... Search files for strings
06708120101........... Search files for lines matching a pattern
07709100030............. List contents of directory
08710100101............. Dump an object file in ASCII, binary, hex or octal
09711100101............. Examine files one screen-full at a time
10712100101............. Print files
11713130101.......... Find spelling errors
12714120101........... Show last part of a file
13715100101............. Edit files with a screen oriented text editor
14716210101.. Brief introduction to the "vi" screen editor
15018030000MORE COMMANDS FOR LOOKING INSIDE FILES AND DIRECTORIES
16719120101........... Concatenate and show packed files compacted by "pack"

The Look Inside Files and Directories menu lists
commands that print part or all of a file, that
search files for specific patterns, or that edit
files.  In general, it lists commands used in
different ways to look at files.

01"bfs" is used to scan files.  It is similar to "ed"
except that it is read-only and can process much
larger files.  Files can be up to 1024K bytes
(maximum).  "bfs" is usually more efficient than
"ed" for scanning a file, since the file is not
copied to a buffer.  It is most useful for identi-
fying sections of a large file where "csplit" can
be used to divide it into more manageable pieces.
02"cat" prints the content of files on your terminal.
If you specify multiple files, "cat" prints each
in turn, thus concatenating the multiple files.
03"ed" is a line-oriented text editor that allows you
to create and edit files.  Editing with "ed" is
line-by-line rather than full-screen (as in "vi").
You can use "ed" on both hard-copy and video display
terminals.
04"egrep" searches for a pattern in a file, using
full regular expressions.  "egrep" works like
"fgrep" and "grep".  Of these three, "egrep" is
the slowest but also the most powerful.
05"fgrep" searches for a pattern in a file using
strings.  It searches only for lines that match
the strings exactly, but allows you to specify
many exact patterns to be searched for simultane-
ously.  "fgrep" works like "grep" and "egrep",
but it is the least powerful of the three.
06"grep" searches for a pattern in a file using a
limited regular expression.  "grep" works like
"fgrep" and "egrep", and it is in between "fgrep"
and "egrep" in terms of speed and power.
07
"ls" lists contents of a directory or directories,
and can give detailed information about each file
listed.

The "ls" menu lists two versions of the "ls" command
form; a short "typical" use of "ls", and a complete
version that incorporates all of the options.
08"od" dumps a file in one or more formats according
to the option you pick.  If you do not use an option,
"od" prints in octal words.  If you do not provide a
file name, "od" uses standard input.  If you want to
start reading your file at some location other than
the beginning of the file, the FILE OFFSET OPTIONS
allow you to advance your starting point.
09"pg" allows you to examine a file one screenful at
a time.  Each screenful is followed by a prompt.
If you type a carriage return, another page is
displayed.
10"pr" prints files on your terminal.  By default,
the file is printed separated into pages.  Each
page is headed by a page number, the date and
time, and the name of the file.
11"spell" collects words from specified files and looks
them up in a spelling list.  Words that neither occur
among nor are derivable (by applying certain inflec-
tions, prefixes, and/or suffixes) from words in the
spelling list, are printed on the standard output.
"spell" ignores most macros used with UNIX system
text formatting commands (for example, "eqn," "mm,"
"nroff," "tbl," and "troff").
12"tail" is used to print the end of a file on your
terminal.  Using the options to "tail", you may
specify just how much of the file contents are
printed.  By default, "tail" prints the last 10
lines of a file.
13
"vi" is a full-screen text editor.  When you use
"vi", the changes you make are reflected on the
screen as you make them.  You can use "vi" to create
and edit any type of text file.  Examples of text
files include:  (1) mail messages, (2) documents
that will be formatted by a command such as "mm",
(3) programs in languages such as C, and (4) shell
scripts.  
14
ASSIST's "vi" walkthru is designed to get you started
with a basic set of "vi" editing commands.

"vi" is a full-screen text editor.  When you use
"vi", the changes you make are reflected on the
screen as you make them.  You can use "vi" to create
and edit any type of text file.  Examples of text
files include:  (1) mail messages, (2) documents
that will be formatted by a command such as "mm",
(3) programs in languages such as C, and (4) shell
scripts.  
16"pcat" unpacks and writes specified files to the
standard output.  "pcat" allows you to make an
unpacked copy of a packed file without destroying
the packed file.  "pcat" returns the number of files
it was unable to unpack.  It may fail to unpack files
if the filename has more than 12 characters, if the
file cannot be opened, or if the file does not appear
to be the output of "pack".

010307bfs
020407cat
030507ed
040607egrep
050707fgrep
060807grep
070907ls
081007od
091107pg
101207pr
111307spell
121407tail
131507vi
141607assist viwalk
161907pcat

01bfs
02cat
03ed
04egrep
05fgrep
06grep
07ls
08od
09pg
10pr
11spell
12tail
13vi
14viwalk
16

