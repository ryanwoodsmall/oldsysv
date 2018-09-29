/* @(#)kunb.c	1.2 */
#include <stdio.h>
#include <a.out.h>

#ifdef vax
struct filehdr fhdr;
struct aouthdr hdr;
#define a_magic	magic
#define	a_text	tsize
#endif
#ifdef pdp11
struct exec hdr;
#endif

char *opn[16] = {
	"add",	"addc",	"subc",	"inc",
	"adc",	"asl",	"aslc",	"dec",
	"mov",	"mov",	"orn",	"and",
	"or",	"xor",	"sub",	"addn"
};

char *srcr[32] = {
	"idl",	"idh",	"odl",	"odh",
	"ial",	"iah",	"oal",	"oah",
	"lur0",	"lur1",	"lur2",	"lur3",
	"lur4",	"lur5",	"lur6",	"lur7",
	"csr0",	"csr1",	"csr2",	"csr3",
	"csr4",	"csr5",	"csr6",	"csr7",
	"npr",	"nprx",
	"pc", "%pc", "mar", "%mar",
	"NEX6", "NEX7",
};

char *dstr[32] = {
	"idl",	"idh",	"odl",	"odh",
	"ial",	"iah",	"oal",	"oah",
	"lur0",	"lur1",	"lur2",	"lur3",
	"lur4",	"lur5",	"lur6",	"lur7",
	"csr0",	"csr1",	"csr2",	"csr3",
	"csr4",	"csr5",	"csr6",	"csr7",
	"npr",	"nprx",
	"NEX10", "pcr",
	"NEX12", "NEX13", "NEX14", "NEX15",
};

char *brn[8] = {
	"jmp",	"br",	"brc",	"brz",
	"br0",	"br1",	"br4",	"br7"
};

char *marn[4] = {
	"-", "%mar", "mar", "mar++"
};

#define	NKMCI	4096
short	kbuf[NKMCI];
short	kbit[NKMCI/16];

main(argc, argv)
char **argv;
{
	register ins, i;
	int	last, areg, breg, dst, mar, op;
	int	rflag;

	while (argc > 1) {
		if (argv[1][0]=='-' && argv[1][1]=='o') {
			if (argc<3) {
				fprintf(stderr, "missing -o file\n");
				exit(1);
			}
			if (freopen(argv[2], "w", stdout)==NULL) {
				fprintf(stderr, "cannot open output %s\n",argv[2]);
				exit(1);
			}
			argc -= 2;
			argv += 2;
			continue;
		}
		if (freopen(argv[1], "r", stdin)==NULL) {
			fprintf(stderr, "cannot open input %s\n",argv[1]);
			exit(1);
		}
		argc--;
		argv++;
	}
#ifdef vax
	read(0, &fhdr, sizeof fhdr);
#endif
	if (read(0, &hdr, sizeof hdr) <= 0) {
		fprintf(stderr, "No input data\n");
		exit(1);
	}
	last = hdr.a_text;
	if (hdr.a_magic==0410) {
		lseek(0, (long)(sizeof hdr), 0);
#ifdef vax
		lseek(0, (long)(sizeof(struct filehdr) +
			2*sizeof(struct scnhdr)), 1);
#endif
	} else if(hdr.a_magic==0440) {
		lseek(0, (long)(sizeof hdr)+48L, 0);
#ifdef vax
		lseek(0, (long)(sizeof(struct filehdr) +
			2*sizeof(struct scnhdr)), 1);
#endif
	} else {
		lseek(0, 0L, 0);
		last = sizeof kbuf;
	}
	last = read(0, kbuf, last)/sizeof(short);
	if (last <= 0) {
		fprintf(stderr, "No input data\n");
		exit(1);
	}
	for (i=0; i<last; i++) {
		if ((kbuf[i]&0160000)==0100000) {
			ins = (kbuf[i]&0377)+((kbuf[i]&014000)>>3)+(i&~01777);
			kbit[ins>>4] |= (1<<(ins&017));
		}
		if ((kbuf[i]&0163400)==0100400 &&
		    (kbuf[i+1]&0163400)!=0100400)
			kbit[(i+1)>>4] |= 1<<((i+1)&017);
		if (kbuf[i])
			op = i;
	}
	last = op+1;
	for(i=0; i<last; i++) {
		ins = kbuf[i];
		areg = ins&017;
		breg = (ins&0360)>>4;
		dst = (ins&03400)>>8;
		mar = (ins&014000)>>11;
		op = (ins>>13)&07;
		rflag = 0;
		if ((i&0377)==0)
			printf("P%d:\n",i>>8);
		if (kbit[i>>4]&(1<<(i&017)))
			printf("L%o:\t\t/*P%oL%o*/\n",i,(i>>8)&017,i&0377);
		putchar('\t');
		switch(op) {
		case 0:
			if (ins==0) {
				while(kbuf[i+1]==0) i++;
				printf(".org\t%d",i+1);
				break;
			}
			printf("mov\t");
			if (ins&0377)
				printf("0%o",ins&0377);
			else	printf("0");
			goto dest;
		case 1:
		case 5:
			if (op==5)
				breg += 16;
			printf("mov\t%s",srcr[breg]);
			goto dest;
		case 2:
		case 3:
			printf("%s\t",opn[breg]);
			switch(breg) {
			case 9:
				if (op==3)
					printf("brg");
				else	printf("mem");
				break;
			default:
				rflag++;
			case 8:
				if (breg<3 || breg>8)
					if(op==3)
						printf("brg,");
					else	printf("mem,");
				if(dst==2||dst==4)
					printf("r0");
				else	printf("r%d",areg);
			}
		dest:
			switch(dst) {
			case 0:
				printf(",%s",marn[mar]);
				break;
			case 1:
				printf(",brg");
				goto destmar;
			case 2:
				printf(",%s",dstr[areg+16]);
				goto destmar;
			case 3:
				printf(",brg>>");
				goto destmar;
			case 4:
				printf(",%s",dstr[areg]);
				goto destmar;
			case 5:
				printf(",mem");
				goto destmar;
			case 6:
				if (rflag && mar==0)
					break;
				printf(",r%d",areg);
				goto destmar;
			case 7:
				printf(",r%d|brg",areg);
			destmar:
				if (mar)
					printf("|%s",marn[mar]);
			}
			break;
		case 4:
			printf("%s\t",brn[dst]);
			if (dst==0) {
				printf("%o",ins&0377);
				break;
			}
			dst = (ins&0377)+(mar<<8)+(i&~01777);
			printf("%c%o",(dst<last)?'L':'U',dst);
			break;
		case 6:
		case 7:
			printf("%s\t(", brn[dst]);
			switch(breg) {
			case 8:
				printf("r%d",areg);
				break;
			case 9:
				if (op==6)
					printf("mem");
				else	printf("brg");
				break;
			default:
				printf("%s,",opn[breg]);
				if (breg<3 || breg>7)
					if (op==6)
						printf("mem,");
					else	printf("brg,");
				printf("r%d",areg);
			}
			putchar(')');
			if (mar)
				printf(",p%d",mar);
		}
		putchar('\n');
	}
	printf("E%o:\n",last);
}
