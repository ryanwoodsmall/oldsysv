#ident	"@(#)forms.files:dircmp.fs	1.3"
00000dircmp
00701060101NAME OF FIRST DIRECTORY:
01703060101NAME OF SECOND DIRECTORY:
02705060001REDIRECT OUTPUT TO:
03707060001WIDTH OF OUTPUT LINE [-w]:
04709060001SHOW CHANGES NEEDED TO BRING FILES INTO AGREEMENT [-d] (y/n):
05711060001PRINT MESSAGES ABOUT IDENTICAL FILES [-s] (y/n):
06713060030ADDITIONS TO THE COMMAND LINE:

00040
01050
02060> 
03030-w72
04011d
05021s
06062


"dircmp" compares two directories and lists all
the files in them stating whether they are the
same or different.  Files are marked as the same
if they have the same name and identical contents.

00Enter the name of the first directory here.
01Enter the name of the second directory here.
02By default, this command will write its output on your terminal.
If you want to save the output in a file instead, enter the name
of the file here.  The current contents of the file will be
destroyed.
03By default, the width of each output line is 72 characters.
Enter a number here to specify a different width. Widths
less than three are not allowed. [-w]
04If you select this option, "dircmp" will compare files with the 
same name in the two directories.  The command will produce a list
telling you what needs to be done to make the two files identical. [-d]
05By default, the command will list files that are common to both
directories, as well as those that are unique.  Enter "n" if you
only want a listing of files that are unique to a directory. [-s]
06ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.



03073372
040968n
051155y

0001
0002
0010
003400001010
0101
0102
0110
013400001010
0201
0203
0213
0308
0332if [ "$F03" -le 2 ];then echo "THE WIDTH MUST BE GREATER THAN TWO";fi
0404yYnN
0504nNyY


001010
011110
021010> 
031010-w72
040001y-dY-dnN
050001n-sN-syY
061110



