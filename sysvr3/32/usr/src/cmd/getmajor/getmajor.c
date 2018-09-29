/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)getmajor:getmajor.c	1.2.1.4"

#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/edt.h>
#include <sys/libxedt.h>
#include <sys/sys3b.h>

extern int errno;
extern struct libxedt *readedt();

        /***************************************************************/
        /*********   It reads the       		     ***********/
        /*********   Equipped Device Table (EDT)             ***********/
	/*********   and returns slot numbers for	     ***********/
	/*********   the requested board name or code.       ***********/
        /***************************************************************/
main (argc,argv)
int	argc;
char	*argv[];
{
	int i;
	int first_time;
	int base, numb_edt;
     
#ifdef u3b2
	int name_size = E_NAMLEN;
#else
	struct myedt {
		struct edts myedt;
		};
	int name_size = 9;
	char temp[10];
	base = 0;
	numb_edt = 1;

#endif

     	struct libxedt *edtptr = NULL;
     	char *iname = NULL;
     
     	unsigned code = 0;    /* should be unsigned since it is in edt.h/libxedt.h*/
     	int found = 0;
	first_time = 1;

     	if(argc < 2 || argc > 2)
		{
#ifdef u3b2
		printf("Usage: getmajor code | name\n");
#else
		printf("Usage: getmajor name\n");
#endif
		goto error_out;
     	}
     	iname = argv[1];
	if(isalpha((int)iname[0]) != 0)
		{
#ifdef u3b15
		if(islower((int)iname[0]) != 0)
			{
			for(i = 0; i < strlen(iname); ++i)
				temp[i] = toupper(iname[i]);
			temp[i] = '\0';
			iname = temp;
		}
#endif

	}
	else
		code = strtol(iname, (char **)NULL, 0);


/* Read the combined main and extended edts */
if((edtptr = readedt()) == NULL) {
	printf("getmajor: read of EDT failed\n");
	goto error_out;
}

#ifdef u3b2
     	for(i = 0; i < edtptr->edtentries; ++i)
		{
		if(code != NULL)
			{
			if(edtptr->edt[i].code != code)
				continue;
		}
		else
			if(strncmp(edtptr->edt[i].dev_name, iname, name_size) != 0)
				continue;
	
		if(first_time == 0)
			printf(" ");
	
		printf("%d", edtptr->edt[i].major);
		found = 1;
		first_time = 0;
     	}

#else
	for(i = 0; i < edtptr->myedt.number; ++i)
		{
		if(strcmp(edtptr->myedt.edtx[i].dev_name, "LBE") == 0)
			++numb_edt;
	}

	while(numb_edt--)
		{
		for(i = 0; i < edtptr->myedt.number; ++i)
			{
			if(strncmp(edtptr->myedt.edtx[i].dev_name, iname, name_size) != 0)
				continue;
			if(first_time == 0)
				printf(" ");

			printf("%d",(edtptr->myedt.edtx[i].bd_code & 0x0f) + base);
			found = 1;
			first_time = 0;
		}

		base += 32;
		edtptr = (struct myedt *)(int)((int)edtptr + (int)edtptr->myedt.number * sizeof(struct edt) + sizeof(int));
	}

#endif

error_out:
     	if(found == 0)
		printf("\0");
     
     	printf("\n");

     	if(edtptr != NULL)
		free(edtptr);

     	if(found == 0)
		exit(1);

     	exit(0);

}
