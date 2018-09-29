static char SCCSID[]="@(#)levcal.c	1.1";
extern char *nodename;
#include "vtoc.h"
levcal(id)
	char *id; {
	int groups;
/*
 *  levcal computes level from the number of
 *  digit-groups in the id.
 */
	if(*id == '0') return(0);
	for(groups = 0; *id != '\0';groups++) {
	while(isdigit(*id)) id++;
	if	(*id != '\0')
		id++;
	}
	return(groups);
}

