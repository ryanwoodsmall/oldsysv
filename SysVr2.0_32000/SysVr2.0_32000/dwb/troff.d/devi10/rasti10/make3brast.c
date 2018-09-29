char	xxx3brast[] = "@(#)make3brast.c	1.1";

/* This program translates a Imagen binary font in VAX format to 3b20 format.
It should be run on a 3b20 with vax fonts as input.
Input file is indicated by argument 1
Standard output is the converted file
Option "-f" forces conversion despite incorrect magic number for VAX
*/
#include <fcntl.h>
#include <stdio.h>

main(argc,argv)
int argc;
char **argv;
{
#ifdef u3b
	int i,j; /*counters */
	unsigned char x[2]; /*holds two chars */
	char null = '\0';
	int fd;
	union u_mag{
		char ch[2];
		short s;
	} magic;
	int forcesw = 0;

	if (strcmp(argv[1], "-f") == 0) {
		forcesw++;
		argv++;  argc--;
	}
	if (argc < 2)
	{
		fprintf(stderr,"usage -- missing font name\n");
		exit(1);
		/*no font name provided */
	}
	if((fd=open(argv[1],O_RDONLY)) < 0)
	{
		fprintf(stderr," font file does not exist %s \n",argv[1]);
		exit(0);
	}
	read(fd, &magic.s, 2);		/*check first 2 bytes for magic number*/
	if (magic.s != 7681) {			/*magic number for a VAX file*/
		if (magic.s == 286)		/*magic number for a 3B file*/
			fprintf(stderr, "Input file is already made for 3B\n");
		else
			fprintf(stderr, "Unrecognized magic number\n");
		if (forcesw == 0)
			exit(2);
	}
	write(1, &magic.ch[1], 1);
	write(1, &magic.ch[0], 1);
	for(i=0;i<4;i++)
	{
		flipbyte(fd); /*the next eight bytes are swabed,
				correcting the global header */
	}
		fillbyte(2); /*two fill bytes are inserted */
	for(j=0;j<256;j++) /*this involves corecting an array of 256 structures
			      which describe each character */
	{
		flipbyte(fd); /* swab the bytes */
		flipbyte(fd);
		for(i=0;i<6;i++)
		{
			putbyte(fd); /* put out six chars */
		}
		fillbyte(2); /* put out two fill bytes */
	}
	while(read(fd,&x[0],1)) /* read and then write out the bit map 1
				byte at a time */
	{
		write(1,&x[0],1);
	}
	close(fd);
}
flipbyte(fd) /* swabs a short int */
int fd;
{
	unsigned char y[2];
	read(fd,&y[1],1);
	read(fd,&y[0],1);
	write(1,&y[0],1);
	write(1,&y[1],1);
}
putbyte(fd) /*puts out a byte */
int fd;
{
	unsigned char y;
	read(fd,&y,1);
	write(1,&y,1);
}
fillbyte(n) /*puts out N null bytes */
int n;
{
	unsigned char x = '\0';
	int i;
	for(i=0;i<n;i++)
	{
		write(1,&x,1);
	}
#endif
}	
