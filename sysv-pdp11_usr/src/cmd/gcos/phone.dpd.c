/*	@(#)phone.dpd.c	1.3	*/
/*	3.0 SID #	1.2	*/
/*
 * dpd -- data-phone daemon dispatcher
 *
 */

#define	SPIDER	0
#define	PHONE	1
#define	LPD	0
#define	FGET	0

char	dname[] = "DPDP";
char	dpd[]	= "/usr/spool/dpd";
char	dfname[30] = "/usr/spool/dpd/";
char	lock[]	= "/usr/spool/dpd/lock";
char	error[]	= "/usr/spool/dpd/errors";
int	WAITTM = 60;
int	tlimit = 16*60;

#include "daemon.c"

/*
 * The remaining part is the data phone interface.
 */

#define	FC4	0104
#define	FC10	0110
#define	OC	0100
#define	ACKOC	0102
#define	MCBCD	0110
#define	MCBIN	0120
#define	RS	036
#define	ETX	03
#define	MASK6	077
#define	MAXJOB	1050000		/*maximum job size for GCOS*/
#define	MAXLINE	83		/*maximum length of BCD record.*/

extern	char	*phone_id;	/*PHONE_ID must be defined in header file gcos.h*/
extern	int	NCONERR;	/*defined in gerts()*/
extern	char	*conerr[];	/*defined in gerts()*/
char	snumbf[]	= "/usr/spool/dpd/snumb";
char	scard[]	= "H$      snumb   XXxxx,31\036\003";
char	rcd[]	= "H$*$rcd\036\003";

FILE	*cfb = NULL;
int	snum	= 0;
int	ioc;
extern	char gbuf[];

dem_con()
{
	int i;

	if((i = connect(phone_id)) != 0) {
		if((i > 0) || (i < -NCONERR))
			logerr("Can't reach GCOS: %-9d", i);
		else
			logerr("Can't reach GCOS: %-9.9s", conerr[-i-1]);
		return(i);
	}
	if((ioc = gerts(FC4, OC, rcd, gbuf)) != ACKOC){
		if(strlen(gbuf) > 2)
			logerr("Bad ACK on rcd: ACK-OC = %-11o\n     %.50s",
				ioc, gbuf);
		else
			logerr("Bad ACK on rcd: ACK-OC = %-11o", ioc);
		dem_dis();
		return(1);
	}
	return(0);
}

dem_dis()
{
	gerts_dis();
	FCLOSE(dfb);
	FCLOSE(cfb);
}

dem_open(file)
char	*file;
{
}

dem_close()
{
}

get_snumb()
{
	FILE *f;
	int i, temp;

	if((f = fopen(snumbf, "r")) != NULL){
		fread((char *)&snum, 2, 1, f);
		fclose(f);
	}
	snum++;
	scard[16] = phone_id[0];
	scard[17] = phone_id[1];
	temp = snum;
	for(i = 20; i > 17; i--){
		scard[i] = (temp % 10) | '0';
		temp = temp/10;
	}
	scard[21] = ',';
	if(temp > 0)
		snum = 0;
	if((f = fopen(snumbf, "w")) == NULL)
		trouble("Can't write %s", snumbf);
	fwrite((char *)&snum, 2, 1, f);
	fclose(f);
	gwrite(FC4, scard, 25);
	snumb = &scard[16];
	scard[21] = 0;
}

lwrite()
{
	if(linel > MAXLINE+1){
		mvbaddf();
		trouble("line too long in job %s", dfname+LDIRNAM);
	}
	line[0] = MCBCD;
	line[linel++] = RS;
	if(line[1] == '$')
		gwrite(FC4, line, linel);
	else
		gwrite(FC10, line, linel);
}


sascii(fc)
char	fc;
{
	static char obuf[400];
	int io, j1, j2;
	int c1, c2, eof;
	int temp;
	extern char gcset[];

	if((cfb = fopen(&line[1], "r")) == NULL){
		sprintf(obuf, "Can't open %s", &line[1]);
		logerr(obuf);
		if(mesp+linel+12 <= message+MXMESS)
			mesp = copline(obuf, 400, mesp);
		return(0);
	}
	io = eof = 0;
	if(fc == 'F')
		c1 = ff;
	else
	    if((c1 = getc(cfb)) == EOF)
		eof = 1;
	while(eof == 0){
	    for(j1 = 1; j1 <= 2; j1++){
		obuf[io++] = MCBIN;
		for(j2 = 1; j2 <= 52; j2++){
			obuf[io++] = gcset[c1>>3 & MASK6];
			if((c2 = getc(cfb)) == EOF){
			    c2 = 0;
			    eof = 1;
			}
			temp = (c1<<3 | (c2>>6 & 07));
			obuf[io++] = gcset[temp & MASK6];
			obuf[io++] = gcset[c2 & MASK6];
			if((c1 = getc(cfb)) == EOF)
			    eof = 1;
			if(eof)
			    break;
		}
		obuf[io++] = RS;
		if(eof)
			break;
	    }
	    gwrite(FC10, obuf, io);
	    io = 0;
	}
	FCLOSE(cfb);
	return(0);
}

gwrite(fc, b, n)
int	fc;
char	*b;
int	n;
{
	register i;

	b[n] = ETX;
	ioc = gerts(fc, OC, b, gbuf);
	for(i = 0; i <= 10; i++){
		if(ioc == ACKOC){
			snsum += n-2;
			return;
		}
		if(strlen(gbuf) > 2)
			logerr("gwrite: %.40s", gbuf);
#ifdef DEBUG
		fprintf(stderr, "ioc=%o, fc=%o, buf='%s'.\n",
			ioc, gbuf[0], &gbuf[1]);
#endif
		if(ioc != 0100 || i >= 10)
			break;
		ioc = gerts(0115, OC, NULL, gbuf);
	}
	if(snsum > MAXJOB)
		mvbaddf();
	trouble("write err: ioc =%6o, pid %.7s",
		ioc, dfname+LPID);
}

etcp1()
{
}


/* VARARGS */
trouble(s, a1, a2, a3, a4)
char	*s;
int	a1, a2, a3, a4;
{
	logerr(s, a1, a2, a3, a4);
	FCLOSE(dfb);
	FCLOSE(cfb);
	if(retcode != 0)
		if(snsum > 0)	/*abort partial job*/
			gerts_abt();
		else
			gerts_dis();
	longjmp(env, 1);
}
