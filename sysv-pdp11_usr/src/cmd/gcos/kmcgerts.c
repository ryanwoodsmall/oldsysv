/*	@(#)kmcgerts.c	1.3	*/
/*	3.0 SID #	1.3	*/
/* gerts communication subroutines*/

#include <stdio.h>
#include <signal.h>
#if RES
#include <sgtty.h>
#else
#include <fcntl.h>
#endif
extern	errno;

/* The following parameters are now defined in "gcos.h". */
extern	char	*du;	/*dataphone device; DEVDU must be defined in gcos.h*/
extern	char	*dn;	/*ACU device; DEVDN must be defined in gcos.h*/
extern	char	*numbers[];	/*phone numbers to dial GCOS*/
				/*NUMBERS must be defined in gcos.h*/
extern	int	NNUMB;		/*length of numbers[]; calculated in config.c*/
extern	char	*console;	/*device for manual dial message;*/
				/*CONSOLE must be defined in gcos.h*/

#define	OKRET	1		/*KMC returns*/
#define	ERRET	-1
#define	TIMEOUT	-2
#define	CKRET	-3

			/*gerts error returns*/
#define	BUSY	-1
#define	TIMEOUT	-2
#define	BADID	-3
#define	ERROP	-4	/*connect; devdu not open in gerts().*/
#define	ERROI	-5	/*too much input*/
#define	ERRWR	-6	/*error from write.*/
#define	ERRRD	-7	/*error from read.*/
#define	ERROO	-8	/*too much output*/
#define	ERRO3	-9	/*no ETX returned from KMC*/
#define	ERRKTO	-10	/*timeout from KMC*/
#define	ERRCKO	-11	/*checksum error from KMC*/
#define	ERRCKI	-12	/*checksum mismatch with KMC*/
#define	ERRK	-16	/*unrecognized return from KMC*/
	/*	<-16	/*error reported by KMC at char -(errno-ERRK-1)*/

char	*conerr[12] = {"DEV BUSY", "TIMEOUT", "BAD ID", "ERROR",
		"OVFLOWI", "WRITE ERROR", "READ ERROR", "OVFLOWO",
		"NO ETX", "KMC TIMEOUT", "KMC CKO", "KMC CKI"};
int	NCONERR = sizeof(conerr)/sizeof(char *);

#define	FCLOSE(F)	if(F != -1){ close(F);  F = -1;}
#define	SCHAR(C)	buf[index++] = C;  ock ^= C;
#define	CCHAR(C)	if(index>=n-1) goto gerror; C=buf[index++]; ick^=C;
#define	ZERONUM(J)	!numbers[J] || !*numbers[J]

#define	LBUF 400
#define	ACKOC	0102
#define	SOH	001
#define	STX	002
#define	ETX	003

char	mancom[] = "Please dial %s on dataphone to connect to GCOS.\n";
char	idmes[] = "H$*$XX passwd\036\003";
char	dis[]	= "H$*$dis\036\003";
int	duio = -1;
int	dno = -1;
int	gclosed = 1;		/*0 means must send GRTS a disconnect.*/
int	gertsrc;
char	ock;		/*checksum of buffer sent to KMC.*/
char	ick;		/*checksum of buffer returned from KMC.*/
char	gbuf[BUFSIZ];	/*buffer for gerts debugging*/

connect(id)
char id[2];
{
	register int i;
	int j, n, ioc, saverc;
	FILE *fc;
	int gertsto();

	signal(SIGALRM, gertsto);
	saverc = ERROP;
	idmes[4] = id[0];
	idmes[5] = id[1];
	FCLOSE(dno);		/*I hope this isn't necessary. MRW*/
	FCLOSE(duio);		/*ditto*/

	for(j = 0; j < NNUMB; j++){
		if((duio = open(du, 2)) == -1)
			return(BUSY);
#if RES
		if(ioctl(duio, FIOCLEX, 0) == -1)	/*close-on-exec*/
#else
		if(fcntl(duio, F_SETFD, 1) == -1)	/*close-on-exec*/
#endif
			logerr("fcntl returned errno %d", errno);
		if(!(ZERONUM(j))){
			if(dn && *dn){
				if((dno = open(dn, 1)) == -1){
					FCLOSE(duio)
					return(BUSY);
				}
				i = strlen(numbers[j]);
				if((n = awrite(dno, numbers[j], i, 120)) != i)
					goto retry;
				FCLOSE(dno);
			}else{
				fc = fopen(console, "w");
				fprintf(fc, mancom, numbers[j]);
				fclose(fc);
				sleep(60);
			}
		}
		sleep(10);
		if ((saverc = ioc = gerts(0103, 0102, 0, gbuf)) != ACKOC)
			goto retry;
		gclosed = 0;
		if((ioc = gerts(0104, 0100, idmes, gbuf)) == ACKOC)
			return(0);
#if DEBUG >= 1
		fprintf(stderr, "ioc=%o\n", ioc);
#endif
		saverc = BADID;
retry:
		gerts_dis();
		if(ZERONUM(j)  ||  j+2 > NNUMB  ||  ZERONUM(j+1))
			break;
		sleep(20);
	}
	if(strlen(gbuf) > 2)
		logerr("connect: '%.60s',\n     ioc = %o", gbuf, ioc);
	return(saverc);
}


gerts(ofc, ooc, ibuf, obuf)
char ofc, ooc;
char *ibuf, *obuf;
{
	char ifc, ioc;
	static char buf[LBUF+1];
	register int index, i;
	register char c;
	int n, xobuf;

	int gertsto();
	signal(SIGALRM, gertsto);
#if DEBUG >= 1
	fprintf(stderr, "gerts(%o, %o, %.40s, %.40s);\n", ofc, ooc, ibuf, obuf);
#endif
	index = xobuf = 0;
	if(duio == -1){
		gertsrc = ERROP;
		goto gerror;
	}
	ock = 0;
	SCHAR(ofc);
	SCHAR(ooc);
	SCHAR(STX);
	if(ibuf)
		for(i = 0; ibuf[i] != ETX; i++){
			SCHAR(ibuf[i]);
			if(index >= LBUF-4){
				gertsrc = ERROI;
				goto gerror;
			}
		}
	SCHAR(ETX);
	SCHAR(ock);
	buf[index] = 0;
	gertsrc = ERRWR;

#if DEBUG >= 2
	fprintf(stderr, "write # %d('%s', %d)\n", 1, buf, index);
#endif
	if((n = awrite(duio, buf, index, 10)) != index)
		goto gerror;
	for(i=0; i<LBUF+1; i++)
		buf[i] = 0;
	index = 0;
	gertsrc = ERRRD;
	if((n = aread(duio, buf, LBUF, 60)) <= 0)
		goto gerror;
#if DEBUG >= 2
	fprintf(stderr, "read # %d('%s', %d)\n", 1, buf, n);
#endif

	if((i = buf[n-1]) != OKRET){
		if(i == TIMEOUT)
			gertsrc = ERRKTO;
		else if(i == CKRET)
			gertsrc = ERRCKO;
		else if(i == ERRET)
			gertsrc = ERRK-n;
		else
			gertsrc = ERRK;
		goto gerror;
	}
	gertsrc = ERRO3;
	ick = 0;
	CCHAR(ifc);
	CCHAR(ioc);
	CCHAR(c);
	while(c != STX){
		CCHAR(c);
	}
#if DEBUG >= 1
	fprintf(stderr, "received fc=%o, oc=%o, message='%.40s'\n",
		ifc, ioc, &buf[index-1]);
#endif
	if(obuf)
		obuf[xobuf++] = ifc;
	while(c != ETX){
		CCHAR(c);
		if(obuf)
			obuf[xobuf++] = c;
		if(index >= LBUF){
			gertsrc = ERROO;
			goto gerror;
		}
	}
	if(obuf)
		obuf[xobuf] = 0;
	if(index < n-1 && ick != buf[index]){
		gertsrc = ERRCKI;
		goto gerror;
	}
	return(ioc & 0707);
gerror:
#if DEBUG >= 1
	fprintf(stderr, "gerror%d: n=%d, index=%d.\n  buf='%s'.\n",
		gertsrc, n, index, buf);
#endif
	if(obuf)
		obuf[xobuf] = 0;
	return(gertsrc);
}


gerts_dis()
{
	if(gclosed++ <= 0){
		gerts(0104, 0100, dis, 0);
		gerts(0102, 0104, 0, 0);	/*tell GCOS to disconnect NOW*/
	}
	FCLOSE(duio);
	FCLOSE(dno);
}


gerts_abt()		/*if error in middle of job, abort it*/
{
	FCLOSE(duio);
	FCLOSE(dno);
	sleep(120);		/*wait for DIMENSION to drop line*/
}


awrite(dev, buf, ct, time)
int dev, ct;
unsigned time;
char *buf;
{
	register int n;

	alarm(time);
	n = write(dev, buf, ct);
	alarm(0);
	return(n);
}


aread(dev, buf, ct, time)
int dev, ct;
unsigned time;
char *buf;
{
	register int n;

	alarm(time);
	n = read(dev, buf, ct);
	alarm(0);
	return(n);
}


gertsto()
{
	gertsrc = TIMEOUT;
}
