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
/*	@(#)ctest.c	1.2	*/
#include "stdio.h"
char buf[256];
main()
{	int w,dp,sign;
	char *s;
	double x;
	for(;;)
	{
		scanf("%d %lf",&w,&x);
		if(feof(stdin)) exit(0);
		s=fcvt(x,w,&dp,&sign);
		strcpy(buf,s);
		printf("%d,%f:\t%d\t%s\n",w,x,dp,buf);
		s=ecvt(x,w,&dp,&sign);
		printf("\t\t%d\t%s\n",dp,s);
	}
}
