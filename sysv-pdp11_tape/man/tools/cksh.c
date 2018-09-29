#include	<stdio.h>

#define	MATCH	0

struct sections {
	int	value;
	char	title[30];
};

struct sections sec[] = {
	0,	".TH",
	1,	".SH NAME",
	2,	".SH SYNOPSIS",
	3,	".SH DESCRIPTION",
	4,	".SH EXAMPLES",
	5,	".SH FILES",
	6,	".SH SEE ALSO",
	6,	".SH \"SEE ALSO\"",
	7,	".SH DIAGNOSTICS",
	7,	".SH RETURN CODES",
	7,	".SH \"RETURN CODES\"",
	7,	".SH RETURN VALUE",
	7,	".SH \"RETURN VALUE\"",
	7,	".SH EXIT CODES",
	7,	".SH \"EXIT CODES\"",
	8,	".SH WARNINGS",
	9,	".SH BUGS",
	-1,	NULL
};

char	buf[256];

main(argc,argv)
	char *	argv[];
{
	register int	prev=0, foundit=0, idx;
	int		filearg;
	char *		strchr(), strncmp(), strcmp();
	FILE *		fptr;

	if(argc < 2) {
		fprintf(stderr, "Usage: %s manpage ...\n", argv[0]);
		exit(1);
	}
	for (filearg=1; filearg < argc; ++filearg) {

		if((fptr=fopen(argv[filearg], "r")) == NULL) {
			fprintf(stderr, "%s: cannot open %s\n",
				argv[0], argv[filearg]);
			continue;
		}

		prev = 0;
		while(fgets(buf, sizeof(buf), fptr) != NULL) {

			if(strncmp(buf, ".SH ", 4) != MATCH)
				continue;

			foundit = 0;
			for(idx=0; sec[idx].value != EOF; ++idx) {
				if(strncmp(buf,sec[idx].title,strlen(buf)-1) == MATCH) {
					foundit++;
					break;
				}
			}
			if(foundit) {
				if(sec[idx].value > prev) {
					prev = sec[idx].value;
					continue;
				}
				else {
					buf[strchr(buf, '\n') - buf] = NULL;
					fprintf(stdout,"%s: %s out of place\n",
						argv[filearg], buf);
				}
			}
			else {
				fprintf(stdout, "%s: Unknown section %s",
					argv[filearg], buf);
				continue;
			}
		}
		fclose(fptr);
	}
}
/*	@(#)cksh.c	5.2 of 5/19/82	*/
