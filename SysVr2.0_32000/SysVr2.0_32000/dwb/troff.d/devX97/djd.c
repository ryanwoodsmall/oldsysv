char djdxver[] = "@(#)djd.c	1.1";		/* SCCS */

# include <stdio.h>

djd_start(pde_name)
int	pde_name ;
{
	printf("+*DJDETEST* FORMAT=%s,BOF=250,FORMS=NONE,FONTINDEX=0,DATA=(1,150),;\n",pde_name) ;
	printf("+*DJDETEST* ASSIGN=(13,(2,6,10,14,18,22,26,30,34,38,42,46,50,54,58,\n") ;
	printf("+*DJDETEST* 62,66,70,74,78,82,86,90,94,98,102,106,110,114,118,122,\n") ;
	printf("+*DJDETEST* 126,130,134,138,142,146,150)),;\n") ;
	printf("+*DJDETEST* ASSIGN=(2,(3,7,11,15,19,23,27,31,35,39,43,47,51,55,59,\n") ;
	printf("+*DJDETEST* 63,67,71,75,79,83,87,91,95,99,103,107,111,115,119,123,\n") ;
	printf("+*DJDETEST* 127,131,135,139,143,147)),;\n") ;
	printf("+*DJDETEST* ASSIGN=(3,(4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,\n") ;
	printf("+*DJDETEST* 64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,\n") ;
	printf("+*DJDETEST* 128,132,136,140,144,148)),;\n") ;
	printf("+*DJDETEST* ASSIGN=(4,(5,21,37,53,69,85,101,117,133,149)),;\n") ;
	printf("+*DJDETEST* ASSIGN=(5,(9,25,41,57,73,89,105,121,137)),;\n") ;
	printf("+*DJDETEST* ASSIGN=(6,(13,29,45,61,77,93,109,125,141)),;\n") ;
	printf("+*DJDETEST* ASSIGN=(7,(17,81,145)),;\n") ;
	printf("+*DJDETEST* ASSIGN=(8,(33,97)),;\n") ;
	printf("+*DJDETEST* ASSIGN=(9,(49,113)),;\n") ;
	printf("+*DJDETEST* ASSIGN=(11,65),;\n") ;
	printf("+*DJDETEST* ASSIGN=(12,129),END,;\n") ;
}
djd_end()
{
	printf("+*DJDETEST* BOF=66,ASSIGN=(1,5),;\n") ;
	printf("+*DJDETEST* ASSIGN=(2,(6,34)),;\n") ;
	printf("+*DJDETEST* ASSIGN=(3,(8,24,44)),;\n") ;
	printf("+*DJDETEST* ASSIGN=(4,(7,20,35,49)),;\n") ;
	printf("+*DJDETEST* ASSIGN=(5,60),;\n") ;
	printf("+*DJDETEST* ASSIGN=(6,59),;\n") ;
	printf("+*DJDETEST* ASSIGN=(7,58),;\n") ;
	printf("+*DJDETEST* ASSIGN=(8,(12,19,26,33,41,48,56)),;\n") ;
	printf("+*DJDETEST* ASSIGN=(10,1),ASSIGN=(11,15),ASSIGN=(12,64),;\n") ;
	printf("+*DJDETEST* FORMAT=PDE1,FORMS=NONE,FONTINDEX=NONE,DATA=(0,150),BOF=66, END,;\n") ;
}
