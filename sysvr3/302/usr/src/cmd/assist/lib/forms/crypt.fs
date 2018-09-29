#ident	"@(#)forms.files:crypt.fs	1.3"
00000crypt
00701060001ENCRYPTION/DECRYPTION PASSWORD:
01703060001REDIRECT INPUT FROM:
02705060001REDIRECT OUTPUT TO:
03707060030ADDITIONS TO THE COMMAND LINE:

00010
01020< 
02030> 
03042

Since you did not redirect input from a file, the "crypt"
command will expect you to enter input from standard input.
Type a ^D to terminate input, and return you to ASSIST. 01

"crypt" converts readable text into encrypted text
or unencrypts text that you encrypted using "crypt".
By default, input comes from standard input, and
output goes to standard output.

00If you do not specify a value here, you will be prompted for one.
If you are concerned about security, you may wish to be prompted,
as the password will be visible on this command form, while attempts
will be made to suppress it if you elect to be prompted.  In either 
case, you will need this password to convert your text back later.
01By default, "crypt" expects input from standard input.  
In most cases, you will want to encrypt or decrypt a file,
so will enter a file name here.
02By default, "crypt" places the output on standard output.  
Most of the time, you will want to save the output in a file.
If so, enter the file name here.  Existing contents of the file
(if any) will be destroyed.
03ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.




0101
0102
0113
013420001010
0207
0203
0201
0213


001110
011010< 
021010> 
031110



