/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
#
#include "uucp.h"
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include "uust.h"
struct us_rsf uu;
FILE *pdf;
delete(no)
{
	char	f[20];
	char	*name;
	register int n;
	int	v;

	v = USR_QUEUED;
	fseek(pdf, 0L, 0);
	while (gnamef(pdf, f)) {
		if(strlen(f) <= 4)
			continue;
		if(f[1] != '.')
			continue;
		name = f + strlen(f) - 4;
		n = atoi(name);
		if (n == no) {
DEBUG(5, "KILL %s\n",f);
			if (f[0] == CMDPRE){
				v = USR_KCOMP;
			}
			if(v == USR_QUEUED)
				v = USR_KINC;
			if(unlink(f) == -1){
				fprintf(stderr,"Can't unlink %s\n",f);
			}
		}
	}
	return(v);
}
int	sigz;
int	getit();
kill(jn)
{
	FILE *fp;
	register int n;
	int	fnd;
	char *name, file[100];
	int	ret, val;
	char	b[BUFSIZ];
	char	s[200];
	register i;
	int	sqit, shup, sint;
	long pos;
	sigz = 0;
	fnd = 0;
	sigz= 1;


	{
		register int i;
		for(i=0; i<=15; i++) {
			if (ulockf(LCKRSTAT, 15) != FAIL) break;
			sleep(1);
		}
		if (i > 15) {
			fprintf(stderr, "cannot lock %s\n", LCKRSTAT);
			return(FAIL);
		}
	}
	shup = (int)signal(SIGHUP, getit);
	sint = (int)signal(SIGINT, getit);
	sqit = (int)signal(SIGQUIT, getit);
	if ((fp=fopen(R_stat, "r+")) == NULL) {
		fprintf(stderr, "cannot open for reading:%s\n", R_stat);
		rmlock(LCKRSTAT);
		sigz = 0;
		signal(SIGHUP, shup);
		signal(SIGINT, sint);
		signal(SIGQUIT, sqit);
		return(FAIL);
	}
	/*
	 * delete the command file from spool dir.
	 * spool directory
	 */
	if ((pdf=fopen(Spool,"r"))==NULL) {
		perror(Spool);
		rmlock(LCKRSTAT);
		fclose(fp);
		sigz = 0;
		signal(SIGHUP, shup);
		signal(SIGINT, sint);
		signal(SIGQUIT, sqit);
		return(FAIL);
	}
	while(fread(&uu, sizeof(uu), 1 , fp) != NULL) {
	if (uu.jobn==jn)
		if  ((strcmp(uu.user,User)==SAME)
	     		|| (strcmp(User,UUCP)==SAME) || (getuid()==0))  {
				if((uu.ustat&(USR_KINC|USR_KCOMP)) == 0){
				DEBUG(5, "Job %d is deleted\n", jn);
				if((uu.ustat&USR_COMP) == 0){
					val = delete(jn);
				if(val == USR_QUEUED){
					val = USR_KINC;
					fprintf(stderr,"Job %hd not found\n",uu.jobn);
				}
/*
				fnd++;
 */
				uu.ustat = val;
				uu.stime = time((long*)0);
				DEBUG(5, "fp: %o\n",fp);
				pos = ftell(fp);
				fseek(fp,pos-(long)(sizeof(uu)), 0);
				fwrite(&uu,sizeof(uu), 1, fp);
				break;
			}else{
				fprintf(stderr,"job %hd already complete\n",uu.jobn);
				break;
			}
		}
	} else {
		fprintf(stderr,"Permission Denied\n");
		return(FAIL);
		}
	}
/*
	if(fnd == 0){
		val = delete(jn);
		if(val == USR_QUEUED)
			fprintf(stderr,"Job %hd not found\n",jn);
	}
*/
	fclose(pdf);
	fclose(fp);
	rmlock(LCKRSTAT);
	sigz = 0;
	signal(SIGHUP, shup);
	signal(SIGINT, sint);
	signal(SIGQUIT, sqit);
	return(0);

}
getit()
{
	rmlock(CNULL);
}
