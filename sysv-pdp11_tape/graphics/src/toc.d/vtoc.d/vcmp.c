static char SCCSID[]="@(#)vcmp.c	1.1";
extern char *nodename;
#include	"vtoc.h"
vcmp(nd1,nd2)
	struct node **nd1,**nd2;
{
	return(idcmp((*nd1)->id,(*nd2)->id));
}
