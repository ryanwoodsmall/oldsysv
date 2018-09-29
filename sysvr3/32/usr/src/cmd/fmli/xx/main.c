/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.xx:src/xx/main.c	1.24"


#include <stdio.h>
#include <signal.h>
#include "wish.h"
#include "token.h"
#include "actrec.h"
#include "terror.h"
#include "ctl.h"
#include "vtdefs.h"
#include "status.h"
#include "moremacros.h"

/*
 * External Globals
 */
int  Vflag = 0;		/* Set if running FACE User Interface */
int  Fmli_pid;		/* Process id of this FMLI */
char Semaphore[PATHSIZ] = "/tmp/FMLIsem.";	/* for asynch coprocs */
bool Suspend_interupt;
char *Suspend_window;
char *Aliasfile;	/* File of path aliases */

/*
 * Defines for SLK layout (4-4 or 3-2-3 layout can be specified in curses)
 */
#define FOURFOUR	"4-4"
#define LAYOUT_DESC	"slk_layout"

/*
 * Default FMLI Stream handler
 */
token physical_stream();
token virtual_stream();
token actrec_stream();
token global_stream();
token menu_stream();
token error_stream();

token (*Defstream[])() = {
	physical_stream,
	virtual_stream,
	actrec_stream,
	global_stream,
	error_stream,
	NULL
};

extern char	*Progname;

/* Status line variables */
long	Mail_check;
char	*Mail_file;
long	Cur_time;

/*
 * Static globals
 */
static char Interpreter[] = "fmli";	
static char Viewmaster[] = "face";	
static char Vpid[12] = "VPID=";

static void vm_initobjects();
static void vm_setup();
static int sig_catch();

main(argc, argv)
int argc;
char *argv[];
{
	register int	i, r, c;
	static	char mail_template[25] = "/usr/mail/";
	token	t;
	extern	char *optarg;
	extern	int optind;
	char	*initfile, *commfile, *aliasfile, *p, *pidstr;
	int	labfmt, errflg;
	vt_id	vid, copyright();
	char	*itoa(), *estrtok(), *path_to_full();
	char	*filename(), *getenv(), *strnsave();
	void	susp_res(), vinterupt(), screen_clean();
	long	strtol(), time();
	int	exit();
	struct	actrec *firstar, *wdw_to_ar();

/* LES: optimize malloc(3X) */

	if ((p = getenv("VMFMLI")) && strcmp(p, "true") == 0) {
		Vflag++;
		putenv("VMFMLI=false");
	}
	errflg = 0;
	Progname = filename(argv[0]);
	Aliasfile = initfile = commfile = (char *) NULL;
	while ((c = getopt(argc, argv, "i:c:a:")) != EOF) {
		switch(c) {
		case 'i':
			initfile = optarg;
			break; 
		case 'c':
			commfile = optarg;
			break; 
		case 'a':
			Aliasfile = strsave(optarg);
			break;
		case '?':
			errflg++;
			break;
		}
	}

	/*
	 * Check arguments
	 */
	if (errflg) {
		fprintf(stderr, "usage: %s [-i initfile] [-c commandfile] [-a aliasfile] object ...\n", argv[0]);
		exit(-1);
	}
	if (optind == argc) {
		fprintf(stderr, "Initial object must be specified\n");
		exit(-1);
	}

	/* 
	 * check that all objects exist AND that they are readable
	 */
	if (initfile && access(initfile, 4) < 0) {
		errflg++;
		fprintf(stderr, "Can't open initialization file \"%s\"\n", initfile); 
	}
	if (commfile && access(commfile, 4) < 0) {
		errflg++;
		fprintf(stderr, "Can't open commands file \"%s\"\n", commfile); 
	}
	if (Aliasfile && access(Aliasfile, 4) < 0) {
		errflg++;
		fprintf(stderr, "Can't open alias file \"%s\"\n", Aliasfile); 
	}
	for (i = optind; i < argc; i++) {
		if (access(argv[i], 4) < 0) {
			errflg++;
			fprintf(stderr, "Can't open object \"%s\"\n", argv[i]);
		}
	}
	if (errflg)
		exit(-1);
		
	/*
	 * handle signals
	 */
	if (p = getenv("DEBUG"))
		_Debug = (int) strtol(p, 0, 0);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, exit);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, exit);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, exit);
	(void) signal(SIGALRM, sig_catch);

	/*
	 * initialize terminal/screen
	 */
	labfmt = 0;
	if (initfile) {
		/*
		 * Set up SLK layout as 4-4 or 3-2-3 ... This needs
		 * to be determined BEFORE curses is initialized
		 * in vt_init.
		 */
		char *slk_layout, *get_desc_val();

		slk_layout = get_desc_val(initfile, LAYOUT_DESC); 
		if (strcmp(slk_layout, FOURFOUR) == 0)
			labfmt = 1;

	}
	vt_init(labfmt);
	onexit(screen_clean);

	/*
	 * Read defaults from initialization file
	 */
	setup_slk_array();
	if (initfile) 
		read_inits(initfile);

	/*
	 * set up default banner and working indicator strings ...
 	 * and set color attributes if terminal supports color
	 */
	set_def_status();
	set_def_colors();

	/*
	 * Put up an Introductory Object and the working indicator
	 */
	vid = copyright();
	working(TRUE);
	if (vid)
		vt_close(vid);

	/*
	 * Initialize command table...
	 * Read commands from commands file
	 */
	cmd_table_init();
	if (commfile)
		read_cmds(commfile);

	/*
	 * Set up FACE globals
	 */
	if (Vflag)
		vm_setup();

	/*
	 * Set mailcheck (use mail_file variable as temp
	 * variable for MAILCHECK)
	 */
	Mail_check = 0L;
	if (Mail_file = getenv("MAILCHECK"))
		Mail_check = (long) atoi(Mail_file);
	if (Mail_check == 0L)
		Mail_check = 300L;
	else
		Mail_check = max(Mail_check, 120L);
	if ((Mail_file = getenv("MAIL")) == NULL) {
		Mail_file = mail_template;
		strcat(Mail_file, getenv("LOGNAME"));
	}
	Cur_time = time(0L);

	/*
	 * Initialize object architecture (SORTMODE)
	 */
	oh_init();

	/*
	 * set VPID env variable to pid for suspend/resume (SIGUSR1)
	 * and asynchronous activity (SIGUSR2)
	 */
	Fmli_pid = getpid();
	pidstr = itoa(Fmli_pid, 10);
	strcat(Vpid, pidstr);
	putenv(Vpid);
	strcat(Semaphore, pidstr);	/* for enhanced asynch coprocs */
	signal(SIGUSR1, susp_res);	/* set sigs for suspend/resume */
	signal(SIGUSR2, vinterupt);	/* set sigs for interactive windows */

	/*
	 * Setup windows to be opened upon login
	 */
	for (i = optind; i < argc; i++) {
		objop("OPEN", NULL, p = path_to_full(argv[i]), NULL);
		free(p);
		ar_ctl(ar_get_current(), CTSETLIFE, AR_IMMORTAL);
	}

	/*
	 * make the first window current
	 */
	if ((argc - optind) >= 1) {
		if (firstar = wdw_to_ar(1))
			ar_current(firstar);
		else {
			mess_flush(FALSE);
			vt_flush();
			sleep(3);
			exit(-1);
		}
	}

	/*
	 * Check wastebasket 
	 */
	if (Vflag) {
		vm_initobjects();
		showmail(TRUE);
	}

	while ((t = stream(TOK_NOP, Defstream)) != TOK_LOGOUT)
		;
	exit(0);
}

static bool Suspend_allowed = TRUE;

bool
suspset(b)
bool b;
{
	bool old = Suspend_allowed;

	Suspend_allowed = b;
	return(old);
}

long Interupt_pending = 0;

static void
vinterupt(sig)
int sig;
{
	(void) signal(sig, vinterupt);
	Interupt_pending++;
	return;
}

static void
susp_res(sig)
int sig;
{
	char *p;	/* temp for some operations */
	char buf[BUFSIZ];
	int  respid;
	FILE *fp;

	char *getepenv();

	(void) signal(sig, susp_res);

	sprintf(buf, "/tmp/suspend%d", getpid());
	if ((fp = fopen(buf, "r")) == NULL) {		/* resume failed */
		_debug(stderr, "Unable to open resume file\n");
		return;
	}

	(void) unlink(buf);
	if (fgets(buf, BUFSIZ, fp) == NULL) {
		_debug(stderr, "could not read resume file");
	} else {
		respid = atoi(buf);
		if (!Suspend_allowed) {
			fflush(stdout);
			fflush(stderr);
			printf("\r\n\nYou are not allowed to suspend at this time.\r\n");
			printf("You are in the process of logging out of FACE.\r\n");
			printf("Please take steps to end the application program you are\n\r");
			printf("currently using.\n\r");
			fflush(stdout);
			sleep(4);
			if (kill(respid, SIGUSR1) == FAIL)
				Suspend_interupt = TRUE;
			fclose(fp);
			return;
		}

		if (fgets(buf, BUFSIZ, fp) != NULL) {
			buf[strlen(buf) - 1] = '\0';
			if (buf[0])
				Suspend_window = strsave(buf);
			else
				Suspend_window = NULL;
		} else
			Suspend_window = NULL;
		_debug(stderr, "Resume pid is %d\n", respid);
		ar_ctl(ar_get_current(), CTSETPID, respid);
	}
	fclose(fp);

	Suspend_interupt = TRUE;	/* let wait loop in proc_current know */
	return;
}

static int
sig_catch(n)
int	n;
{
	signal(n, sig_catch);
}

static void
pull()		/* force various library routines to get pulled in */
{
	menu_stream();
	actrec_stream();
	error_stream();
	virtual_stream();
	stubs();
	indicator();
}

char 		Opwd[PATHSIZ+5];
extern char	*Home;
extern char	*Filecabinet;
extern char	*Wastebasket;
extern char	*Filesys;
extern char	*Oasys;

static void
vm_setup()
{
	static	char filecabinet[] = "/FILECABINET";
	static	char wastebasket[] = "/WASTEBASKET";
	char *p;

	/*
	 * get/set globals
	 */
	if ((Home = getenv("HOME")) == NULL)
		fatal(MUNGED, "HOME!");
	if ((Filesys = getenv("VMSYS")) == NULL)
		fatal(MUNGED, "VMSYS");
	if ((Oasys = getenv("OASYS")) == NULL)
		fatal(MUNGED, "OASYS");
	Filecabinet = strnsave(Home, strlen(Home) + sizeof(filecabinet) - 1);
	strcat(Filecabinet, filecabinet);
	Wastebasket = strnsave(Home, strlen(Home) + sizeof(wastebasket) - 1);
	strcat(Wastebasket, wastebasket);

	sprintf(Opwd, "OPWD=%s", Filecabinet);
	putenv(Opwd);
	if (p = getepenv("UMASK"))		/* set file masking */
		umask((int) strtol(p, NULL, 8));
}

#define MAX_WCUST (9)
static char	Loginwins[] = "LOGINWIN ";	/* leave space for a digit */

static void
vm_initobjects()
{
	char	*p;
	register int i;
	char *path_to_full(), *getepenv();
	struct actrec *prev, *firstobj;

	prev = NULL;
	firstobj = ar_get_current();
	for (i = 1; i <= MAX_WCUST; i++) {
		Loginwins[sizeof(Loginwins) - 2] = '0' + i;

		if ((p = getepenv(Loginwins)) != NULL && *p) {
			p = path_to_full(p);
			if (prev)
				ar_current(prev); /* aids ordering */
			objop("OPEN", NULL, p, NULL);
			if (firstobj != (prev = ar_get_current()))
				ar_ctl(prev, CTSETLIFE, AR_PERMANENT);
			free(p);
			ar_current(firstobj); /* aids ordering */
		}
	}

	/* clean out WASTEBASKET, if needed */
	if (p = getepenv("WASTEPROMPT")) {
		objop("OPEN", "MENU", strCcmp(p, "yes") ?
			"$VMSYS/OBJECTS/Menu.remove" :
			"$VMSYS/OBJECTS/Menu.waste", NULL);
		free(p);
	}
	else
		objop("OPEN", "MENU", "$VMSYS/OBJECTS/Menu.remove", NULL);
}
