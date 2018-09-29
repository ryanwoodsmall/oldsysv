static char errpr__[]="@(#)errpr.h	1.1";
#define ERRPR0(str)	fprintf(stderr,"!!  %s: str\n",nodename)
#define ERRPR1(str,par)	fprintf(stderr,"!!  %s: str\n",nodename,par);
#define ERRPR2(str,par1,par2)	fprintf(stderr,"!!  %s: str\n",nodename,par1,par2);
