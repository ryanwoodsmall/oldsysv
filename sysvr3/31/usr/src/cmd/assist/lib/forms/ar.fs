#ident	"@(#)forms.files:ar.fs	1.1"
400ar menu
00701150101 ....... Delete file(s) from an archive file
01703150101 ....... Move file(s) within an archive file
02705150101 ....... Print the named file(s) in the archive file
03707150101 ....... Quickly append named file(s) to end of archive file
04709150101 ....... Replace the named file(s) in existing archive
05910240030Create new archive file containing named file(s)
06712150101 ....... Print a table of contents of the archive file
07714150101 ....... Extract the named file(s) from the archive file

"ar" maintains groups of files combined into a single
archive file.  "ar" is primarily used to create and
update archive files of object code for library files;
for example, files such as /lib/libc.a and /lib/libm.a
were created with "ar".  You can also use "ar" to
combine a set of text files into a single archive file.

00"ar" with the "-d" option removes files from an
archive file.
01"ar" with the "-m" option moves files to different
positions within the archive file.
02"ar" with the "-p" option prints one or more files
that are stored in an archive file.
03"ar" with the "-q" option quickly appends a new file
to the end of an archive file.  "ar" with the "-q"
option does not check whether the added file is
currently in the archive file.  
04"ar" with the "-r" option adds new files to an
archive file and replaces one or more files in
an archive file.  "ar" with the "-r" option will
create the named archive file if it does not exist.  
06"ar" with the "-t" option prints a table of
contents of the archive file.
07"ar" with the "-x" option extracts one or more files
that are stored in an archive file. 'ar" makes a copy,
in the current directory, of each extracted file.  The
extracted file also remains in the archive file.

000109ar -d
010309ar -m
020509ar -p
030709ar -q
040909ar -r
061209ar -t
071409ar -x

00ar_d
01ar_m
02ar_p
03ar_q
04ar_r
06ar_t
07ar_x

