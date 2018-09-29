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
#include "/usr/include/stdio.h"

#include <a.out.h>

#include "Ga.out.h"

#define TEXT	0
#define DATA	1
#define BSS	2

struct filehdr f;
struct aouthdr a;
struct scnhdr s[3];

struct exec e;

main(argc, argv)
char **argv;
{
	int tsize;

	read(0, &f, sizeof(f));
	if (f.f_opthdr != sizeof(a)) {
		fprintf(stderr, "no opt hdr\n");
		exit(2);
	}
	read(0, &a, sizeof(a));
	if (a.magic != 0413) {
		fprintf(stderr, "Bad magic\n");
		exit(1);
	}
	read(0, s, sizeof(s));
	e.a_magic = ZMAGIC;
	tsize = (s[TEXT].s_size + 0x1ff) & ~0x1ff;
	e.a_text = tsize;
	e.a_data = s[DATA].s_size;
	e.a_bss = s[BSS].s_size;
	e.a_entry = a.entry;
	e.a_mod = 0x20;
	e.a_text_addr = s[TEXT].s_paddr;
	e.a_dat_addr = s[DATA].s_paddr;

	write(1, &e, sizeof(e));
	lseek(1, 1024, 0);
	copyb(s[TEXT].s_scnptr, tsize);
	copyb(s[DATA].s_scnptr, s[DATA].s_size);

	exit(0);
}
copyb(off, n)
{
	char buf[512];
	int i, j, k;

	lseek(0, off, 0);
	while (n) {
		i = n>512 ? 512 : n;
		read(0, buf, i);
		write(1, buf, i);
		n -= i;
	}
}
