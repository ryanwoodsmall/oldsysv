/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)mcs:mcs.c	1.4"
/*
 *	mcs - Manipulate the Comment Section of an a.out file
 *
 *	mcs will function with any number of sections in the object file.
 *	Should a .comment section previously exist, the section's relative
 *	position will be preserved if the file is updated. If the .comment
 *	section does not exist, the new section will be created after all
 *	the other sections in the object file.
 *
 *	Arguments are queued in the array "Action". Every file is processed
 *	using the "Action" array. This allows the user to specify any number
 *	(up to ACTSIZE) of arguments in any order, and guarentees that the same
 *	arguments will be performed in the same order on all files.
 */

#include <stdio.h>
#include <a.out.h>	/* pulls in filehdr.h, aouthdr.h, scnhdr.h, reloc.h,
			 *	linenum.h, syms.h
			 */
#include <ar.h>
#include <string.h>
#include <fcntl.h>
#include <sys/signal.h>

	/* file and section headers */
FILHDR	file_hdr;	/* file header */
struct	aouthdr *aout;	/* a.out optional header */
int	aoutsize;	/* sizeof aout */
SCNHDR	*scn_hdr;	/* an array of the section headers */
int	scn_count;	/* number of sections in the array */

	/* data for the comment section header */
char	*Sect_name = ".comment";   /* name of the section we are accessing */
long	comm_pos;	/* its position in the object file */
long	comm_size;	/* its size */
FILE	*comm_des;	/* file descriptor it resides on */
FILE	*ctemp_des;	/* temp file if opened */
FILE	*outdes;	/* output descriptor */
int	Exists;		/* if a comment section exists */
int	Changed;	/* if the file is to be rewritten */
int	Might_chg;	/* if the file might be changed (used for archives
			 * because archives are always copied out if a change
			 * type is being done. If nothing is really changed
			 * in the file, it is not copied back in
			 */
int	Nochg_shdr;	/* used to flag we are processing an aligned object
			 * file that we can't add or delete section headers,
			 * e.g., a 413 (paged) or a 443 (shared library).
			 */
#define	PAGED	0413
#define	SHLIB	0443

int	Sym_adjust;	/* amount the symbol table has to be adjusted by */
int	Offset;		/* how far you are into an archive file */

	/* current input file data */
char	*curfile;	/* name of the file (used in diagnostics) */
char	*curname;	/* name of the file/archive element */
FILE	*ifile;		/* descriptor */

	/* file pointers to define the remainder of the file */
long	txt_start;	/* where the data after the headers starts */
long	txt_size;	/* length of data up to the start of comment section */
long	comm_end;	/* start of data after the comment section */
long	sym_start;	/* start of the symbol table */
long	sym_eof;	/* end of the symbol table (and object file) */

/*
 *	The above variables provide the following layout:
 *
 *			----------------
 *			|  file header |
 *			|--------------|
 *			| section hdrs |
 *	txt_start ----> |--------------|
 *			|              |
 *	txt_size	|              |
 *			|              |
 *	comm_pos -----> |--------------|
 *			|   comment    |
 *	comm_size	|   section    |
 *			|              |
 *	comm_end -----> |--------------|
 *			|              |
 *	sym_start ----> |--------------|
 *			| symbol table |
 *			|--------------|
 *			|              |
 *	sym_eof ------> |______________|
 *
 */

	/* fields related to processing an archive file */
struct	ar_hdr arhead;	/* current archive header */
int	Is_archive;	/* if the current file being processed is an archive */
int	Elem_size;	/* size of the current archive element */
int	arsym_exist;	/* if archive symbols existed in the file */

	/* option processing */
#define ACTSIZE 20
int	Action[ACTSIZE];
int	actmax = 0;
#define ACT_DELETE	1
#define ACT_PRINT	2
#define ACT_PPRINT	3
#define ACT_COMPRESS	4

/*
 * data structures used when compressing the slist space....all strings are
 * stored in memory at that point and can be written out directly...before
 * compressing, strings had to be processed on disk to avoid consuming to
 * much main memory...once the strings are compressed, they shouldn't take
 * up that much space, so most of the work can be done directly in memory
 */
int	*hash_key;	/* an array of hash keys for the file being compressed */
int	hash_num;	/* current number of elements in hash_key */
int	hash_end;	/* last element in hash_key */
int	*hash_str;	/* an array of strings corresponding to the array
			   hash_key */

char	*strings;	/* actual string storage */
int	next_str;	/* current position in the string array */
int	str_size;	/* length of the string array */

	/* tempoary files */
char	*comm_temp = "/usr/tmp/MCSXXXXXX";	/* used in recreating
						 * the comment section data
						 */
char	*out_temp = "/usr/tmp/MCSXXXXXX";	/* used in recreating the
						 * changed file
						 */
int	Label_files;	/* if output lines should be labeled */

#define TRUE 1
#define FALSE 0

#define ROUND2(x)	(((x + 1) >> 1) << 1)
#define ROUND4(x)	(((x + 3) >> 2) << 2)

/* hardwire sizes of optional header so this works on cross-compiled files */
#define SAOUT1	28
#define SAOUT2	36

/* extern functions called */
extern char *malloc(), *realloc();
extern int (*signal())();
extern char *mktemp();
extern void exit();

main(argc, argv)
int	argc;
char	**argv;
{
	extern int	optind;
	extern char	*optarg;
	int	c;
	long	size;

	while ((c = getopt(argc, argv, "a:cdn:pP")) != EOF) {
		switch (c) {
		case 'a':
			queue(optarg);
			Might_chg++;
			break;
		case 'c':
			queue(ACT_COMPRESS);
			Might_chg++;
			break;
		case 'p':
			queue(ACT_PRINT);
			break;
		case 'P':
			queue(ACT_PPRINT);
			break;
		case 'd':
			queue(ACT_DELETE);
			Might_chg++;
			break;
		case 'n':
			Sect_name = optarg;
			if (strlen(Sect_name) <= 8) break;
			(void) fprintf(stderr,"%s: section name '%s' is too long\n",
				argv[0], Sect_name);
			exit(1);
		default:
			usage();
		}
	}

	if (optind == argc) {
		usage();
	}
	if (argc - optind > 1) Label_files++;
	for (; optind < argc; optind++) {
		curfile = argv[optind];
		curname = curfile;
		if ((ifile = fopen(curfile, "r")) == NULL) {
			(void) fprintf(stderr, "%s: can not open '%s'\n",
			    argv[0], argv[optind]);
			tidy_up(1);
		}
		init();
		if (fseek(ifile,0,2) != 0) read_error();
		if ((size = ftell(ifile)) == EOF || size == 0) bad_object();
		if (fseek(ifile,0,0) != 0) read_error();
		dofile(size);
		if(Changed) {
			move_back();
			if (arsym_exist)
				(void) fprintf(stderr,
				 "%s: Warning archive symbol table deleted from '%s'\n",
				 argv[0], curfile);
		}
		(void) fclose(ifile);
	}
	tidy_up(0);
	return(0);
}


init()
{
	Changed = FALSE;
	Offset = 0;
	Is_archive = 0;
	arsym_exist = 0;
	if(outdes != NULL) 
		if(fseek(outdes,0,0) != 0) read_error();
	arinit();
}

/* arinit is called to reinitialize between archive file elements */

arinit() {
	comm_pos = 0;
	comm_size = 0;
	comm_des = NULL;
	Exists = FALSE;
	Nochg_shdr = 0;
	txt_start = 0;
	txt_size = 0;
	comm_end = 0;
	sym_start = 0;
	sym_eof = 0;
	Sym_adjust = 0;
}


dofile(file_size)
long	file_size;
{
	int	act_index;	/* index into Action array */
	int	comm_sindex;	/* index of comment section hdr in scn_hdr array */
	int	i;	/* temp */

	sym_eof = file_size;
	if (file_size < sizeof(FILHDR)) bad_object();
	if (fread(&file_hdr, sizeof(file_hdr), 1, ifile) != 1) 
		read_error();
	if (!obj_magic(file_hdr.f_magic)) {
		if (!do_arfile(&file_hdr)) bad_object();
		return;
	}
	/* read the a.out header */
	if (file_hdr.f_opthdr != 0) {
		if (aoutsize != file_hdr.f_opthdr) {
			if (aoutsize == 0) {
				aoutsize = file_hdr.f_opthdr;
				aout = (struct aouthdr *) malloc(aoutsize);
			} else {
				aoutsize = file_hdr.f_opthdr;
				aout=(struct aouthdr *) realloc(aout,aoutsize);
			}
		}
		if (fread(aout, aoutsize, 1, ifile) != 1) 
			read_error();
		/* is this a file whose secion header can't be deleted */
		if ((aoutsize == SAOUT1 || aoutsize == SAOUT2) &&
		    (aout->magic == PAGED) || (aout->magic == SHLIB))
			Nochg_shdr = 1;
	}

	/* read in the section headers, get an extra entry
	 * in case we need to create a comment section
	 */
	scn_hdr = (SCNHDR *)malloc(sizeof(SCNHDR) * (file_hdr.f_nscns+1));
	comm_sindex = -1;
	for (i=0; i < file_hdr.f_nscns; i++) {
		if (fread(&scn_hdr[i], sizeof(SCNHDR), 1, ifile) != 1)
			read_error();
		if (strncmp(scn_hdr[i].s_name, Sect_name, 8) == 0) {
			comm_sindex = i;
			Exists = TRUE;
		}
	}
	scn_count = file_hdr.f_nscns;
	/* locate start of sections in case file needs to be rewritten */
	txt_start = FILHSZ + file_hdr.f_opthdr + file_hdr.f_nscns * SCNHSZ;
	sym_start = (file_hdr.f_symptr) ? file_hdr.f_symptr : sym_eof;

	if (Exists) {
		comm_pos = scn_hdr[comm_sindex].s_scnptr;
		comm_size = scn_hdr[comm_sindex].s_size;
		txt_size = comm_pos - txt_start;
		comm_end = comm_pos + comm_size;
		comm_des = ifile;
		/*
		 * adjust the file pointers to eliminate the comment
		 * section from the headers, as if it was deleted
		 */
		file_hdr.f_nscns--;
		readjust(comm_pos, -comm_size);
		if ( (i=SCNHSZ * (comm_sindex - 1)) < 0)
			i = 0;
		readjust((long)(FILHSZ+file_hdr.f_opthdr+i), -SCNHSZ);
		comm_pos += Offset;	/* adjust to an archive */
	} else {
		int	last;
		last = 0;
		/* determine last (highest address) section
		 * if needed, the comment section will be added after this
		 */
		for (i=0; i<=file_hdr.f_nscns; i++)
			if (scn_hdr[i].s_scnptr > scn_hdr[last].s_scnptr)
				last = i;

		if (scn_hdr[last].s_size != 0)
			txt_size = scn_hdr[last].s_size +
				scn_hdr[last].s_scnptr - txt_start;
		else
			txt_size = 0;
		comm_end = txt_size + txt_start;
	}
	/* process the file */
	for (act_index=0; act_index < actmax; act_index++)
		switch (Action[act_index]) {
			char	*new_str;
		case ACT_PRINT:
			if (Label_files)
				doprint(curname);
			else
				doprint(NULL);
			break;
		case ACT_PPRINT:
			doprint(curname);
			break;
		case ACT_DELETE:
			if ( !Exists ) 
				break;
			comm_size = 0;
			if (!Nochg_shdr)
				Exists = FALSE;
			Changed++;
			break;
		case ACT_COMPRESS:
			if ( !Exists ) 
				break;
			if (fseek(comm_des, comm_pos, 0) != 0) read_error();
			compress(comm_des, comm_size);
			if (comm_des == ifile || comm_des == NULL)
				newcomm_des();
			Changed++;
			if (fseek(comm_des, 0, 0) != 0) read_error();
			if (fwrite(strings, next_str, 1, comm_des) != 1)
				write_error();
			comm_size = next_str;
			break;
		default:
			/* we are adding a string to the comment section */
			if (Nochg_shdr && !Exists) {
				nocomment();
				break;
			}
			new_str = (char *) Action[act_index];
			if (comm_des == ifile || comm_des == NULL)
				newcomm_des();
			Changed++;
			if (fseek(comm_des, comm_size, 0) != 0)
				write_error();
			if (fputs(new_str, comm_des) == EOF)
				write_error();
			if (putc(0, comm_des) == EOF) write_error();
			comm_size += strlen(new_str) + 1;
			break;
		}
	/* at this point, the headers are setup to process the file */
	if (!Changed) 
		return;
	/* round up the comment section if need be */
	if (comm_size > 0) {
		if (comm_size != ROUND2(comm_size)) {
			if (putc(0, comm_des) == EOF) write_error();
			comm_size++;
		}
	}
	set_tidy_sigs();
	if (outdes == NULL) make_outdes();

	/* add in the comment section if it exists at this point */
	if (Exists) {
		long	comm_start = 0L;
		file_hdr.f_nscns++;
		if (comm_sindex == -1) {	/* brand new comment section */
			comm_sindex = scn_count; /* set up an array entry */
			comm_start = SCNHSZ;	/* add sect hdr size to start */
		}
		if ( (i=SCNHSZ * (comm_sindex - 1)) < 0)
			i = 0;
		readjust((long)(FILHSZ+file_hdr.f_opthdr+i), SCNHSZ);
		comm_start += txt_start + txt_size;
		readjust(comm_start, comm_size);
		(void)strncpy(scn_hdr[comm_sindex].s_name, Sect_name, 8);
		scn_hdr[comm_sindex].s_size = comm_size;
		scn_hdr[comm_sindex].s_scnptr = comm_start;
		scn_hdr[comm_sindex].s_flags = STYP_INFO;
	}
	/*
	 * if we are processing an archive file, we have to write the
	 * archive header at this point.
	 * we compute the amount the section has changed by, put it into
	 * the header and write it out.
	 */
	if (Is_archive) {
		char convbuf[20];
		(void) sprintf(convbuf,"%-10d",Elem_size + Sym_adjust);
		(void) strncpy(arhead.ar_size,convbuf,10);
		if (fwrite(&arhead, sizeof(arhead), 1,outdes) != 1)
			write_error();
	}
	if (fwrite(&file_hdr, sizeof(file_hdr), 1, outdes) != 1) write_error();
	if (file_hdr.f_opthdr != 0)
		if (fwrite(aout, file_hdr.f_opthdr, 1, outdes) != 1) write_error();
	for (i=0; i < file_hdr.f_nscns; i++) {
		if (!Exists && i == comm_sindex)  /* skip nonexistent comment section */
			continue;
		if (fwrite(&scn_hdr[i], sizeof(SCNHDR), 1, outdes) != 1)
			write_error();
	}
	/* read and write the data */
	if( fseek(ifile, Offset + txt_start, 0) != 0) read_error();
	fmove(outdes, ifile, txt_size);
	if (Exists) {
		if( fseek(comm_des, comm_pos, 0) != 0) read_error();
		fmove(outdes, comm_des, comm_size);
	}
	if (fseek(ifile, Offset + comm_end, 0) != 0) read_error();
	fmove(outdes, ifile, sym_start - comm_end);

	/* made it to the symbol table....read and write symbols
	 * looking for pointers that have to be adjusted
	 */
	if (file_hdr.f_symptr != 0) {
		if (fseek(ifile,Offset + sym_start,0) != 0) read_error();
		for (i=0; i < file_hdr.f_nsyms; i++) {
			SYMENT work;
			AUXENT aux;
			if (fread(&work, SYMESZ, 1, ifile) != 1) read_error();
			if (fwrite(&work, SYMESZ, 1, outdes) != 1)
				write_error();
			if (work.n_numaux == 0) 
				continue;
			if (fread(&aux, AUXESZ, 1, ifile) != 1) read_error();
			if (ISFCN(work.n_type)) 
				aux.x_sym.x_fcnary.x_fcn.x_lnnoptr += Sym_adjust;
			if (fwrite(&aux, AUXESZ, 1, outdes) != 1) write_error();
			i++;
		}
	}
	/* finally, move anything that is left over */
	fmove(outdes, ifile, (sym_eof - (sym_start + file_hdr.f_nsyms * SYMESZ)) );
}


readjust(daddr, size)
long	daddr;
long	size;
{
	int	i;
	/* do the file header */
	if (daddr <= file_hdr.f_symptr)
		file_hdr.f_symptr += size;
	for (i=0; i < scn_count; i++) {
		if (daddr <= scn_hdr[i].s_scnptr)
			scn_hdr[i].s_scnptr += size;
		if (daddr <= scn_hdr[i].s_relptr)
			scn_hdr[i].s_relptr += size;
		if (daddr <= scn_hdr[i].s_lnnoptr)
			scn_hdr[i].s_lnnoptr += size;
	}
	Sym_adjust += size;
}

queue(activity)
int	activity;
{
	if (actmax == ACTSIZE) {
		(void) fprintf(stderr, "mcs: greater than %d command options\n", ACTSIZE);
		tidy_up(1);
	}
	Action[actmax++] = activity;
}


newcomm_des() 
{
	set_tidy_sigs();
	if (ctemp_des == NULL) {
		int temp;
		temp = open(mktemp(comm_temp), O_RDWR|O_CREAT,0644);
		if (temp == -1) temp_error();
		ctemp_des = fdopen(temp, "r+");
		if (ctemp_des == NULL) temp_error();
	} else {
		if (fseek(ctemp_des,0,0) != 0) temp_error();
	}
	comm_des = ctemp_des;
	Exists = TRUE;
	if (fseek(ifile, comm_pos, 0) != 0) read_error();
	fmove(comm_des, ifile, comm_size);
	comm_pos = 0;
}


/*
   takes as input a file containing an s-list to be compressed and writes
   to the memory array "strings".
*/

compress(fd, fd_len)
FILE *fd;
long	fd_len;	/* number of bytes to read from fd */
{
	int	hash;
	int	i;

	if (hash_key == NULL) {
		hash_key = (int *)malloc(sizeof(int)*200);
		hash_end = 200;
		hash_str = (int *)malloc(sizeof(int)*200);
		str_size = 10000;
		strings = (char *)malloc(str_size);
	}
	hash_num = 0;
	next_str = 0;
	while (fd_len > 0) {
		int	pos;
		pos = getstr(fd);
		fd_len -= (next_str - pos);
		hash = dohash(pos + strings);
		for (i=0; i < hash_num; i++) {
			if (hash != hash_key[i]) 
				continue;
			if (strcmp(pos + strings, hash_str[i] + strings) == 0) 
				break;
		}
		if (i != hash_num) {
			next_str = pos;
			continue;
		}
		if (hash_num == hash_end) {
			hash_end *= 2;
			hash_key = (int *) realloc(hash_key, hash_end * sizeof(int));
			hash_str = (int *) realloc(hash_str, hash_end * sizeof(int));
		}
		hash_key[hash_num] = hash;
		hash_str[hash_num++] = pos;
	}
}


int	getstr(fd)
FILE *fd;
{
	register c;
	int	start;

	start = next_str;
	while ((c = getc(fd)) != NULL) {
		if (c == EOF) 
			read_error();
		if (next_str >= str_size) {
			str_size *= 2;
			strings = (char *)realloc(strings, str_size);
		}
		strings[next_str++] = c;
	}
	if (next_str >= str_size) {
		str_size *= 2;
		strings = (char *)realloc(strings, str_size);
	}
	strings[next_str++] = NULL;
	return(start);
}


#define HALFLONG 16
#define low(x)	(x&((1L<<HALFLONG)-1))
#define high(x)	(x>>HALFLONG)

/*
 * hashing has the effect of arranging line in 7-bit bytes and then
 * summing 1-s complement in 16-bit hunks 
 */

dohash(str)
char	*str;
{
	long	sum;
	register unsigned	shift;
	register t;
	sum = 1;
	for (shift=0; (t = *str++) != NULL; shift += 7) {
		sum += (long)t << (shift %= HALFLONG);
	}
	sum = low(sum) + high(sum);
	return((short)low(sum) + (short)high(sum));
}


fmove(out_des, in_des, count)
FILE *out_des;
FILE *in_des;
long	count;
{
	register bsize;
	char	buf[512];
	while (count > 0) {
		if (count > 512) 
			bsize = 512; 
		else 
			bsize = count;
		if (fread(buf, 1, bsize, in_des) != bsize) read_error();
		if (fwrite(buf, bsize, 1, out_des) != 1) write_error();
		count -= bsize;
	}
}

doprint(name)
char *name;
{
	if (Exists) {
		long	temp_size;

		temp_size = comm_size;
		if (fseek(comm_des, comm_pos, 0) != 0) read_error();
		if (name != NULL) {
			(void)fputs(name, stdout);
			if(puts(":") == EOF)
				write_error();
		}
		while (temp_size > 0L) {
			register c;
			while (temp_size-- > 0L && (c=getc(comm_des)) != NULL)
				if(putchar(c) == EOF)
					write_error();
			if(putchar('\n') == EOF)
				write_error();
		}
	}
}


read_error()
{
	(void) fprintf(stderr, "mcs: error reading '%s'\n",curfile);
	tidy_up(1);
}


usage() 
{
	(void) fprintf(stderr, "Usage: mcs -cdp -a 'string' files\n");
	tidy_up(1);
}


bad_object()
{
	(void) fprintf(stderr, "mcs: '%s' is not an object file\n",curname);
	tidy_up(1);
}

nocomment()
{
	(void) fprintf(stderr, "mcs: can not add '%s' section to '%s'\n",
		Sect_name,curname);
	tidy_up(1);
}

write_error()
{
	(void) fprintf(stderr, "mcs: write error processing '%s'\n",curfile);
	tidy_up(1);
}


temp_error()
{
	(void) fprintf(stderr, "mcs: error in processing a temporary file\n");
	tidy_up(1);
}

/* catch all "human" generated signals whilst we are moving a file back */

set_sigs()
{
	(void)signal(SIGHUP, SIG_IGN);
	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)signal(SIGTERM, SIG_IGN);
}

tidy_up(val)
int val;
{
	(void) unlink(comm_temp);
	(void) unlink(out_temp);
	exit(val);
}

set_tidy_sigs()
{
	 if (signal(SIGHUP,SIG_IGN) != SIG_IGN) (void)signal(SIGHUP, tidy_up);
	 if (signal(SIGINT,SIG_IGN) != SIG_IGN) (void)signal(SIGINT, tidy_up);
	 if (signal(SIGTERM,SIG_IGN) != SIG_IGN) (void)signal(SIGTERM, tidy_up);
}

int obj_magic(mnum)
unsigned short mnum;
{
	if (ISCOFF(mnum)) return(1);
	return(0);
}

int do_arfile(str)
char *str;
{
	char namebuf[256];
	char *name_ar;
	if(strncmp(str,ARMAG,SARMAG) != 0) return(0);
	if(fseek(ifile,SARMAG,0) != 0) read_error();
	(void) strcpy(namebuf,curname);
	(void) strcat(namebuf,"[]");
	curname = namebuf;
	name_ar = namebuf + strlen(namebuf) - 1;
	if (Might_chg) {
		Changed++;
		if (outdes == NULL) make_outdes();
		if(fputs(ARMAG,outdes) == EOF)
			write_error();
	}
	Is_archive++;
	while(fread(&arhead,sizeof(arhead),1,ifile) == 1) {
		int nam_size;
		nam_size = strchr(arhead.ar_name,'/') - arhead.ar_name;
		(void) sscanf(arhead.ar_size,"%10d",&Elem_size);
		if (nam_size == 0) {
			arsym_exist++;
			if(fseek(ifile,Elem_size,1) != 0) read_error();
			continue;
		}
		Offset=ftell(ifile);
		(void) strncpy(name_ar,arhead.ar_name,nam_size);
		name_ar[nam_size] = ']';
		name_ar[nam_size+1] = NULL;
		arinit();
		dofile((long)Elem_size);
		if (Might_chg && ROUND2(Elem_size + Sym_adjust) !=
			Elem_size + Sym_adjust) (void) putc('\n',outdes);
		if(fseek(ifile,ROUND2(Offset + Elem_size),0)) read_error();
	}
	return(1);
}

/* function moves a temp file back to the file it came from */
move_back()
{
	sym_eof = ftell(outdes);
	set_sigs();
	if(fclose(ifile) == EOF) read_error();
	if ((ifile = fopen(curfile, "w")) == NULL) {
		(void) fprintf(stderr, "mcs: Couldn't rewrite '%s'\n", curfile);
		return;
	}
	if(fseek(outdes,0,0)) read_error();
	fmove(ifile, outdes, sym_eof);
	set_tidy_sigs();
}

/* create the temp file to rebuild a new object in */
make_outdes()
{
	int temp;
	temp = open(mktemp(out_temp), O_RDWR|O_CREAT,0644);
	if (temp == -1) temp_error();
	outdes = fdopen(temp, "r+");
	if (outdes == NULL)  temp_error();
}
