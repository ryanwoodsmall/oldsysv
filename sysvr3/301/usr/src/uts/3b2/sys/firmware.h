/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/firmware.h	10.2"

#define BASE	0x48C	/* The physical address of the vector table	*/
			/* in system board rom.  Fixed address.		*/
#define VROM	0x20000	/* The virtual address at which the kernel maps	*/
			/* the system board rom.			*/

/*	The following is the virtual address of the vector table
**	in the kernel.
*/

#define VBASE	((struct vectors *)(VROM + BASE))

#define ON 1		 /* multi purpose defines  */
#define OFF 0

#define FW_PASS 1
#define FW_FAIL 0

/* default password for entry to mcp interactive mode */
#define DEF_PW "mcp"

/* Reset Request */
#define RST SBDWCSR->s_reqrst

/* Default value for softpower initialization */
#define SITINIT 0x64

/********************/
/* Console location */
/********************/

#define CONS OIDUART		/* 3b2 console DUART */
#define LINK_ADDR (&((struct duart *)CONS)->mr1_2b)
				/* 3b2 link address */

#define LINK ((struct duart *)LINK_ADDR)
#define DEMON_TTY 1
#define DATA_LINK 0

/************************************/
/* structure for PROM serial number */
/************************************/

struct serno {
		char fill0;
		char fill1;
		char fill2;
		char serial0;
		char fill4;
		char fill5;
		char fill6;
		char serial1;
		char fill8;
		char fill9;
		char filla;
		char serial2;
		char fillc;
		char filld;
		char fille;
		char serial3;
} ;


/*************************/
/* TRANSFER VECTOR TABLE */
/*************************/

	struct vectors {
		long *p_runflg;		/* flag indicating whether */
					/* system is safe for booting */
					/* 0xFEEDBEEF = fatal error */
		struct edt *p_edt;	/* ptr to equipped device table */
		long (**p_inthand)();	/* ptr to location containing */
					/* address of int. handlers */
		long (**p_exchand)();	/* ptr to locationcontaining */
					/* address of exc. handler */
		long (**p_rsthand)();	/* ptr to location containing */
					/* address of reset handler */
		struct bootcmd *p_cmdqueue;	/* ptr to command queue */
						/* for boot command */
		struct fl_cons *p_fl_cons;	/* ptr to float cons struct */

		/* Commonly used routines available to firmware */

		long *p_option;		/* ptr to ptr to option */
					/* number (for getedt) */
		char (*p_getedt)();	/* routine to fill edt structure */
		char (*p_printf)();	/* location of printf routine */
		char (*p_gets)();	/* location of gets routine */
		char (*p_sscanf)();	/* location of sscanf routine */
		char (*p_strcmp)();	/* location of strcmp routine */
		char (*p_excret)();	/* routine to set up a return */
					/* point for exceptions */
		char *p_access;		/* access permiss for printf etc. */
		char (*p_getstat)();	/* routine to check console for */
					/* a character present */
		char (*p_chknvram)();	/* location of routine to verify */
					/* checksum over non-volatile RAM */
		char (*p_rnvram)();	/* location of routine to read */
					/* non-volatile RAM */
		char (*p_wnvram)();	/* location of routine to write */
					/* non-volatile RAM */
		char (*p_hd_acs)();	/* location of routine to access */
					/* hard disk */
		char (*p_fd_acs)();	/* location of routine to access */
					/* floppy disk */
 		char *p_num_edt;	/* ptr to location containing */
					/* number of devices in edt */
 		long *p_memsize;	/* ptr to location containing */
					/* size of main memory */
 		long *p_memstart;	/* ptr to location containing */
					/* start of main memory for UN*X */
 		char *p_release;	/* ptr to location containing */
					/* release of code */
		struct pdinfo *p_physinfo;	/* ptr to disk phys info */
  		long *p_pswstore;	/* ptr to psw before exc or interrupt */
  		long *p_pcstore;	/* ptr to pc before exc or interrupt */
  		struct duart **p_console;	/* ptr to ptr to console uart */
  		char (*p_setbaud)();    /* ptr to setbaud routine */
  		long *p_save_r0;	/* ptr to r0 before exc */
		struct serno *p_serno;  /* ptr to serial number struct */
		long (**p_bpthand)();	/* ptr to location containing address */
					/* of bpt and trace exc handler */
		char *p_spwrinh;	/* location for soft power inhibit */
		long *p_meminit;	/* location for memory init flag */
		long (*p_bzero)();	/* ptr to memory zero routine */
		long (*p_setjmp)();	/* ptr to setjmp routine */
		long (*p_longjmp)();	/* ptr to longjmp routine */
		char (*p_dispedt)();	/* ptr to display edt routine */
		long (*p_hwcntr)();	/* ptr to duart counter delay routine */
		long *dmn_vexc;		/* demon virt proc/stk excep hdlr pcb */
		long *dmn_vgate;	/* demon virtual gate table location */
		long *dmn_vsint;	/* demon virt stray intrpt pcb loc */
		char (*p_fw_sysgen)();	/* ptr to generic sysgen routine */
		char (*p_ioblk_acs)();	/* ptr to ioblk_acs routine */
		char (*p_brkinh)();	/* ptr to break inhibit routine */
		char *p_hdcspec;	/* location of fw hdc spec params */
		int  (*p_symtell)();	/* Function to tell xmcp where	*/
					/* the symbol table is.		*/
		int (*p_demon)();	/* Function to enter demon w/o init */
	};

/*************************************/
/* defines for transfer vector table */
/*************************************/

/* gives contents of run flag */
#define RUNFLG (*(((struct vectors *)BASE)->p_runflg))
#define SAFE 0L
#define FATAL 0xFEEDBEEFL   /* fatal error, reset system */
#define INIT  0x3B02F1D0L   /* MAS has been init'ed */
#define VECTOR 0xA11C0DEDL  /* reset goes to rst_handler */
#define REBOOT 0x8BADF00DL  /* reboot w/o diags for UN*X */
#define REENTRY 0xADEBAC1EL /* reenter fw from a reset w/o failure mesage */

/* ptr to equipped device table */
#define P_EDT (((struct vectors *)BASE)->p_edt)
#define EDTP(X) (P_EDT + X)

/* ptr to location containing interrupt handler addr */
#define INT_HAND *(((struct vectors *)BASE)->p_inthand)

/* ptr to location containing exception handler addr */
#define EXC_HAND *(((struct vectors *)BASE)->p_exchand)

/* ptr to location containing bpt/trace exception handler addr */
#define BPT_HAND *(((struct vectors *)BASE)->p_bpthand)

/* ptr to location containing exception handler addr */
#define RST_HAND *(((struct vectors *)BASE)->p_rsthand)

/* ptr to boot command queue */
#define P_CMDQ (((struct vectors *)BASE)->p_cmdqueue)

/* ptr to board number currently under diagnosis */
#define OPTPTR (*(((struct vectors *)BASE)->p_option))
#define OPTION (*((long *)OPTPTR))

/* ptr to getedt routine, calls routine directly */
#define GETEDT (((struct vectors *)BASE)->p_getedt)

/* ptr to getstat routine, calls routine directly */
#define GETSTAT (((struct vectors *)BASE)->p_getstat)

/* ptr to printf routine, calls routine directly */
#define PRINTF (((struct vectors *)BASE)->p_printf)

/* ptr to gets routine, calls routine directly */
#define GETS (((struct vectors *)BASE)->p_gets)

/* ptr to strcmp routine, calls routine directly */
#define STRCMP (((struct vectors *)BASE)->p_strcmp)

/* ptr to sscanf routine, calls routine directly */
#define SSCANF (((struct vectors *)BASE)->p_sscanf)

/* ptr to chknvram routine, calls routine directly */
#define CHKNVRAM (((struct vectors *)BASE)->p_chknvram)

/* ptr to rnvram routine, calls routine directly */
#define RNVRAM (((struct vectors *)BASE)->p_rnvram)

/* ptr to chknvram routine, calls routine directly */
#define WNVRAM (((struct vectors *)BASE)->p_wnvram)

/* ptr to hard disk access routine */
#define HD_ACS (((struct vectors *)BASE)->p_hd_acs)

/* ptr to floppy disk access routine */
#define FD_ACS (((struct vectors *)BASE)->p_fd_acs)

/* floppy motor and select controls */

#define FIRST 0		/* initial xfer of multiple xfer (select,spin-up) */
#define NOCHANGE 1	/* subsequent xfers except for final */
#define LAST 2		/* final xfer (deselect,spin-down) */
#define SINGLE 3	/* one xfer (select,spin-up then deselect,spin-down) */

/* indication to hd_acs, fd_acs, and ioblk_acs routines of read or write */
#define BLKRD 0
#define BLKWT 1
#define DISKRD 0
#define DISKWT 1

/* ptr to excep. return routine,  exceptions return to the statement */
/* immediately following the last call to this routine */

#define EXCRET  (((struct vectors *)BASE)->p_excret)()

/* Change access rights to printf */
#define IO (*(((struct vectors *)BASE)->p_access))

/* ptr to number of edt entries */
#define NUM_EDT (*(((struct vectors *)BASE)->p_num_edt))

/* ptr to main memory size */
#define SIZOFMEM (*(((struct vectors *)BASE)->p_memsize))

/* ptr to main memory start */
#define STRTOFMEM (*(((struct vectors *)BASE)->p_memstart))

/* ptr to generic of firmware */
#define FW_REL (*(((struct vectors *)BASE)->p_release))

/* ptr to defect table in RAM */
#define PHYS_INFO (((struct vectors *)BASE)->p_physinfo)

/* ptr to pswstore */
#define OLDPSW (*(((struct vectors *)BASE)->p_pswstore))

/* ptr to pcstore */
#define OLDPC (*(((struct vectors *)BASE)->p_pcstore))

/* ptr to setbaud access routine */
#define SETBAUD (((struct vectors *)BASE)->p_setbaud)

/* ptr to console data structure */
#define CONSOLE (*(((struct vectors *)BASE)->p_console))

/* ptr to old r0 */
#define OLDR0 (*(((struct vectors *)BASE)->p_save_r0))

/* ptr to PROM serial number */
#define SERNO (((struct vectors *)BASE)->p_serno)

/* soft power inhibit */
#define SPWR_INH (*(((struct vectors *)BASE)->p_spwrinh))

/* memory initialization */
#define MEMINIT (*(((struct vectors *)BASE)->p_meminit))

/* ptr to bzero routine, calls routine directly */
#define BZERO (((struct vectors *)BASE)->p_bzero)

/* ptr to setjmp routine, calls routine directly */
#define SETJMP (((struct vectors *)BASE)->p_setjmp)

/* ptr to longjmp routine, calls routine directly */
#define LONGJMP (((struct vectors *)BASE)->p_longjmp)

/* ptr to dispedt routine, calls routine directly */
#define DISPEDT (((struct vectors *)BASE)->p_dispedt)

/* ptr to hwcntr routine, calls routine directly */
#define HWCNTR (((struct vectors *)BASE)->p_hwcntr)

/* addr of floating console structure */
#define FL_CONS (((struct vectors *)BASE)->p_fl_cons)

/* addr of demon virtual proc/stk exception pcb for component test */
#define DMN_VEXC (((struct vectors *)BASE)->dmn_vexc)

/* addr of demon virtual gate table for component test */
#define DMN_VGATE (((struct vectors *)BASE)->dmn_vgate)

/* addr of demon virtual stray interrupt pcb for component test */
#define DMN_VSINT (((struct vectors *)BASE)->dmn_vsint)

/* ptr to fw_sysgen routine, calls routine directly */
#define FW_SYSGEN (((struct vectors *)BASE)->p_fw_sysgen)

#define FW_CQ_ADDR 0x20037ec  /* 0x814 from BOOTADDR for min comp and req Qs */
#define FW_RQ_ADDR 0x20037f4  /* min for req and comp Q overlap */

/* ptr to ioblk_acs routine, calls routine directly */
#define IOBLK_ACS (((struct vectors *)BASE)->p_ioblk_acs)

/* ptr to break inhibit routine, calls routine directly */
#define BRK_INH (((struct vectors *)BASE)->p_brkinh)

/* addr of hard disk controller specify parameters for dgn to restore */
#define FW_HDCSPEC (((struct vectors *)BASE)->p_hdcspec)

/* Location of function used to inform xmcp of location of symbol table. */

#define	P_SYMTELL	(((struct vectors *)BASE)->p_symtell)
#define	P_DEMON		(((struct vectors *)BASE)->p_demon)

/* Power checker macro */

#define CHECKPWR if (SPWR_INH != ON && SBDSIT->count0 != SITINIT) {	\
			RUNFLG = SAFE;					\
			MEMINIT = SAFE;					\
			((struct duart *)CONS)->ip_opcr=0x00;		\
			((struct duart *)CONS)->scc_sopbc=KILLPWR;	\
			while (1);					\
		}
