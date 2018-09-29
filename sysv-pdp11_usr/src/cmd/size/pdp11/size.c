/*	@(#)size.c	2.1	*/
#include	<stdio.h>
#include 	<a.out.h>

/*
	size -- determine object size

*/

main(argc, argv)
char **argv;
{
	struct	exec	buf;
	long	sum;
	int	gorp,ck;
	FILE	*f;

	if (argc==1) {
		*argv = "a.out";
		argc++;
		--argv;
	}
	gorp = argc;
	while(--argc) {
		++argv;
		if ((f = fopen(*argv, "r"))==NULL) {
			fprintf(stderr,"size: %s cannot open", *argv);
			perror(":");
			continue;
		}
		ck = fread((char *)&buf, sizeof(buf), 1, f);
		if(BADMAG(buf) || (ck == 0)) {
			fprintf(stderr,"size: %s not an object file\n", *argv);
			fclose(f);
			continue;
		}
		if (gorp>2)
			printf("%s: ", *argv);
		sum = buf.a_text;
#ifdef	pdp11
		sum += ((long) buf.a_hitext << 16);
#endif
		printf("%ld+%u+%u = ", sum, buf.a_data, buf.a_bss);
		sum += (long) buf.a_data + (long) buf.a_bss;
		printf("%ldb = 0%lob\n", sum, sum);
		fclose(f);
	}
}
