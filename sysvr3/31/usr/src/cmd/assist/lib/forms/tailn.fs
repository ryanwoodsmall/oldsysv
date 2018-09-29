#ident	"@(#)forms.files:tailn.fs	1.5"
00000tail
00701060001FILE NAME:
01903020030COUNTING FROM THE END OF THE FILE
02704060001NUMBER OF LINES FROM END [-l]:
03705060001NUMBER OF CHARACTERS FROM END [-c]:
04706060001NUMBER OF BLOCKS FROM END [-b]:
05908020030COUNTING FROM THE BEGINNING OF THE FILE
06709060001NUMBER OF LINES FROM BEGINNING [+l]:
07710060001NUMBER OF CHARACTERS FROM BEGINNING [+c]:
08711060001NUMBER OF BLOCKS FROM BEGINNING [+b]:
09713060030ADDITIONS TO THE COMMAND LINE:

00050
02000
03000
04000
06000
07000
08000
09062

Since you did not enter an input file name, "tail" will expect
input from your keyboard.  Use ^D to indicate to the "tail"
command when you have completed input.00

The "tail" command is used to print out selected portions of a
file.  When used without specifying options, the last ten lines
of the file will be displayed.  You may indicated that fewer or
more lines be printed.  You may also request that printing start
at a particular character or block location in the file.  

00Enter here the name of the file whose contents you
wish to display.  If you leave this field blank "tail" 
will expect input from your terminal.  In this case, 
you must use ^D to terminate input to "tail."
02Enter here the number of lines from the end of the
file where "tail" should begin printing.  "tail" will
print from there to the end of the file.  [-l]
03Enter here the number of characters from the end
of the file where "tail" should begin printing.
The default value is 10.  [-c]
04Enter the number of blocks from the end of the
file where "tail" should begin printing.  It
will print from there to the end of the file.  
Blocks consist of 512 characters.  [-b]
06Enter the number of lines from the beginning of the
file where "tail" should start printing.  "tail" will
print from here to the end of the file.  [+l]
07Enter the number of characters from the beginning of
the file where "tail" should start printing.  "tail" will
print from here to the end of the file.  [+c]
08Enter here the number of blocks from the beginning
of the file where "tail" should start printing.  It
will print from here to the end of the file.  
Blocks consist of 512 characters.  [+b]
09ASSIST appends anything you type here to the command line.
You can use this field for "piping," "redirection," or to
execute a command in the "background."  However, since
ASSIST does not perform any error checking on this field,
you should use it with caution.

020304060708
030204070806
040203060708
060203040708
070203040608
080203040607



0002
003420001010
0208
0205
0232if [ $F02 -eq 0 ];then echo "MUST BE GREATER THAN ZERO";fi
0305
0308
0332if [ $F03 -eq 0 ];then echo "MUST BE GREATER THAN ZERO";fi
0408
0405
0432if [ $F04 -eq 0 ];then echo "MUST BE GREATER THAN ZERO";fi
0608
0605
0632if [ $F06 -eq 0 ];then echo "MUST BE GREATER THAN ZERO";fi
0708
0705
0732if [ $F07 -eq 0 ];then echo "MUST BE GREATER THAN ZERO";fi
0805
0808
0832if [ $F08 -eq 0 ];then echo "MUST BE GREATER THAN ZERO";fi


001110
011110
021110-l
031010-c
041010-b
051110
061010+l
071010+c
081010+b
091110



