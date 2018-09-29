400MAINTENANCE 
00001250000MAINTENANCE AND PERFORMANCE
01703110101...... Create and maintain groups of files in one archive file
02704110101...... List contents of directory with options for file data
03705130101.... Maintain, update, and regenerate groups of programs
04706130101.... Print section sizes of common object files
05707140101... Strip symbols and line numbers from object files
06009170000MORE COMMANDS FOR MAINTENANCE AND PERFORMANCE
07711140030... Create and administer SCCS files
08712140030... Maintain, update, regenerate programs unique to individuals
09713140030... Make a change to an SCCS file
10714120030..... Get a version of an SCCS file
11715130101.... Display profile data
12716120030..... Print an SCCS file
13717130101.... Time a command
14718130030.... Identify SCCS files

The Maintenance and Performance menu lists commands
that will help you maintain the files you use.  It
provides commands that make archives, report sizes
of files, and strip unnecessary characters out of
files.

01"ar" maintains groups of files combined into a single
archive file.  "ar" is primarily used to create and
update archive files of object code for library files;
for example, files such as /lib/libc.a and /lib/libm.a
were created with "ar".  You can also use "ar" to
combine a set of text files into a single archive file.
02
"ls" lists contents of a directory or directories,
and can give detailed information about each file
listed.

The "ls" menu lists two versions of the "ls" command
form; a short "typical" use of "ls", and a complete
version that incorporates all of the options.
03"make" maintains, updates, and regenerates groups
of programs.  "make" reads a file that describes
how the components of the program depend on each
other and how to process these components to create
an up-to-date version of the program.
04"size" prints size information for each section of
an object file.  The size of the text, data, and
bss (uninitialized data) sections are printed along
with the total size of the object file.  If you
use an archive file as input to the command, the
information for all archive members is displayed.
05"strip" removes the symbol table and line number
information from common object files, including
archives.  Once this has been done, no symbolic
debugging access will be available for that file.
Therefore, "strip" is normally run only on
production modules that have been debugged and
tested.
07"admin" creates new SCCS files and changes parameters of existing ones.
Arguments to "admin" consist of keyletter arguments, which begin with
"-", and file names.  SCCS files must begin with the characters "s.)".
If a file does not exist, it is created, and its parameters are
initialized according to the specified keyletter arguments.  Parameters
not initialized are assigned defaults.  If a file exists, parameters
corresponding to keyletter arguments are changed and other parameters
are left as is.  If you specify a directory, "admin" treats each file
as an input file, except that non-SCCS files and unreadable files are
silently ignored.  If you specify "-" as the input, "admin" reads
the standard input.
08"build" maintains, updates, and regenerates groups of programs
that are unique to an individual or group.  It reads a
file that describes how the components of the program depend
on each other and how to process these components to create  
an up-to-date version of the program.  "build" searches a
private node and a project node for objects needed to generate
the target.  The resulting private version of the target is
generated in a directory of the private node.  If you do not
specify a list of directories to be searched for source files,
"build" behaves identically to "make."  The "build" command is
available with UNIX System 5.3. 
09"delta" makes permanent changes in the SCCS file retrieved by "get."
It makes a change to each SCCS file that you name.  If you specify
a directory, "delta" behaves as though each file were specified as
an input file.  Non-SCCS files and unreadable files are silently
ignored.  If you specify "-" as input, "delta" reads the standard
input.
10"get" generates an ASCII text file for each SCCS file you name,
according to the arguments you specify.  All arguments specified 
apply to all the files you name.  If you name a directory, "get"
treats each file in the directory as if it were an input file.
Non-SCCS files and unreadable files are silently ignored.  If
you specify "-" as input, "get" reads the standard input.  "get"
puts the generated text into a "g-file" whose name is derived
from the SCCS file name by removing the leading "s.".
11"prof" reports statistics on functions in your C
programs.  These statistics include:
  - the percentage of time spent in the function
  - the number of calls to the function
  - the average time spent per call to the function.
In order to use "prof", your C program must have been
compiled with the "cc -p".
12"prs" prints parts or all of an SCCS file in a user-supplied
format.  If you name a directory, "prs" treats each file in
the directory as if it were an input file.  Non-SCCS files and
unreadable files are silently ignored.  If you specify "-"
as input, "prs" reads the standard input.
13"time" measures the amount of time a command takes
to execute.  After you execute a command, "time"
prints the elapsed time during the command, the
time spent in the system, and the time spent in
execution of the command.  Times are reported in
seconds and are printed on standard error.
14"what" searches the given files for all occurrences of the
pattern that "get" substitutes for @(#) and prints out what
follows until the first ~, >, new-line, \, or null character.
"what" should be used in conjuction with "get."

010308ar
020408ls
030508make
040608size
050708strip
071108admin
081208build
091308delta
101408get
111508prof
121608prs
131708time
141808what

01ar
02ls
03make
04size
05strip
07
08
09
10
11
12
13
14

