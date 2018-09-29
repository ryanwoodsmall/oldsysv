/*	@(#)arcv.c	2.1	*/

#include <stdio.h>
#include <signal.h>

abort();        /* interrupt handler, defined below */
char *tempnam();/* C library routine for creating temp file name */
long time(); 	/* C library routine which gives you the time of day */

long genarc(); /* routine for converting archives and their member
		 files.  the routine returns the number of bytes
		 in the converted file.  the converted file is
		 assumed to be in tmpfil.  a returned value of
		 0 implies that something has gotten screwed up */

long gencpy(); /* routine for copying a file which does not (or can not)
		 need to be converted.  the tmpfil file is made a copy of
		 the infil input file.  */

/* input/output/temporary files */

char *infilname, *outfilname, *tmpfilname, *nm_newarc, *nm_member;

FILE	*infil, *outfil, *tmpfil, *newarc, *member;

char buffer[BUFSIZ]; /* file copy buffer */

/* archive file header format */

/*		COMMON ARCHIVE FORMAT


	ARCHIVE File Organization:

	_______________________________________________
	|__________ARCHIVE_HEADER_DATA________________|
	|					      |
	|	Archive Header	"ar_hdr"	      |
	|.............................................|
	|					      |
	|	Symbol Directory "ar_sym"	      |
	|					      |
	|_____________________________________________|
	|________ARCHIVE_FILE_MEMBER_1________________|
	|					      |
	|	Archive File Header "arf_hdr"	      |
	|.............................................|
	|					      |
	|	Member Contents (either a.out.h       |
	|			 format or text file) |
	|_____________________________________________|
	|					      |
	|	.		.		.     |
	|	.		.		.     |
	|	.		.		.     |
	|_____________________________________________|
	|________ARCHIVE_FILE_MEMBER_n________________|
	|					      |
	|	Archive File Header "arf_hdr"	      |
	|.............................................|
	|					      |
	|	Member Contents (either a.out.h       |
	|			 format or text file) |
	|_____________________________________________|              */


#define	ARMAG	"<ar>"
#define	SARMAG	4


struct	ar_hdr {			/* archive header */
	char	ar_magic[SARMAG];	/* magic number */
	char	ar_name[16];		/* archive name */
	char	ar_date[4];		/* date of last archive modification */
	char	ar_syms[4];		/* number of ar_sym entries */
};

struct	ar_sym {			/* archive symbol table entry */
	char	sym_name[8];		/* symbol name, recognized by ld */
	char	sym_ptr[4];		/* archive position of symbol */
};

struct	arf_hdr {			/* archive file member header */
	char	arf_name[16];		/* file member name */
	char	arf_date[4];		/* file member date */
	char	arf_uid[4];		/* file member user identification */
	char	arf_gid[4];		/* file member group identification */
	char	arf_mode[4];		/* file member mode */
	char	arf_size[4];		/* file member size */
};


/* old pdp11 format */

#define	OARMAG	0177545
struct	oar_hdr {
	char	ar_name[14];
	long	ar_date;
	char	ar_uid;
	char	ar_gid;
	int	ar_mode;
	long	ar_size;
};

/* main program for arcv 

   usage is 'arcv infile outfile'

   Reference: "UNIX 5.0 Machine Language Processors: VAX and 3B Processor
	       Requirements Specification", R.M. Klein, Memorandum for
               File Case 12143-116, October 7, 1981.
*/

main ( argc, argv )

int argc;
char * argv[];

{
	unsigned int in_magic; /* magic number of input file */

	long bytes_out;
	int bytes_in;


	/* trap for interrupts */

	if ((signal(SIGINT, SIG_IGN)) == SIG_DFL)
		signal(SIGINT, abort);
	if ((signal(SIGHUP, SIG_IGN)) == SIG_DFL)
		signal(SIGHUP, abort);
	if ((signal(SIGQUIT, SIG_IGN)) == SIG_DFL)
		signal(SIGQUIT, abort);
	if ((signal(SIGTERM, SIG_IGN)) == SIG_DFL)
		signal(SIGTERM, abort);


	if (argc != 3) { /* usage is 'arcv inargument outargument' */
		fprintf(stderr,"usage: arcv infile outfile\n");
		exit(1);
	}

	infilname = argv[1]; /* get input file name from command line */
	outfilname = argv[2];/* get output file name from command line*/

	if ((infil = fopen(infilname,"r")) == NULL) 
		/* can't get the input file */
		readerr(infilname);

	tmpfilname = tempnam("","arcv");
	if ((tmpfil = fopen(tmpfilname,"w")) == NULL)
		/* can't write onto the temporary file */
		writerr(tmpfilname);


	/* let's get down to business: are we dealing with an archive or
	   some other kind of file?  answer: look at the 'magic number' */

	if (fread(&in_magic,1,sizeof(in_magic),infil) != sizeof(in_magic))
		readerr(infilname);
	in_magic &= 0177777;
	fseek(infil,0L,0);

	switch (in_magic) {

		case OARMAG:			/* archive input */
			bytes_out = genarc(infil);
			break;

		default:			/* questionable input */
			bytes_out = gencpy(infil, infilname);
			fprintf(stderr,"arcv: warning, contents of file %s not modified\n",
				infilname);
			break;
		};


	/* copy converted output, which resides in the tmpfil file to the
	   outfil file */

	fclose(infil);
	if ((outfil = fopen(outfilname,"w")) == NULL) 
		/* can't write onto the output file */
		writerr(outfilname);

	/* always begin at the beginning */
	fclose(tmpfil);
	if ((tmpfil = fopen(tmpfilname,"r")) == NULL)
		/* can't read the temporary file */
		readerr(tmpfilname);

	while (bytes_out) {
		bytes_in = fread(buffer,1,BUFSIZ,tmpfil);
		bytes_out -= (long)fwrite(buffer,1,bytes_in,outfil);
	};

	/* we are all done, so clean up! */

	fclose(outfil);
	fclose(tmpfil);
	unlink(tmpfilname);
	exit(0);
}


/* interrupt handler: delete all the temp files, unlink the output
   and then bug out with an error code of 1 */

abort ()
{
	fclose(infil);		/* close all files */
	fclose(outfil);
	fclose(tmpfil);
	fclose(newarc);
	fclose(member);
	unlink(tmpfilname);	/* delete temp file and output */
	if (strcmp(infilname,outfilname))
		unlink(outfilname);
	unlink(nm_newarc);
	unlink(nm_member);
	exit(1);	/* bug out! */
}


readerr(filname) /* input read error, give the user a reason and bug out */

char *filname;

{
	fprintf(stderr,"arcv: input error on file %s ",filname);
	perror(" ");
	abort();
}


writerr(filname) /* output write error, give the user a reason and bug out */

char *filname;

{
	fprintf(stderr,"arcv: output error on file %s ",filname);
	perror(" ");
	abort();
}





long
genarc (arcfile) /* procedure to convert a pdp11 archive into
		   a 5.0+ archive format.  the converted file
		   is written onto the temp file tmpfil.  the
		   genarc function returns the number of bytes
		   in the arcved file tmpfil. */

FILE *arcfile;

{
	long arc_size; /* size of arcved archive file on output */
	int mem_size; /* size of an archive member file */
	short incr;

	struct ar_hdr  new_header;
	struct oar_hdr oldf_header;
	struct arf_hdr newf_header;

	/* set up temporary files */

	nm_newarc = tempnam("","arcv");
	nm_member = tempnam("","arcv");

	if ((newarc = fopen(nm_newarc,"w")) == NULL)
		writerr(nm_newarc);

	/* skip past the magic number in the input file: arcfile */
	fseek(arcfile,(long)sizeof(int),0);
		
	/* set up the new archive header */
	strncpy(new_header.ar_magic,ARMAG,SARMAG);
	strncpy(new_header.ar_name,infilname,sizeof(new_header.ar_name));
	sputl(time(NULL),new_header.ar_date);
	sputl(0L,new_header.ar_syms); /* recreate archive without symbols */


	if ((arc_size = (long)fwrite(&new_header,1,sizeof(new_header),newarc)) != 
		sizeof(new_header))
			writerr(nm_newarc);

	/* now process each archive member in turn */

	while (fread(&oldf_header,1,sizeof(oldf_header),arcfile) ==
			sizeof(oldf_header)) {

		/* if the member has a name of "__.SYMDEF" then we can
		   delete it from the new archive.  this special member
		   was used with earlier versions of random access
		   libraries.  it has no application or use in the new
		   archive format. */

		if (strcmp(oldf_header.ar_name,"__.SYMDEF") == 0) {
			fseek(arcfile,oldf_header.ar_size,1); /* skip */
			continue; /* and go on to the next archive member */
		};

		/* translate header data for each member */
		strncpy(newf_header.arf_name,oldf_header.ar_name,
				sizeof(oldf_header.ar_name));
		sputl(oldf_header.ar_date,newf_header.arf_date);
		sputl((long)oldf_header.ar_uid,newf_header.arf_uid);
		sputl((long)oldf_header.ar_gid,newf_header.arf_gid);
		sputl((long)oldf_header.ar_mode,newf_header.arf_mode);

		/* prepare the member for conversion */
		
		if ((member = fopen(nm_member,"w")) == NULL)
			writerr(nm_member);
		
		mem_size = oldf_header.ar_size;

		if (mem_size & 1) { /* ar expects members to be evenly sized */
			mem_size++;
			incr = 1;
		} else
			incr = 0;

		while (mem_size >= BUFSIZ) {
			if (fread(buffer,1,BUFSIZ,arcfile) != BUFSIZ)
				readerr(infilname);
			if (fwrite(buffer,1,BUFSIZ,member) != BUFSIZ)
				writerr(nm_member);
			mem_size -= BUFSIZ;
		};
		if (mem_size) {
			if ((long)fread(buffer,1,(int)mem_size,arcfile) != mem_size)
				readerr(infilname);
			if ((long)fwrite(buffer,1,(int)mem_size,member) != mem_size)
				writerr(nm_member);
		};
		
		/* now perform the actual conversion */

		fclose(member);
		if ((member = fopen(nm_member,"r")) == NULL)
			readerr(nm_member);

		mem_size = gencpy(member,newf_header.arf_name);

		fclose(member);

		/* now let's put the sucker back into the new archive */

		sputl((long)(mem_size-(long)incr),newf_header.arf_size); /* finish up */
		
		if (fwrite(&newf_header,1,sizeof(newf_header),newarc) !=
			sizeof(newf_header)) writerr(nm_newarc);

		arc_size += mem_size + sizeof(newf_header);

		/* put the new member into the new archive */

		fclose(tmpfil);
		if ((tmpfil = fopen(tmpfilname,"r")) == NULL)
			readerr(tmpfilname);
		while (mem_size = (long)fread(buffer,1,BUFSIZ,tmpfil))
			fwrite(buffer,1,(int)mem_size,newarc);
	};

	/* copy new archive file to tmpfil */

	fclose(newarc);
	fclose(tmpfil);
	if ((newarc = fopen(nm_newarc,"r")) == NULL)
		readerr(nm_newarc);
	if ((tmpfil = fopen(tmpfilname,"w")) == NULL)
		writerr(tmpfilname);

	while (mem_size = (long)fread(buffer,1,BUFSIZ,newarc))
		fwrite(buffer,1,(int)mem_size,tmpfil);

	/* time to clean up */

	fclose(newarc);
	unlink(nm_newarc);
	fclose(member);
	unlink(nm_member);

	return(arc_size);
}




long
gencpy(cpyfile, cpyname) /* routine for copying a file which does not 
		   	    need to be converted.  the tmpfil file is made
		    	    a copy of the infil input file.  */

FILE *cpyfile;
char *cpyname;

{
	long cpy_size;	
	int bytes_in;

	cpy_size = 0;
	
	fclose(tmpfil); /* start with a clean file */
	if ((tmpfil = fopen(tmpfilname,"w")) == NULL)
		writerr(tmpfilname);

	while (bytes_in = fread(buffer,1,BUFSIZ,cpyfile)) {
		fwrite(buffer,1,bytes_in,tmpfil);
		cpy_size = cpy_size + bytes_in;
		}

	return(cpy_size);
}

/*
 * The intent here is to provide a means to make the value of
 * bytes in an io-stream correspond to the value of the long
 * in the memory while doing the io a `long' at a time.
 * Files written and read in this way are machine-independent.
 *
 */

#include <values.h>

sputl(w, buffer)
long w;
register char *buffer;
{
	register int i = BITSPERBYTE * sizeof(long);

	while ((i -= BITSPERBYTE) >= 0)
		*buffer++ = (char) (w >> i);
}
