/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:ldsysdump.c	1.8"
#include <stdio.h>
#include <fcntl.h>

#include "sys/sbd.h"
#include "sys/cdump.h"
#include "errno.h"

/* needed  for message in err_query */
int	bindx ,
	seq_num = 1 ;

	
main (argc, argv)

int	argc ;
char	*argv[] ;

{
	int	dest ,
		cd_file ,
		timestamp ;

	char	reply[30] ,
		msg_buff[80] ;
	int	buffer[BLKSZ / sizeof (int)] ;
	struct	crash_hdr	crash_hdr ;
	
	if( argc != 2)
	{
		printf("usage: ldsysdump destination_file\n") ;
		exit(0) ;
	}
	
	if ((dest = creat(argv[1],0644)) == -1)
	{
		sprintf(msg_buff,"cannot open destination: %s\n",argv[1]) ;
		fatal(msg_buff) ;
	}

	setbuf(stdout,NULL) ;
		
	if(!query("Insert first sysdump floppy."))
	{
		printf("\ndestination file empty\n") ;
		exit(0) ;
	}
			
	while(1)
	{
		if((cd_file = open("/dev/ifdsk06",O_RDONLY)) == -1)
		{
			err_query("could not access sysdump floppy\n") ;
			continue ;
		}

		/* is this the first floppy? */
		if (lseek(cd_file,SPMEM - MAINSTORE - CHDR_OFFSET,0) == -1
		  ||  read(cd_file,&crash_hdr,sizeof(struct crash_hdr)) != sizeof(struct crash_hdr)
		  || strcmp(crash_hdr.sanity,CRASHSANITY) != 0)
		{
			/* is this floppy no. 2 - N? */
			if(lseek(cd_file,0,0) == -1
			   || read(cd_file,&crash_hdr,sizeof(struct crash_hdr)) != sizeof(struct crash_hdr)
			   || strcmp(crash_hdr.sanity,CRASHSANITY) != 0)
			{
				close(cd_file) ;
				err_query("floppy not recognized as sysdump floppy\n") ;
			}
			else
			{
				close(cd_file) ;
				err_query("sysdump floppy out of sequence") ;
			}
			continue ;
		}
		break ;
	}
	
	if(lseek(cd_file,0,0) == -1) /* reposition fp to start of file */
		fatal("could not reposition") ;
		
	timestamp = crash_hdr.timestamp ;
	
	printf("\nloading sysdump\n") ;
	
	for (bindx = 0; bindx < crash_hdr.mem_size / BLKSZ; )
	{
		switch(read(cd_file,buffer,BLKSZ))
		{
			case -1 :
				fatal("read error") ;
			case 0 :			/* EOF */
				close(cd_file) ;
				if(!query("\nInsert next sysdump floppy."))
				{
					close (cd_file) ;
					close (dest) ;
					printf("\n%d Sysdump files coalesced, %d (%d byte) blocks\n",seq_num,bindx,BLKSZ) ;
					exit (0) ;
				}
			
				while(1)
				{
					if((cd_file = open("/dev/ifdsk06",O_RDONLY)) == -1)
					{
						err_query("could not access floppy\n") ;
						continue ;
					}
				
					if (read(cd_file,buffer,BLKSZ) != BLKSZ)
					fatal("read error") ;
						
					crash_hdr = *(struct crash_hdr *)buffer ;
					
					if(strcmp(crash_hdr.sanity,CRASHSANITY) != 0)
					{
						if (lseek(cd_file,SPMEM - MAINSTORE - CHDR_OFFSET,0) == -1
						  ||  read(cd_file,&crash_hdr,sizeof(struct crash_hdr)) != sizeof(struct crash_hdr)
						  || strcmp(crash_hdr.sanity,CRASHSANITY) != 0)
						{
							err_query("floppy not recognized as sysdump floppy\n") ;
							close(cd_file) ;
						}
						else
						{
							close(cd_file) ;
							err_query("sysdump floppy out of sequence") ;
						}
						continue ;
					}

					if(crash_hdr.timestamp != timestamp)
					{
						close(cd_file) ;
						err_query("timestamps do no match\n") ;
						continue ;
					}

					if(crash_hdr.seq_num != ++seq_num)
					{
						seq_num-- ;
						close(cd_file) ;
						err_query("sysdump floppies out of sequence\n") ;
						continue ;
					}
					printf("\nloading more sysdump\n") ;
					break ;
				}
				continue ;				
		}
		
		if(write(dest,buffer,BLKSZ) == -1)
			fatal("write error") ;

		bindx++ ;
		
		/* print some dots for the folks to see */
		if (bindx % 10 == 9)
			printf(".");
	}

	close (dest) ;
	printf("\n%d Sysdump files coalesced, %d (%d byte) blocks\n",seq_num,bindx,BLKSZ) ;
	exit (0) ;
}

fatal(s)

char *s ;

{
	extern char *sys_errlist[];
	extern int sys_nerr;

	if((errno > 0) && (errno <= sys_nerr)) 
		printf("ldsysdump: %s\n",sys_errlist[errno]);
	else printf("ldsysdump: %s, errno = %d\n",s,errno);
	exit(-1);
}

int
query(s)

char	*s ;

{
	char	reply[100] ;
	
	printf("%s\n",s) ;
	while (1)
	{
		printf("Enter 'c' to continue, 'q' to quit: ") ;
		gets(reply) ;
		
		if(strcmp(reply,"q") == 0)
			return (0) ;

		if(strcmp(reply,"c") == 0)
			return (1) ;
			
		printf("\nreply \"%s\" unrecognized\n\n",reply) ;
	}
}

err_query(m)

char *m ;

{
	printf("%s\n",m) ;
	if(!query("Insert new floppy for retry"))
	{
		printf("\n%d Sysdump files coalesced, %d (%d byte) blocks\n",seq_num,bindx,BLKSZ) ;
		exit(0) ;
	}
	return ;
}
