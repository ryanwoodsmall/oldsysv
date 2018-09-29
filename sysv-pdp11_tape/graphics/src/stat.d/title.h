static char title__[]="@(#)title.h	1.1";
#define TITOUT0(fd,lab,max)	if(getlabel(fd,lab,max))\
				 printf("\1%s\n",lab); else
#define TITOUT1(fd,lab,max,node)	if(getlabel(fd,lab,max))\
				 printf("\1%s %s\n","node",lab); else
#define TITOUT2(fd,lab,max,node,par)	if(getlabel(fd,lab,max))\
					 printf("\1%s%g %s\n","node",par,lab); else
