/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)chkshlib:common/chkshlib.h	1.4"


#define	TRUE	1
#define	FALSE	0

#define	EXEC	0
#define	TARG	1
#define	HOST	2
#define	ISEXECUTABLE(x)		(x == EXEC)
#define	ISTARGET(x)		(x == TARG)
#define	ISHOST(x)		(x == HOST)

#define TABSIZ		503
#define STORE	0
#define CHECK	1
#define OPTARGS	"nvib"

extern int 	exit_status;
extern int 	incompatible_flag;
extern int 	bounds_flag;
extern int	vflag;
extern int	iflag;
extern int	bflag;
extern int	nflag;

typedef struct symrec {
	char 	*name;
	long 	addr;
	struct 	symrec 	*next;
} symrec;
extern symrec *hashtab[];

typedef struct bounds {
	long 		b_min;
	long 		b_max;
	struct 	bounds	*next;
} bounds;

typedef struct targ_name {
	char 		*t_name;
	struct 	targ_name	*next;
} targ_name;

typedef struct file_data {
	char 		*f_name;
	int		f_type;
	targ_name 	*f_targ;
	bounds		*f_bounds;
	struct file_data	*next;
} file_data;
extern file_data	*head, *tail, *current;

/* System supplied functions */
extern char 	*malloc(), *ldgetname();
extern void 	exit(), free();
extern LDFILE	*ldopen();
extern int	stat();

/* User supplied functions */
extern void	cmdline();
extern int	check_lib();
extern void	process_files();
extern void	process();
extern void	allsame();
extern void	read_lib();
extern void	read_symbols();
extern void	read_sym_tbl();
extern void	do_check();
extern void	checks();
extern long	hash();
extern void	usagerr();
extern void	printerr();
extern void	printwarn();
extern void	printwarn2();
extern void	print1();
extern void	print1_1();
extern void	print1_2();
extern void	print_result();
extern void	null_out();
extern void	reverse_files();
extern int 	type();
extern bounds 	*store_bounds();
extern bounds 	*fill_bounds();
extern char 	*chk_malloc();
extern char 	*chk2_malloc();
extern char 	*stralloc();
extern file_data 	*filedata_alloc();
extern targ_name 	*targname_alloc();
extern targ_name	*store_lib();
extern symrec	*look_up();
