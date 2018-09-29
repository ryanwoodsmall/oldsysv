/*	vpmtest.c 1.3 of 2/10/82
	@(#)vpmtest.c	1.3
 */

#include "sys/types.h"
#include "stdio.h"
#include "ctype.h"
#include "sys/stat.h"
#include "sys/file.h"
#include "sys/csi.h"
#include "sys/csikmc.h"
#include "sys/vpmt.h"

static char Sccsid[] = "@(#)vpmtest.c	1.3";

#define	NO	0
#define	YES	1

int fd, retcode, opts, c, vpmopts;
int exitcode, pid, status;
char fsrloc[] = "/etc/vpmfsr";
char count[] = "10000";
char waitmsg[] = "Answer 'y' when ready: ";
int tflg = 0;
int errflg = 0;
int kflg = 0;
int nflg = 0;
char  *kmc, *top;
char *linkno, *lineno, *channel;
char *pktsiz;

main(argc,argv)
char *argv[];
{
	extern int optind;
	extern char *optarg;

	while ((c = getopt(argc, argv, "t:k:n:")) != EOF)
		switch (c) {
		case 'n':
			if (nflg)
				errflg++;
			else {
				nflg++;
				lineno = optarg;
			}
			break;
		case 'k':
			if (kflg)
				errflg++;
			else {
				kflg++;
				kmc = optarg;
			}
			break;
		case 't':
			if (tflg)
				errflg++;
			else {
				tflg++;
				top = optarg;
			}
			break;
		case '?':
			errflg++;
			break;
		}
	opts = argc - optind;
	if (opts != 0) {
		fprintf(stderr, "Arg count incorrect\n");
		++errflg;
	}
	if(!(nflg&kflg&tflg))
		++errflg;
	if(errflg)
		err();
	printf("top name: %s\n",top);
	printf("kmc name: %s\n",kmc);
	printf("line number: %s\n",lineno);
	printf("Is the above information correct?  ");
	if(reply("Answer 'y' to continue: ")==NO)
		exit(0);
	/*
 * Run internal loopback test
 */
	retcode = test(HWLOOP|ADRSWTCH);
	if(retcode == 0){
		fprintf(stderr, "Internal line unit loopback succeeded.\n");
	} else{
		fprintf(stderr, "Internal line unit loopback failed.\n");
		exit(14);
	}
	/*
 * Run local modem loopback test
 */
	printf("Place local modem in local loopback (analog test) mode.\n");
	if(reply(waitmsg)==NO)
		exit(0);
	retcode = test(ADRSWTCH);
	if(retcode == 0){
		fprintf(stderr, "Local modem loopback succeeded.\n");
	} else{
		fprintf(stderr, "Local modem loopback failed.\n");
		exit(14);
	}
	/*
 * Run far-end modem loopback test
 */
	printf("Place local modem in normal mode.\n");
	printf("Place far-end modem in digital loopback (digital test) mode.\n");
	if(reply(waitmsg)==NO)
		exit(0);
	retcode = test(ADRSWTCH);
	if(retcode == 0){
		fprintf(stderr, "Remote modem loopback succeeded.\n");
	} else{
		fprintf(stderr, "Remote modem loopback failed.\n");
		exit(14);
	}
}

reply(s)
char *s;
{
	char inbuf[80];

	printf("%s", s);
	if(getline(stdin,inbuf,sizeof(inbuf)) == EOF)
		exit(0);
	printf("\n");
	if(inbuf[0] == 'y' || inbuf[0] == 'Y')
		return(YES);
	else
		return(NO);
}

getline(fp,loc,maxlen)
FILE *fp;
char *loc;
{
	register n;
	register char *p, *lastloc;

	p = loc;
	lastloc = &p[maxlen-1];
	while((n = getc(fp)) != '\n') {
		if(n == EOF)
			return(EOF);
		if(!isspace(n) && p < lastloc)
			*p++ = n;
	}
	*p = 0;
	return(p - loc);
}

test(vpmopts)
int vpmopts;
{
	/*
 * Connect the top to the kmc
 */
	setdev(top,mdevno(kmc)|((atoi(lineno)<<5)&0340),0,vpmopts);
	printf("Data structures connected\n");
	printf("vpm options = %o\n", vpmopts);
	/*
 * Execute fsr
 */
	if((pid = fork()) < 0){
		fprintf(stderr, "Can't fork\n");
		unsetdev(top);
		exit(8);
	} else if(pid == 0){
		fprintf(stderr, "%s %s %s\n", fsrloc, top, count);
		execlp(fsrloc, fsrloc, top, count, 0);
		printf("can't exec %s\n",fsrloc);
		exit(13);
	} else{
		wait(&status);
		printf("status = %o\n", status);
		if(status){
			printf("fsr terminated abnormally\n");
		}
		exitcode = (status>>8)&0xff;
		printf("exit code = %d\n", exitcode);
	}
	unsetdev(top);
	return(exitcode);
}
mdevno(file)
char *file;
{
	struct stat *bp;
	struct stat buf;

	bp = &buf;
	if (stat(file, bp)) {
		perror("Stat failed");
		exit(1);
	}
	if ((bp->st_mode & 0170000) != 0020000) {
		fprintf(stderr,"%s ", file);
		perror("not character special file");
		exit(1);
	}
	return(bp->st_rdev&0377);
}
setdev(file, mdno, bottom, options)
char *file;
{
	int fd;

	if ((fd = open(file, FNDELAY)) <0) {
		perror("Open failed");
		exit(1);
	}
	if (ioctl(fd, VPMSDEV, mdno) < 0) {
		perror("VPMSDEV ioctl failed");
		exit(1);
	}
	if (ioctl(fd, VPMPCDOPTS, options) < 0) {
		perror("VPMPCDOPTS ioctl failed\n");
		exit(1);
	}
	close(fd);
}
unsetdev(file)
char *file;
{
	int fd;

	if ((fd = open(file, FNDELAY)) <0) {
		perror("Open failed");
		exit(1);
	}
	if (ioctl(fd, VPMDETACH, 0) < 0) {
		perror("VPMDETACH ioctl failed");
		exit(1);
	}
	close(fd);
}
err()
{
	fprintf(stderr,"usage: vpmtest  -k kmcname -n lineno -t topname\n");
	exit(9);
}
