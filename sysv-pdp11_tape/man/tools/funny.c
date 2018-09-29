#include	<stdio.h>
#include	<ctype.h>

main(argc,argv)
int	argc;
char	*argv[];
{
	register int	c;
	register char	*letter;
	register int	cnt = NULL;
	register int	loop;
	int	newline = NULL;
	int	filename = NULL;
	FILE	*fopen(), *fptr = NULL;

	if(argc == 1) {
		fptr = stdin;
	}
	else if(argc > 2) {
		filename++;
	}

	for(loop=1; loop < argc; ++loop) {
		if(fptr == NULL) {
			if((fptr=fopen(argv[loop],"r")) == NULL) {
	                        printf("cannot open %s\n",argv[loop]);
				continue;
			}
		}


		newline = 1;
		cnt = 0;
		while((c=fgetc(fptr)) != EOF) {
			cnt++;
			c &= 0377;
			if(c == '\n') {
				newline++;
				continue;
			}
			if(! isprint(c) && ! isspace(c)) {
				switch (c) {
					case 000:
						letter = "nul";
						break;
					case 001:
						letter = "soh";
						break;
					case 002:
						letter = "stx";
						break;
					case 003:
						letter = "etx";
						break;
					case 004:
						letter = "eot";
						break;
					case 005:
						letter = "enq";
						break;
					case 006:
						letter = "ack";
						break;
					case 007:
						letter = "bel";
						break;
					case 010:
						letter = "bs";
						break;
					case 016:
						letter = "so";
						break;
					case 017:
						letter = "si";
						break;
					case 020:
						letter = "dle";
						break;
					case 021:
						letter = "dc1";
						break;
					case 022:
						letter = "dc2";
						break;
					case 023:
						letter = "dc3";
						break;
					case 024:
						letter = "dc4";
						break;
					case 025:
						letter = "nak";
						break;
					case 026:
						letter = "syn";
						break;
					case 027:
						letter = "etb";
						break;
					case 030:
						letter = "can";
						break;
					case 031:
						letter = "em";
						break;
					case 032:
						letter = "sub";
						break;
					case 033:
						letter = "esc";
						break;
					case 034:
						letter = "fs";
						break;
					case 035:
						letter = "gs";
						break;
					case 036:
						letter = "rs";
						break;
					case 037:
						letter = "us";
						break;
					case 0177:
						letter = "del";
						break;
					default:
						letter = "unknown";
						break;
				}
		if(filename) printf("%s: ",argv[loop]);
				printf("line/char=%d/%d: %#3.3o (%s)\n",
					newline,cnt,c,letter);
			}
		}
		fclose(fptr);
		fptr = NULL;
	}
}
/*	@(#)funny.c	5.2 of 5/19/82	*/
