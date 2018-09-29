static char hpd_dev__[]="@(#)dev.h	1.1";
#define ARCCCW		printf("u")
#define ARCCW		printf("t")
#define DRAW		printf("q")
#define FIXDASHL	printf( "~Q")
#define LABCHSET	printf("~P")
#define LABMODEON	printf("~'")
#define LABSLANT	printf("~/")
#define LABSZ		printf("~%%")
#define MOVEPEN		printf("p")
#define PENSEL		printf("v")
#define PLOTON		printf("\033.(")
#define PLOTOFF		printf("\033.)")
#define ROTATE		printf("w")
#define SETGLIM		printf("~W")
#define SETGRIDSZ	printf("~S")

#define   BLKSIZE	1000
#define   SHKENAB	17
#define   SHKSTR	0
#define SETHSHK1	printf("\033.H%d;%d;%d:", BLKSIZE, SHKENAB, SHKSTR)

#define   OUTTERM	10
#define SETOMODE	printf("\033.M;;;%d:", OUTTERM)

#define TERMINATOR	printf("}")
#define VARDASHL	printf("~R")


#define ADJRAT		8
