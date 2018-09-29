/*	vpmfmt.c 1.5 of 3/3/82
	@(#)vpmfmt.c	1.5
 */

#include <stdio.h>
#include "sys/lapbtr.h"
#define MASK16	0177777
#ifdef vax
#define	SSTOL(x, y)	((((long)x)<<16)|(((long)y)&MASK16))
#else
#define	SSTOL(x, y)	((((long)y)<<16)|(((long)x)&MASK16))
#endif
static char Sccsid[]="@(#)vpmfmt.c	1.5";
struct event {
	unsigned short	lbolt1;
	unsigned short	lbolt2;
	unsigned short	seq;
	unsigned char	typ;
	unsigned char	dev;
	unsigned short	wd1;
	unsigned short	wd2;
} ev;
main(argc, argv)
char *argv[];
{
	extern int optind;
	int x, fd, k, n, seqno, con, topt, errflg;
	char *type, c;
	long time1;
	char xxx;

	while ((c = getopt (argc, argv, "t")) != EOF)
		switch (c) {
		case 't':
			topt = 1;
			break;
		case '?':
			errflg++;
		}
	if (errflg) {
		fprintf(stderr, "usage: vpmfmt [-t]\n");
		exit(1);
	}
	if (argc - optind > 0){
		fprintf(stderr,"Arg count\n");
		exit(2);
	}
	setbuf(stdout,NULL);
	seqno = 1;
	for (;;) {
		x = getchar();
		n = getchar();
		if ((k = fread((char *) &ev, sizeof(xxx), n, stdin)) < 0) {
			perror();
			exit(3);
		}
		if (k == 0) {
			clearerr(stdin);
			sleep(1);
			continue;
		}
		if (ev.seq != seqno)
			printf("**%d event records lost**\n", ev.seq - seqno);
		seqno = ev.seq + 1;
		if (k == 12) {
			time1 = SSTOL(ev.lbolt2, ev.lbolt1);
			printf(" %10lu%6d", time1, ev.seq);
			switch((int)ev.typ){
			case 'T':
				type = "trace";
				printf(" %-8s %2o", type, ev.dev);
				if(topt){
					switch(ev.wd2&0340){
					case TXMT: 
					case TDSCFRM: 
					case TRCV:
						if(((ev.wd2&0377)&0360) == TXMT)
							printf(" XMT   ");
						else if(((ev.wd2&0377)&0360) == TDSCFRM)
							printf(" DSC   ");
						else
							printf(" RCV   ");
						con = (ev.wd2>>8)&0377;
						if((con&01) == 0){
							printf("I	%1o%3o%3o",(con>>5)&07,(con>>4)&01,(con>>1)&07);
						}
						else{
							switch(con&017){
							case 01:
								printf("RR	%1o%3o",(con>>5)&07,(con>>4)&01);
								break;
							case 05:
								printf("RNR	%1o%3o",(con>>5)&07,(con>>4)&01);
								break;
							case 011:
								printf("REJ	%1o%3o",(con>>5)&07,(con>>4)&01);
								break;
							case 017:
								if(con&040)
/* code folded from here */
	printf("SABM	%4o",(con>>4)&01);
/* unfolding */
								else
/* code folded from here */
	printf("DM	%4o",con&07);
/* unfolding */
								break;
							case 03:
								if(con&040)
/* code folded from here */
	printf("UA	%4o",(con>>4)&01);
/* unfolding */
								else
/* code folded from here */
	printf("DISC	%4o",(con>>4)&01);
/* unfolding */
								break;
							case 07:
								printf("FRMR	%4o",(con>>4)&01);
								break;
							default:
								printf(" Funny frame");
								break;
							}
						}
					default:
						printf("%7o%6o%6o", ev.wd1, ev.wd2&0377, (ev.wd2>>8)&0377);
					}
				} else {
					printf("%7o%6o%6o", ev.wd1, ev.wd2&0377, (ev.wd2>>8)&0377);
				}
				break;
			case 's':
				type = "start";
				printf(" %-8s ",type);
				printf("%2o%6o%6o", ev.dev, 
				    ev.wd1, ev.wd2);
				break;
			case 'w':
				type = "write";
				printf(" %-8s %2o%6o%6o", type, ev.dev, 
				    ev.wd1, ev.wd2);
				break;
			case 'r':
				type = "read";
				printf(" %-8s %2o%6o%6o", type, ev.dev, 
				    ev.wd1, ev.wd2);
				break;
			case 'o':
				type = "open";
				printf(" %-8s %2o%6o%6o", type, ev.dev, 
				    ev.wd1, ev.wd2);
				break;
			case 'c':
				type = "close";
				printf(" %-8s %2o%6o%6o", type, ev.dev, 
				    ev.wd1, ev.wd2);
				break;
			case '0':
				type = "chan0";
				printf(" %-8s %2o%6o%6o%6o%6o",type,ev.dev,
				    (ev.wd1>>8)&0377,ev.wd1&0377,(ev.wd2>>8)&0377,ev.wd2&0377);
				break;
			case 'e':
				type = "recv";
				printf(" %-8s %2o%6o%6o%6o%6o",type,ev.dev,
				    (ev.wd1>>8)&0377,ev.wd1&0377,(ev.wd2>>8)&0377,ev.wd2&0377);
				break;
			case 'v':
				type = "csixmt";
				printf(" %-8s %2o%6o%6o%6o%6o",type,ev.dev,
				    (ev.wd1>>8)&0377,ev.wd1&0377,(ev.wd2>>8)&0377,ev.wd2&0377);
				break;
			case 'q':
				type = "flg_seq";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'j':
				type = "kmcset";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'p':
				type = "no_start";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'i':
				type = "vpbioctl";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'k':
				type = "setkmc";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'Z':
				type = "stop1";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 't':
				type = "stop2";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'L':
				type = "trychg";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'Q':
				type = "chgover";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'K':
				type = "bkrint";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'H':
				type = "stop_ack";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'y':
				type = "badrbuf";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'x':
				type = "badxbuf";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'I':
				type = "itrace";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'E':
				type = "INTterm";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'P':
				type = "scr_rpt";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'F':
				type = "errs_rtn";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'B':
				type = "strt_ack";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'A':
				type = "base_ack";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'C':
				type = "csiclean";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'Y':
				type = "vpmwake";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'O':
				type = "OK ck";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'R':
				type = "p-fail";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'S':
				type = "snap";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1&0377, (ev.wd1>>8)&0377, ev.wd2&0377, (ev.wd2>>8)&0377);
				break;
			case 'n':
				type = "ncioctl";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			case 'm':
				type = "x25-err";
				printf(" %-8s %2o%7o%7o",type,ev.dev,
				    ev.wd1, ev.wd2);
				break;
			default:
				printf(" %-10c%2o%7o%6o", ev.typ, ev.dev, 
				    ev.wd1, ev.wd2);
			}
			printf("\n");
		}
	}
}

