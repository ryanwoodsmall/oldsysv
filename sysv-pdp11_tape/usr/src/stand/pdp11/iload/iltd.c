/*	@(#)iltd.c	1.1	*/
int	buf[2560];
char	nunix[14];
char	ans[256];
int	*boot;
int	rp03boot(), rp04boot(), rm05boot(), rl11boot();
int	(*read)(), (*write)();
int	tmread(), htread(), rpwrite(), hpwrite(), rlwrite(), hmwrite();
main()
{
	register count;
	int	tapa, disa,c;
	int	diskunit, tapeunit;

	nunix[0] = 0;
	strcat(&nunix,"unix");
	printf("UNIX -- Initial Load: Tape-to-Disk\n");
	printf("\nThe type of disk drive on which the Root file system will reside,\n");
	printf("as well as the type of tape drive that will be used for Tape 1\n");
	printf("must be specified below.\n");
	printf("\nAnswer the questions with a 'y' or 'n' followed by\n");
	printf("a carriage return or line feed.\n");
	printf("There is no type-ahead -- wait for the question to complete.\n");
	printf("The character '@' will kill the entire line,\n");
	printf("while the character '#' will erase the last character typed.\n\n");
	if (yes("RP03 at address 176710")) {
		write = &rpwrite;
		boot = &rp03boot;
		strcat(&nunix,"rp");
	} else
	if (yes("RP04/5/6 at address 176700")) {
		write = &hpwrite;
		boot = &rp04boot;
		strcat(&nunix, "gd");
	} else
	if (yes("RM05 at address 176700")) {
		write = &hmwrite;
		boot = &rm05boot;
		strcat(&nunix, "gd");
	} else
/*
	if (yes("RL01 at address 174400")) {
		write = &rlwrite;
		boot = &rl11boot;
		strcat(&nunix,"rl");
	} else
*/
	{
		printf("This release only supports the above disk drives.\n");
		printf("Restart and answer correctly.\n\n");
		exit(1);
	}
	diskunit = number(write == &rlwrite ? "Drive number (0-3)" : "Drive number (0-7)");
	if (diskunit>(write == &rlwrite ? 3 : 7)) {
		printf("Out of range; 0 assumed\n");
		diskunit = 0;
	}
	printf("Disk drive %d selected.\n",diskunit);
	printf("\nMount a formatted pack on drive %d.\n",diskunit);
	while (!yes("Ready"))
		printf("So what's the matter?\n");
	putchar('\n');
	if (yes("TU10/TM11 at address 172520")) {
		read = &tmread;
		strcat(&nunix, "tm");
	} else
	if (yes("TU16 at address 172440")) {
		read = &htread;
		strcat(&nunix, "ht");
	} else
	{
		printf("This release only supports the above tape drives.\n");
		printf("Restart and answer correctly.\n\n");
		exit(1);
	}
	tapeunit = number("Drive number (0-7)");
	if (tapeunit>7) {
		printf("Out of range; 0 assumed.\n");
		tapeunit = 0;
	}
	printf("Tape drive %d selected.\n",tapeunit);
	printf("\nThe tape on drive %d will be read from the current position\n",tapeunit);
	printf("at 800bpi, 5120 characters (10 blocks) per record,\n");
	printf("and written onto the pack on drive %d starting at block 0.\n\n",diskunit);
	while (!yes("Ready"))
		printf("So what's the matter?\n");
	tapa = 0;
	disa = 0;
	buf[2+256] = 0;
	if((*read)(tapa,&buf,sizeof buf,tapeunit)){
		printf("Hard tape error -- abort\n");
		exit(1);
	}
	if(buf[2+256])
		printf("Size of filesystem to be copied is %d blocks.\n",buf[2+256]);
	else	{
			printf("Bad format on tape -- abort\n");
			exit(1);
		}
	printf("What is the pack volume label? (e.g. p0001): ");
	getline(&ans);
	if (ans[0]=='\0')
		strcpy(&ans,"p0001");
	ans[5] = '\0';
	printf("The pack will be labelled %s.\n",&ans);
	strcpy(&buf[219+256],&ans);
	count = (buf[2+256]+9)/10;
	for(;;) {
		if ((*write)(disa,&buf,sizeof buf,diskunit)){
			printf("Hard disk error -- abort\n");
			exit(1);
		}
		disa =+ 10;
		tapa++;
		if(--count<=0)
			break;
		if ((*read)(tapa,&buf,sizeof buf,tapeunit)) {
			printf("Hard tape error -- abort\n");
			exit(1);
		}
	}
	printf ("The boot block for your type of disk drive will now be installed.\n");
	if ((*write)(0,boot,512,diskunit)) {
		printf("Warning: cannot write boot block.\n");
	}
	printf("\nThe file system copy is now complete.\n");
	printf("\nTo boot the basic unix for your disk and tape drives\n");
	printf("as indicated above, mount this pack on drive 0\n");
	printf("and read in the boot block (block 0) using\n");
	printf("whatever means you have available; see romboot(8), 70boot(8).\n");
	printf("\nThen boot the program %s using diskboot(8).\n",&nunix);
	printf("Normally:  #0=%s\n\n",&nunix);
	printf("The system will initially come up single-user; see init(8).\n");
	printf("If you have an upper case only console terminal,\n");
	printf("you must execute: stty lcase; see stty(1).\n");
	printf("\nAfter UNIX is up, link the file %s to unix using ln(1).\n",&nunix);
	printf("        # ln /%s /unix\n",&nunix);
	printf("\nSet the date(1).\n");
	printf("\nGood Luck!\n\n");
	printf("The tape will now be rewound.\n\n");
	(*read)(0,0,0,tapeunit);
	exit(0);
}
yes(qs)
char	*qs;
{
	printf("%s?: ",qs);
	getline(&ans);
	if (ans[0]=='y')
		return(1);
	if (ans[0]=='n' || ans[0]=='\0')
		return(0);
	printf("assumed 'n'\n");
	return(0);
}

number(qs)
char	*qs;
{
	register c, num;

	num = 0;
	printf("%s?: ",qs);
	getline(&ans);
	c = ans[0];
	if('0'<=c && c<='9')
		num = num*10 + c - '0';
	return(num);
}

getline(s)
register char *s;
{
	register char *bp, c;

	bp = s;
	while ((c=getchar())!='\n') {
		if (c=='@')
			s = bp; else
		if (c=='#')
			s--; else
		if (c==0177)
			exit(1); else
		*s++ = c;
	}
	*s = '\0';
}
