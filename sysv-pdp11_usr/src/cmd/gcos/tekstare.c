/*	@(#)tekstare.c	1.2	*/
/*	3.0 SID #	1.2	*/
# include "stdio.h"
# define ESC 033
# define CR '\r'
# define US 037
# define GS 035
# define RS 036
# define LF '\n'
# define FF 014
# define FS 034
# define ENQ 005
# define SUB 032
# define ALPHA 0
# define GRAPH 1
# define PTPLOT 2
# define INCPLOT 3

# define  CONTROL 1
# define HIGH 2
# define LOWX 3
# define LOWY 4
# define ILLEGAL 5
# define HIMASK  (037<<7)

#ifdef unix
#define	SPTYPE	GCOSB
#define FIRSTCHAR 'k'-1
#define	NTEMP	3
char	Xfname[NTEMP][25];
char	Xchar[NTEMP] = {'d', 't', 'y'};
char	*dfname = Xfname[0];
char	*tfname = Xfname[1];
char	*yfname = Xfname[2];
char	*NAME;

#include	"spool.c"
#endif

#ifdef gcos
int debug = 0;
int nact = 0;
int wantmail = 0;
char	*mailfile = 0;
int	argc;
char	**argv;
char	*arg;
FILE *tff = stdout;
char	file1[30];
#endif

char *remotid;
char *media;
char *demon;
int staref = 1;

int sizetb[] = {40,80,120,160};
FILE *cin = stdin;
FILE *cout = stdout;

main (agc, agv)
int agc;
char *agv[];
{
	/* turn tek stuff into fortran readable format */

	int mode, cx, cy, hix, k1, cc, nx, ny, newg, cl, c2, lowy;
	int lobits, hiy, csize;
	char str[100], *sp;

	argc = agc;
	argv = agv;
	csize = 1;
# ifdef unix
	setup();
	remotsw = ONL;
#ifndef NMAIL
	wantmail = 1;		/*default is send mail.*/
#endif
	grade = '2';
# endif
# ifdef gcos
	sprintf( file1, "ieh1335g");
# endif
# ifdef unix
	demon = "L";
# endif
# ifdef gcos
	demon = "";
# endif
	while (argc >= 2 && (arg = argv[1])[0] == '-'){
		switch (arg[1]){

		case '\0':
		case 'd': case 'D': debug = 1; break;
		case 'j': case 'J': wantmail = 1; break;
		case 's': case 'S':
			if(arg[2])  goto unknown;
			staref = 1; break;
		case 'm': case 'M':
			if(arg[2])  goto unknown;
			staref = 0; media= "print"; break;
		case 'r': case 'R': remotid = argv[1]+2; break;
		case 'v': case 'V': staref=0; media = "vugr"; break;
		case 'c': case 'C': staref=0; media= "cdraw"; break;
		case 'p': case 'P': staref=0; media = "photo"; break;
		case 'f': case 'F': staref=0; media = "print"; break;
		case 'l': case 'L': staref=0; media = "slide"; break;

unknown:
		default:
#ifdef unix
			if(!comopt(arg[1]))
#endif
				err("Unknown flag %s.", arg);
			break;

		}
		argv++;
		argc--;
	}
	gcos1();
	if (remotid)
		fprintf(tff,"%s$      remote  **,%s\n", demon, remotid);
	else
		fprintf(tff,"%s$      remote  **,onl\n", demon);
# ifdef unix
	fprintf(tff,"L$      select  cc/unixsupport/fsend.2\n");
	fprintf(tff,"La *ot\n");
	fprintf(tff,"L$      file    ot,x1s,10l\n");
	fprintf(tff,"L$      data    i*,ncksum\n");
	yfname[INCHAR]++;
	fprintf(tff,"B%s\n",yfname);
# endif
   /* the following is needed, but god only knows why */
	fprintf(tff,"%s$      option  fcb,nomap\n", demon);
	fprintf(tff,"%s$      lowload\n", demon);
	fprintf(tff,"%s$      use     ......\n", demon);
	fprintf(tff,"%s$      library x0,x1\n", demon);
	fprintf(tff,"%s$      library gl\n", demon);
	if (staref == 0)
		fprintf(tff,"%s$      use     %s\n", demon, media);
	fprintf(tff,"%s$      execute\n", demon);
	fprintf(tff,"%s$      limits  10,30k\n", demon);
	if (remotid)
		fprintf(tff,"%s$      remote  GR,%s\n",demon,remotid);
	else
		fprintf(tff,"%s$      remote  GR,s3\n",demon);
	fprintf(tff,"%s$      prmfl   x0,,,pounce/stare.r\n", demon);
	fprintf(tff,"%s$      prmfl   x1,,,bfor/batchlib\n", demon);
	fprintf(tff,"%s$      prmfl   gl,,,cc/graphics\n", demon);
	cout = tff;
# ifdef unix
	fprintf(tff,"L$      file    05,x1d\n");
	if((cout = fopen(yfname, "w")) == NULL)
		errout("Can't create %s", yfname);
# endif
	fprintf(cout,"%s\n", staref? "STARE" : "MICRO");


    do{
	if (argc >1){
		arg = *++argv;
		if((cin = fopen(arg, "r")) == NULL){
			err("Cannot open %s", arg);
			continue;
		}
		if(mailfile == 0)
			mailfile = arg;
	}else
		if(mailfile == 0)
			mailfile = "pipe.end";
	nact++;
	mode = ALPHA;
	cy = cx = 0;
	sp = str;
	while (cc=getch()){
		switch (cc){
		case ESC:
			switch(c2=getch()){
			case FF:
				fprintf(cout,"F\n");
				mode = ALPHA;
				newg=0;
				continue;
			case ENQ:
			case SUB:
				continue;
			case '8':
			case '9':
			case ':':
			case ';': /* size changes */
				csize = ';' - c2; /*0,1,2,3*/
				fprintf(cout,"Z%5d\n", (csize+1)*5);
				continue;
			case '`': case 'a': case 'b': case 'c': case 'd': case 'e':
				/* various kinds of dotted lines */
				continue;
			}
			err("unknown sequence ESC 0%o\n",c2);
			continue;
		case CR:
			mode = ALPHA;
			cx = 0;
			newg=0;
			continue;
		case LF:
			if (mode==ALPHA) {
				*sp++ = '\0';
				outstr(sp=str, cx, cy);
				cy = cy - sizetb[csize];
				if (cy <= 0)
					cy = 4000;
				cx = 0;
			}
			continue;
		case US:
			mode = ALPHA;
			newg=0;
			continue;
		case GS:
			mode = GRAPH;
			newg = 1;
			*sp++ = '\0';
			outstr(sp=str, cx,cy);
			continue;
		case FS:
			mode = PTPLOT;
			*sp++ = '\0';
			outstr(sp=str, cx, cy);
			continue;
		case RS:
			mode = INCPLOT;
			continue;
		}
		switch(mode){
		case ALPHA:
			*sp++ = cc;
			continue;
		case GRAPH:
		case PTPLOT:
			hix = hiy = lowy = lobits = 0;
			ny = cy & ~03;;
			for (k1=cc; k1 && (cl=class(k1)) != LOWX; k1=getch())
				switch(cl){
				case HIGH:
					if (lowy)
						hix = k1;
					else
						hiy = k1;
					continue;
				case LOWY:
					if (lowy != 0)
						lobits = lowy;
					lowy = k1;
					continue;
				default:
					err("wha? %o\n",k1);
					continue;
			}
			if (hix)
				nx = ((hix-040) << 7) + ((k1 - 0100) << 2);
			else
				nx = (cx & HIMASK) + ((k1 - 0100) <<2);
			if (hiy && lowy)
				ny = ((hiy-040) << 7)  + ((lowy - 0140) << 2);
			else if (hiy)
				ny = ((hiy-040) << 7) + (cy &(037<<2));
			else if (lowy)
				ny = (cy & HIMASK) + ((lowy - 0140)<<2);
			if (lobits){
				ny += ((lobits & 014) >> 2);
				nx += (lobits &03);
			}
			else{
				ny += (cy & 03);
				nx += (cx&03);
			}
			if (mode == GRAPH && !newg)
				fprintf(cout,"S%5d%5d%5d%5d\n",cx,cy,nx,ny);
			if (mode == PTPLOT)
				fprintf(cout,"P%5d%5d%5d%5d\n",nx,ny,0,0);
			newg = 0;
			cx = nx;
			cy = ny;
			break;
		case INCPLOT:
			switch(cc){
				case ' ': /* beam off */
					newg=1; continue;
				case 'P': /* beam on */
					newg=0; continue;
				case 'D': /* north */
					nx = cx; ny = cy+1; break;
				case 'E': /* ne */
					nx = cx+1; ny = cy+1; break;
				case 'A': /* e */
					nx = cx+1; ny = cy; break;
				case 'I': /* se */
					nx = cx+1; ny = cy -1; break;
				case 'H': /* s */
					nx = cx; ny = cy -1; break;
				case 'J': /* sw */
					nx = cx -1; ny = cy -1; break;
				case 'B': /* w */
					nx = cx -1; ny = cy; break;
				case 'F': /* nw */
					nx = cx-1; ny = cy+1; break;
				default:
					err("incplot?? %o\n",k1); continue;
			}
			if (newg==0)
				fprintf(cout,"S%5d%5d%5d%5d\n",cx,cy,nx,ny);
			cx=nx; cy=ny;
			break;
		}
	}
	if (argc>2)
		fprintf(cout,"F\n");
    }while(argc-- > 2);

# ifdef unix
	fclose(cout);
	if(size(yfname, yfname) <= 0)
		out();
	fprintf(tff, "U%s\n", yfname);
# endif
	/* if (!staref)
	fprintf(tff,"%s$      select  mfilm/37\n", demon); */
	gcos2();
}
outstr(s,cx,cy)
char *s;
{
	int n;
	char slewed[200], tabbed[200];

	untab(tabbed, s);
	n = slen(tabbed);
	if (n==0) return;
	escape(slewed, tabbed);
	fprintf(cout,"L%5d%5d%5d%5d\n%.80s\n",cx,cy,n,0,slewed);
}
slen(s)
char *s;
{
	int n;
	for (n=0; *s++; n++);
	return(n);
}
untab(new, old)
char *new, *old;
{
	int col, c;
	char *nstart;
	nstart = new;
	while (c = *old++)
		if (c != '\t')
			*new++ = c;
		else{
			*new++ = ' ';
			col = new-nstart -1;
			while (++col%8)
				*new++ = ' ';
		}
	*new++ = 0;
}
escape (new, old)
 char *new, *old;
 { /* escapes lower case letters */
 int c;
 while (c = *old++)
  {
  if (c == '#')
     *new++ = '#';
  if (c >= 0140)
     {
     *new++ = '#';
     c -= 040;
      }
 if (c>= 'a' && c<= 'z') c+= 040;
 *new++ = c;
  }
*new = 0;
}
class (val)
{
	switch (val>>5){
		case 0: return (CONTROL);
		case 1: return(HIGH);
		case 2: return(LOWX);
		case 3: return(LOWY);
	}
	return(ILLEGAL);
}
getch()
{
int c;
while ( (c=getc(cin))==0 && !feof(cin))
	;
if (c<0) c= 0;
return(c);
}

#ifdef gcos
gcos1()
{
	if (!debug)
	tff = fopen(file1, "w");
/*  the following line causes stare as well as print to vanish
	fprintf(tff,"%s$      sysout  toss\n", demon);
 */
}

gcos2()
{
	fprintf(tff,"%s$      endjob\n", demon);
	fclose(tff);
	if (debug) exit(0);
	system("jrun ident;ieh1335g");
	exit(0);
}
err(s,n)
char *s;
{
	fprintf(stderr, s, n);
}
#endif
