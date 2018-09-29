00000bfs
00703060101FILE NAME:
01705060001SUPPRESS THE PRINTING OF SIZES [-] (y/n):
02707060030ADDITIONS TO THE COMMAND LINE:

00020
01010
02032

"bfs" is an interactive command.

Type "q" to exit the big file scanner.

"bfs" is used to scan files.  It is similar to "ed"
except that it is read-only and can process much
larger files.  Files can be up to 1024K bytes
(maximum).  "bfs" is usually more efficient than
"ed" for scanning a file, since the file is not
copied to a buffer.  It is most useful for identi-
fying sections of a large file where "csplit" can
be used to divide it into more manageable pieces.

00Enter the name of the file you want to scan here.
01If you specify a "y" for this field, it will suppress typical
messages about size reported by the big file scanner.  [-]
02
ASSIST appends anything you type here to the command line.  
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since 
ASSIST does not perform any error checking on this field,
you should use it with caution.



010548n

0002
003420001010
0104yYnN


001010
011011y-Y-nN
021110



