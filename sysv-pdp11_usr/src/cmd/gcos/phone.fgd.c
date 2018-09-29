/*	@(#)phone.fgd.c	1.2	*/
/*	3.0 SID #	1.2	*/
/*
 * Retrieve files from GCOS via dataphone
 */

#define	SPIDER	0
#define	PHONE	1
#define	LPD	0
#define	FGET	1

char	dname[] = "FGETP";

#include "fgdemon.c"

#define	ETX	003
#define	RS	036
#define	BINREC	0127
#define	FLUSH	(*output)(inp, lbuf-inp);  lbuf = inp;
#define	BADCHAR	cgset[cp[1] - 040] & 040

extern	int	NCONERR;	/*defined in gerts()*/
extern	char	*conerr[];	/*defined in gerts()*/
char	conmes[] = "Can't connect to GCOS: %-14.14s";
char	cerrmes[] = "Can't connect to GCOS: ERROR %-8d";

extern	char	*fget_id;	/*FGET_ID must be defined in header file gcos.h*/
char	inp[BUFSIZ];
char	*ebuf;
char	*lbuf	= inp;
char	savc;
int	state;
int	ioc;
char	ibuf[BUFSIZ];
char	out[] = "H$*$out\036\003";


fgd_read()
{
	register i, n;
	register char *cp;
	extern char cgset[];

	backup[0] = 0;
	while((i = connect(fget_id)) != 0) {
		if((i > 0) || (i < -NCONERR))
			trouble(BADTRB, 60, cerrmes, i);
		else
			trouble(BADTRB, 60, conmes, conerr[-i-1]);
	}

	if((ioc = gerts(0104, 0100, out, ibuf)) != 0103){
#ifdef DEBUG
		fprintf(stderr, "ioc = %o; *ibuf = %o\n", ioc, *ibuf);	/*DEBUG*/
		fprintf(stderr, "out\n");	/*DEBUG*/
#endif
		trouble(BADTRB, 60,"out: ioc=%o, buf='%.40s'", ioc, ibuf);
	}

	if((ioc = gerts(0102, 0100, (char *)0, ibuf)) != 0102) {
#ifdef DEBUG
		fprintf(stderr, "sack\n");	/*DEBUG*/
#endif
		trouble(BADTRB, 60,"sack: ioc=%o, buf='%.40s'", ioc, ibuf);
	}

	if((ioc = gerts(0102, 0100, (char *)0, ibuf)) != 0100) {
#ifdef DEBUG
		fprintf(stderr, "data\n");	/*DEBUG*/
#endif
		trouble(BADTRB, 60,"data: ioc=%o, buf='%.40s'", ioc, ibuf);
	}

	ebuf = inp + BUFSIZ;
	do {
#ifdef DEBUG
		fprintf(stderr, "%o %o %o %o\n", ibuf[0], ibuf[1],
			ibuf[2], ibuf[3]);	/*DEBUG*/
#endif
		cp = ibuf;
		state = 0;
		while (*++cp != ETX) {
			if(*cp == BINREC && !(BADCHAR)) {
				while (*++cp != RS) {
					if(cp >= ibuf+BUFSIZ) {
#ifdef DEBUG
						fprintf(stderr, "overflow\n");
#endif
						trouble(BADTRB, 60,"overflow");
					}
					if(cp[1] == 037) {
						n = cgset[cp[2]-040]+1;
						while(n--)
							if(putcod(*cp)){
								FLUSH
							}
						cp += 2;
					} else if(putcod(*cp)){
						FLUSH
					}
				}
			} else {
				if(cp[1] == '!' && cp[2] == '!') {
					FLUSH
					endfile();
				}
				while(*++cp != RS)
					if(cp >= ibuf+BUFSIZ)
						goto rrec;
				state = 0;
			}
		}
rrec:		;
	} while ((ioc = gerts(0110, 0100, (char *)0, ibuf)) == 0100);
	if(strlen(ibuf) > 2)
		trouble(BADTRB, 60,"rrec: ioc=%o, buf='%.40s'", ioc, ibuf);
	return;
}


dem_dis()
{
	FCLOSE(ofd);
	FCLOSE(tfd);
	gerts_dis();
}


putcod(ch)
{
	extern char cgset[];
	register c, code;

	if(ch < 040 || ch >= 0140) {
#ifdef DEBUG
		fprintf(stderr, "%o: ", ch);		/*DEBUG*/
#endif
		fgderr("Bad code: %o, in file %s", ch, fname);
	}
	c = cgset[ch-040];
	switch(state) {

	case 0:
		state++;
		savc = c;
		return(0);

	case 1:
		state++;
		code = (savc<<3)|(c>>3);
		savc = c&07;
		break;

	case 2:
		state = 0;
		code = c | ( savc<<6);
		break;
	}

#ifdef DEBUG
	putc(code, stderr);	/*DEBUG*/
#endif
	*lbuf = code;
	if(++lbuf >= ebuf || code == '\n')
		return(1);
	return(0);
}

fgderr(s, ch, f)
char	*s;
char	ch;
char	*f;
{

	if(ofd != NULL){
		fprintf(ofd, s, ch, f);
		putc('\n', ofd);
	}
	trouble(BADTRB, 60,s, ch, f);
}
