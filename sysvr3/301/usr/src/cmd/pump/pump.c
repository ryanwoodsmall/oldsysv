/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)pump:pump.c	1.3"
/*
 *		Copyright 1984 AT&T
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <ldfcn.h>
#include <sys/cio_defs.h>
#include <sys/pump.h>

#define MAXBUF	2048

struct stat statb;

struct pump_st pumpst;

char buffer[MAXBUF];

extern int errno;

int fd;

main(argc, argv)
int argc;
char *argv[];
{
	LDFILE	*ldptr;
	FILHDR	*filehead;
	SCNHDR	secthdr;
	int	sect;
	char	*ptr;
	char	*startbuf;
	char	*endbuf;
	long	last_paddr;
	long	last_size;
	int	size;
	short	sections;
	long	force;
	long	addr;
	int	i;
	int	found = 0;
	extern 	int sendcmd();

	if( argc < 3 )
	{
		fprintf(stderr, "Usage: pump devfile pumpfile\n");
		exit(1);
	}

	if(stat(argv[1], &statb) < 0)
	{
		fprintf(stderr, "Pump error: %d - Can't get status of %s\n",
		errno, argv[1]);
		exit(1);
	}

	pumpst.dev = statb.st_rdev;
	pumpst.min = (short)minor(statb.st_rdev);

	if((fd = open(argv[1], O_PUMP)) < 0)
	{
		fprintf(stderr, "Error %d: Can't open %s\n", errno, argv[1]);
		exit(1);
	}

	if(sendcmd(&pumpst, argv[1], PU_RST, NORMAL) != 0)
	{
		if(pumpst.retcode == PU_DEVCH)
			exit(2);
		else
			exit(1);
	}

	pumpst.options = 0;

	ldptr = NULL;

	if((ldptr = ldopen(argv[2], ldptr)) == NULL)
	{
		fprintf(stderr, "Can't open %s for reading!\n", argv[2]);
		exit(1);
	}

	if(HEADER(ldptr).f_magic != B16MAGIC && HEADER(ldptr).f_magic != X86MAGIC)
	{
		fprintf(stderr,
		"Error: Object file is not in b16 or x86 common object format\n");
		exit(1);
	}

	ptr = startbuf = buffer;
	endbuf = buffer + MAXBUF;
	pumpst.bufaddr = (long)buffer;
	pumpst.size = MAXBUF;

	sections = HEADER(ldptr).f_nscns;

	for(sect = 1; sect <= sections; sect++)
	{
		if(ldshread(ldptr, sect, &secthdr) == FAILURE)
		{
			fprintf(stderr, "Pump: can't read section in %s\n",
 			argv[1]);
			exit(1);
		}

		if(strcmp(secthdr.s_name, ".start") == 0)
		{
			force = secthdr.s_paddr;
			found = 1;
		}

		if(sect == 1)
		{
			if(secthdr.s_paddr % 256 != 0)
			{
				fprintf(stderr,
				"Pump: Start address must be a multiple of 256\n");
				exit(1);
			}

			pumpst.to_addr = secthdr.s_paddr;
		}
		else
		{
			if(secthdr.s_paddr - last_size != last_paddr)
			{
				addr = last_paddr + last_size;
				while(addr < secthdr.s_paddr)
				{
					if(ptr < endbuf)
					{
						*ptr++ = 0;
						addr++;
					}
					else
					{
						if(sendcmd(&pumpst, argv[1],
						PU_DLD, NORMAL) != 0)
							exit(1);

						pumpst.to_addr += MAXBUF;
						ptr = startbuf;
					}
				}
			}
		}

		if(secthdr.s_scnptr > 0)
		{
			if(ldsseek(ldptr, sect) == FAILURE)
			{
				fprintf(stderr,
				"Pump: Can't read section %d\n", sect);
				exit(1);
			}
		}

		addr = secthdr.s_paddr + secthdr.s_size;
		last_paddr = secthdr.s_paddr;

		while(last_paddr < addr)
		{
			if((addr - last_paddr) < (endbuf - ptr))
			{
				if(secthdr.s_scnptr > 0)
				{
					FREAD(ptr, sizeof(*ptr),
					(int)(addr - last_paddr), ldptr);
					ptr += (addr - last_paddr);
					last_paddr += (addr - last_paddr);
				}
				else
				{
					while(last_paddr < addr)
					{
						*ptr++ = 0;
						last_paddr++;
					}
				}

			}
			else
			{
				if(secthdr.s_scnptr > 0)
				{
					FREAD(ptr, sizeof(*ptr),
					(int)(endbuf - ptr), ldptr);
					last_paddr += (endbuf - ptr);
				}
				else
				{
					while(ptr < endbuf)
					{
						*ptr++ = 0;
						last_paddr++;
					}
				}

				ptr = startbuf;
				if(sendcmd(&pumpst, argv[1],
				PU_DLD, NORMAL) != 0)
					exit(1);

				pumpst.to_addr += MAXBUF;
			}
		}

		if(sect == sections && ptr != startbuf)
		{
			pumpst.size = ptr - startbuf;
			if(sendcmd(&pumpst, argv[1], PU_DLD, NORMAL) != 0)
				exit(1);
		}
		last_paddr = secthdr.s_paddr;
		last_size = secthdr.s_size;
	}

	ldclose(ldptr);

	if(found == 0)
	{
		fprintf(stderr, "Error: No section name called .start\n");
		exit(1);
	}

	pumpst.to_addr = force;

	if(sendcmd(&pumpst, argv[1], PU_FCF, NORMAL) != 0)
		exit(1);

	sleep(1);

	if(sendcmd(&pumpst, argv[1], PU_SYSGEN, SYSGEN) != 0)
		exit(1);

	if(close(fd) < 0) {
		fprintf(stderr, "Couldn't close the driver\n");
		exit(1);
	}
}

struct key {
	char *keyword;
} keytab[] = {
	{"Null"},
	{"Download"},
	{"Reset"},
	{"Force call to function"},
	{"Go to address"},
	{"Sysgen"}
};

void
prtcode(s, cmd, ret)
char *s;
int cmd;
int ret;
{

	switch(ret) {
		case FAULT:
			fprintf(stderr, "Pump: %s returned a CIO FAULT during %s\n", s, keytab[cmd].keyword);
			break;
		case QFAULT:
			fprintf(stderr, "Pump: %s returned a CIO Invalid Queue Entry during %s\n", s, keytab[cmd].keyword);
			break;
		case PU_OTHER:
			fprintf(stderr, "Pump: %s did not respond during %s\n", s, keytab[cmd].keyword);
			break;
		case PU_TIMEOUT:
			fprintf(stderr, "Pump: A timeout has occurred on %s during %s\n", s, keytab[cmd].keyword); 
			break;
		default:
			fprintf(stderr, "Pump: There was no return code for %s during %s\n", s, keytab[cmd].keyword);
			break;
	}

	return;
}

sendcmd(pumptr, arg, cmdval, retval)
struct pump_st *pumptr;
char *arg;
int cmdval;
int retval;
{
	extern int fd;
	extern void prtcode();

	pumptr->retcode = PU_NULL;
	pumptr->cmdcode = cmdval;

	if(ioctl(fd, PUMP, pumptr) < 0)
	{
		fprintf(stderr, "Pump Error: %d - ioctl call\n", errno);
		exit(1);
	}

	if(pumptr->retcode != retval)
	{
		prtcode(arg, pumptr->cmdcode, pumptr->retcode);
		return(pumptr->retcode);
	}

	return(0);
}
