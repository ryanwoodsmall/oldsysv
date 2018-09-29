
/*
 * COPYTAPE.C
 *
 * This program duplicates magnetic tapes, preserving the
 * blocking structure and placement of tape marks.
 *
 * This program was updated at
 *
 *	U.S. Army Artificial Intelligence Center
 *	HQDA (Attn: DACS-DMA)
 *	Pentagon
 *	Washington, DC  20310-0200
 *
 *	Phone: (202) 694-6900
 *
 **************************************************
 *
 *	THIS PROGRAM IS IN THE PUBLIC DOMAIN
 *
 **************************************************
 *
 * August 1997		John Holden
 *		Add -x option to extract files
 *
 * April 1985		David S. Hayes
 *		Original Version.
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/file.h>

extern int      errno;

#define ERR		-1

#define BUFLEN		32768	/* max tape block size */
#define TAPE_MARK	-100	/* return record length if we read a
				 * tape mark */
#define END_OF_TAPE	-101	/* 2 consecutive tape marks */
#define FORMAT_ERROR	-102	/* data file munged */

int             totape = 0,	/* treat destination as a tape drive */
                fromtape = 0;	/* treat source as a tape drive */

int		inp_fd, out_fd;	/* input/output files */

int             verbose = 0;	/* tell what we're up to */
int		extract = 0;	/* extract files */

int		debug;		/* debug code */

char		*file_proto;	/* file prototype for extract */
int		file_numb = 1;	/* file number */
int		ex_fd = 0;	/* extract fd */
char		filename[256];

char                tapebuf[BUFLEN];

main(argc, argv)
    int             argc;
    char           *argv[];
{
    int             len;	/* number of bytes in record */
    int             skip = 0;	/* number of files to skip before
				 * copying */
    unsigned int    limit = 0xffffffff;
    int             i;
    struct mtget    status;

    	for (i = 1; i < argc && (argv[i][0] == '-' && argv[i][1] != '\0'); i++)
	{
		switch (argv[i][1])
		{
	  		case 's':		/* skip option */
	    			skip = atoi(&argv[i][2]);
	    			break;

	  		case 'l':
	    			limit = atoi(&argv[i][2]);
	    			break;

	  		case 'd':		/* debug flag */
	    			debug++;
	    			break;

	  		case 'f':		/* from tape option */
	    			fromtape = 1;
	    			break;

	  		case 't':		/* to tape option */
	    			totape = 1;
	    			break;

	  		case 'v':		/* be wordy */
	    			verbose = 1;
	    			break;

			case 'x':		/* extract files */
				extract++;
				break;

	  		default:
	    			fprintf(stderr, "usage: copytape [-f] [-t] [-lnn] [-snn] [-v] from to\n");
	    			exit(-1);
		}
    	}
    	if(i + 2 != argc)
    	{
		fprintf(stderr, "argc ?\n");
		exit(1);
    	}
/*
**	test for standard input with filename of '-'
*/
	if(strcmp(argv[i], "-") == NULL)
	{
		inp_fd = 0;
		i++;
	}
	else if((inp_fd = open(argv[i++], 0)) == ERR)
	{
	    perror("copytape: input open failed");
	    exit(-1);
	}
/*
**	test for standard output with filename of '-'
*/
	if(extract)
	{
		file_proto = argv[i];
		out_fd = 0;
	}
	else if(strcmp(argv[i], "-") == NULL)
		out_fd = 1;
	else if((out_fd = creat(argv[i], 0600)) == ERR)
	{
	    	perror("copytape: output open failed");
	    	exit(-1);
	}
/*
**	Determine if source and/or destination is a tape device. Try to
**	issue a magtape ioctl to it.  If it doesn't error, then it was a
**	magtape. 
*/

    	errno = 0;
    	ioctl(inp_fd, MTIOCGET, &status);
    	fromtape |= errno == 0;
    	errno = 0;
    	ioctl(out_fd, MTIOCGET, &status);
    	totape |= errno == 0;
    	errno = 0;

	if(extract && (totape || fromtape || skip))
	{
		fprintf(stderr, "Can only extract from file\n");
		exit(1);
	}
/*
**	Skip number of files, specified by -snnn, given on the command
**	line. This is used to copy second and subsequent files on the
**	tape. 
*/

    	if (verbose && skip)
		fprintf(stderr, "copytape: skipping %d input files\n", skip);
    	for (i = 0; i < skip; i++)
	{
		do
	    		len = input();
		while (len != TAPE_MARK && len != END_OF_TAPE);

		if (len == END_OF_TAPE)
		{
	    		fprintf(stderr, "copytape: only %d files in input\n", i);
	    		exit(-1);
		}
    	}

/*
**	Do the copy. 
*/

    	while (limit && len != END_OF_TAPE)
	{
		do
		{
	    		do
			{
				len = input();
				if (len == FORMAT_ERROR)
				{
		    				perror("copytape: data format error - block ignored");
		    				exit(1);
				}
	    		} while (len == FORMAT_ERROR);

	    		output(len);

	    		if (verbose)
			{
				switch (len)
				{
		  			case TAPE_MARK:
		    				fprintf(stderr, "  copied TAPE_MARK\n");
		    				break;

		  			case END_OF_TAPE:
		    				fprintf(stderr, "  copied END_OF_TAPE\n");
		    				break;

		  			default:
		    				fprintf(stderr, "  copied %d bytes\n", len);
				}
	    		}
	}
	while (len > 0);
	limit--;
    }
    exit(0);
}


/*
 * Input up to 32K from a file or tape. If input file is a tape, then
 * do markcount stuff.  Input record length will be supplied by the
 * operating system. 
 *
 * If input file is not a tape, then read an integer from the input.  This
 * integer will specify the number of bytes that comprise the following
 * record, not counting the length of the integer itself. 
 *
 * An input length of zero corresponds to a tape mark. We return the
 * number of bytes read.  If we hit logical EOT (two tape marks in a
 * row), then return -1. 
 */

input()
{
    static          markcount = 0;	/* number of consecutive tape
					 * marks */
    int             len;
    int		    check[2];

    if (fromtape) {
	len = read(inp_fd, tapebuf, BUFLEN);
	if (len == 0) {
	    if (++markcount == 2)
		return END_OF_TAPE;
	    else
		return TAPE_MARK;
	}
	markcount = 0;		/* reset tape mark count */
	return len;
    }
/*
**	read format word and checksum word
*/

	if(read(inp_fd, check, sizeof check) != sizeof check)
	{
		fprintf(stderr, "Unexpected EOF on input\n");
		exit(1);
	}
	if(check[0] != -check[1])
	{
		fprintf("Check error on input record size\n");
		exit(1);
	}
	len = check[0];
	if(len != TAPE_MARK && len != END_OF_TAPE)
	{
		if(read(inp_fd, tapebuf, len) != len)
		{
			fprintf(stderr, "Unexpected read error on data\n");
			exit(1);
		}
	}
    	return len;
}

/*
 * Copy a buffer out to a file or tape. 
 *
 * If output is a tape, write the record.  A length of zero indicates that
 * a tapemark should be written. 
 *
 * If not a tape, write len to the output file, then the buffer.  
 */

output(len)
    int             len;
{
    int	check[2], cnt;
    struct mtop     op;

    if (extract)
    {
	if(len == TAPE_MARK || len == END_OF_TAPE)
    	{
		if(!ex_fd)
			fprintf(stderr, "Unexpected error on extract. No file open!\n");
		close(ex_fd);
		ex_fd = 0;
		return;
	}
	if(!ex_fd)		/* must be a new file */
	{
		sprintf(filename, file_proto, file_numb++);
		if((ex_fd = creat(filename, 0600)) < 0)
		{
			fprintf(stderr, "Could not create file '%s'\n", filename);
			exit(1);
		}
		if(verbose)
			fprintf(stderr, "Created '%s'\n", filename);
	}
   	if((cnt = write(ex_fd, tapebuf, len)) != len)
   	{
		fprintf(stderr, "Unexpected write error\n");
		fprintf(stderr, "Count was %d, asked %d\n", cnt, len);
		perror("possible cause :-");
		exit(1);
	}
	return;
   }

    if (totape && (len == TAPE_MARK || len == END_OF_TAPE)) {
	op.mt_op = MTWEOF;
	op.mt_count = 1;
	ioctl(out_fd, MTIOCTOP, &op);
	return;
    }
    if (!totape)
    {
		check[0] = len;
		check[1] = -len;
		if(write(out_fd, check, sizeof check) != sizeof check)
		{
			fprintf(stderr, "Unexpected write error\n");
			exit(1);
		}
    }
    if(len == TAPE_MARK || len == END_OF_TAPE)
	return;

   if((cnt = write(out_fd, tapebuf, len)) != len)
   {
	fprintf(stderr, "Unexpected write error\n");
	fprintf(stderr, "Count was %d, asked %d\n", cnt, len);
	perror("possible cause :-");
	exit(1);
   }
}
