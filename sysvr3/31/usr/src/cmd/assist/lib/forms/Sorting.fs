400SORT
00001170000SORT, TRANSFORM, AND ARCHIVE FILES
01703090101.......... Create and maintain groups of files in one archive file
02704100101......... Pattern scanning and processing language
03705110101........ Copy file archives in and out of a file
04706120101....... Encode or decode a file
05707100101......... Stream editor
06708110101........ Sort and/or merge the lines of files
07010110000MORE COMMANDS FOR SORTING, TRANSFORMING, AND ARCHIVING FILES
08712140101..... Change the format of a text file
09713110101........ Store files in a compressed form
10714090101.......... Translate characters in a file
11715110101........ Report lines in a sorted file that occur more than once
12716130101...... Expand files created by "pack"

The Sort, Transform, and Archiv Files menu lists
commands that change the form and content of your
files, and archive many of your files into one file.

01"ar" maintains groups of files combined into a single
archive file.  "ar" is primarily used to create and
update archive files of object code for library files;
for example, files such as /lib/libc.a and /lib/libm.a
were created with "ar".  You can also use "ar" to
combine a set of text files into a single archive file.
02"awk" is a programmable UNIX system filter that
does pattern scanning and language processing.  It
includes text substitution, arithmetic, variables,
built-in functions, and a programming language that
looks similar to C. 
03"cpio" has three distinct uses.  First, "cpio -o"
can take a list of file names, and display their
content on standard output.  By redirecting output,
you can create an archive and write it to external
storage on to a single file.  Second, "cpio -i" can
take an archive file coming from standard input,
and unarchive it into its constituent files.  This
form is often used to read in a tape made with an
earlier "cpio" command.  Finally, "cpio -p" can copy
a set of files and directories from one location to
another.
04"crypt" converts readable text into encrypted text
or unencrypts text that you encrypted using "crypt".
By default, input comes from standard input, and
output goes to standard output.
05"sed" reads lines from a file, changes the lines
according to a script you supply, and puts out the
modified lines on standard output.  The actual
files are not changed.
06"sort" enables you to sort the lines of files, merge
files, or check if files are sorted properly.  "sort"
can sort a file according to fields.  "sort" can sort
the lines of files in many different orders including
reverse alphabetical.

The "sort" menu lists two versions of the "sort"
command form; a short typical use of "sort", and a
complete version that incorporates all the options.
08"newform" prints the contents of files on your
terminal reformatted according to your specifi-
cations.
09"pack" attempts to store the files you specify in
a compressed form to save storage space on your
computer.  Wherever possible, each file is replaced
by a packed file, "filename.z".  If "pack" is
successful, the original input file is removed.
The "-f" option forces packing of the file.  You
may restore packed files to their original form by
using "unpack".  You can view a packed file by using
"pcat".  You usually will not save storage space by
using files smaller than three blocks.
10"tr" copies the standard input to the standard
output with substitution or deletion of selected
characters.  Input characters found in the first
string are mapped into the corresponding characters
of the second string.
11"uniq" removes second and succeeding copies of
repeated lines in your file.
12"unpack" expands files that were previously compressed
by "pack".  The packed form of the file does not exist
after you run "unpack" on it.  If "unpack" is unable
to unpack any files, it tells you how many.  "unpack"
may fail for the following reasons:  the file name has
more than 12 characters, the file cannot be opened,
the file does not appear to be the output of "pack",
a file with the unpacked name already exists, or the
unpacked file cannot be created.

010306ar
020406awk
030506cpio
040606crypt
050706sed
060806sort
081206newform
091306pack
101406tr
111506uniq
121606unpack

01ar
02awk
03cpio
04crypt
05sed
06sort
08
09
10
11
12

