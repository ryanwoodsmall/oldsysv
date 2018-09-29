#ident	"@(#)forms.files:Format.fs	1.2"
400FORMAT TEXT
00001150000FORMAT TEXT AND HARDCOPY OUTPUT
01703110101...... Concatenate and show files
02704100101....... Send files to a line printer
03705140101... Report the status of the line printers
04706100101....... Format text with MM macros using the "nroff" text processor
05707130101.... Format text containing "nroff" commands
06708100101....... Print files on standard output
07010080000MORE COMMANDS FOR FORMATTING TEXT AND HARDCOPY OUTPUT
08712140101... Show strings in large letters
09713150101.. Check MM macros for correctness
10714110101...... Filter reverse line-feeds
11715150101.. Change the format of text files
12716100101....... Number lines of a file

The Format Text and Hardcopy Output menu lists
commands that you use to create documents and
print them.

01"cat" prints the content of files on your terminal.
If you specify multiple files, "cat" prints each
in turn, thus concatenating the multiple files.
02"lp" sends files to a line printer.  If you do not
include any file names, "lp" will take input from
standard input.
03"lpstat" prints information about the current status
"lpstat" prints information about the current status
and configuration of the line printers on your system.
You may request multiple pieces of information at one
time.
04"mm" invokes the "nroff" text formatter.  The "nroff"
text formatter is a program that formats text by
interpreting special requests and macros that you
place in a file.  You can use the "mm" command to
format a variety of documents such as letters and
reports.  To use "mm" you must have first created
a file that contains MM macros.
05"nroff" is a text formatter that manipulates text
by interpreting special requests and macros that
you place in a file.  You can use "nroff" to format
a variety of documents such as letters, reports,
and books.  To use the "nroff" command, you must 
create a text file that contains "nroff" requests.
06"pr" prints files on your terminal.  By default,
the file is printed separated into pages.  Each
page is headed by a page number, the date and
time, and the name of the file.
08"banner" prints strings of up to 10 characters in
large letters.  For example, "banner hello" produces
the following output:



      #    #  ######  #       #        ####
      #    #  #       #       #       #    #
      ######  #####   #       #       #    #
      #    #  #       #       #       #    #
      #    #  #       #       #       #    #
      #    #  ######  ######  ######   ####
09"checkmm" examines files that contain Memorandum
Macros (MM) to identify potential MM usage errors.
For example, if your file contained a .DS to make
a display but did not contain a .DE, "checkmm"
would report that a .DE was missing.
10"col" interprets ASCII codes for reverse line feeds,
and forward or reverse half-line feeds.  Using
various options, output can be tailored to the
line-feed capabilities of output devices.  For
example, use the "-b" option if your device cannot
backspace.
11"newform" prints the contents of files on your
terminal reformatted according to your specifi-
cations.
12"nl" reads lines from a file or from standard input.
It reproduces these lines on the standard output with
line numbers appearing on the left.  "nl" views the
text it reads in terms of logical pages.  A logical
page consists of a header, a body, and a footer
section.  You may choose different line numbering
options for the different page sections.  "nl" resets
the line numbering at the start of each logical page.

010307cat
020407lp
030507lpstat
040607mm
050707nroff
060807pr
081207banner
091307checkmm
101407col
111507newform
121607nl

01cat
02lp
03lpstat
04mm
05nroff
06pr
08
09
10
11
12

