/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.sys:src/sys/evfuncs.c	1.10"

#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"wish.h"
#include	"eval.h"
#include	"ctl.h"
#include	"moremacros.h"

#ifdef TEST
main(argc, argv)
char	*argv[];
{
	IOSTRUCT	*in, *out;

	wish_init(argc, argv);
	in = io_open(EV_USE_FP, stdin);
	out = io_open(EV_USE_FP, stdout);
	exit(evalargv(argc - 1, argv + 1, in, out));
}

mess_temp(s)
char	*s;
{
	fprintf(stderr, "%s\n", s);
}

mess_perm(s)
char	*s;
{
	fprintf(stderr, "%s\n", s);
}

#endif 

shell(argc, argv, instr, outstr)
int	argc;
char	*argv[];
IOSTRUCT	*instr;
IOSTRUCT	*outstr;
{
	register int	i;
	register int	len;
	register char	*crunch;
	char	*myargv[4];
	char	*strnsave();

	len = 0;
	for (i = 1; i < argc; i++)
		len += strlen(argv[i]);
	crunch = strnsave(nil, len + argc - 1);
	for (i = 1; i < argc; i++) {
		strcat(crunch, " ");
		strcat(crunch, argv[i]);
	}
	myargv[0] = "sh";
	myargv[1] = "-c";
	myargv[2] = crunch;
	myargv[3] = NULL;
	i = execute(4, myargv, instr, outstr);
	free(crunch);
	return i;
}

execute(argc, argv, instr, outstr)
int	argc;
char	*argv[];
IOSTRUCT	*instr;
IOSTRUCT	*outstr;
{
	register char	*p;
	register int	errfd;
	register int	pid;
	int	pfd[2];
	char	*strchr();

	if (strcmp(argv[0], "extern") == 0)
		argv++;
#ifdef _DEBUG2
	for (pid = 0; pid < argc; pid++)
		_debug2(stderr, "argv[%d] = '%s'\n", pid, argv[pid]);
#endif
	if (pipe(pfd))
		return FAIL;
	if ((errfd = open(p = tmpnam(NULL), O_EXCL | O_CREAT | O_RDWR, 0600)) >= 0)
		unlink(p);
	else
		return FAIL;
	switch (pid = fork()) {
	case -1:
		return FAIL;
	case 0:
		close(pfd[0]);
		close(1);
		dup(pfd[1]);
		close(pfd[1]);
		close(2);
		dup(errfd);
		close(errfd);
		{
			if (instr->flags & EV_USE_FP) {
				close(0);
				dup(fileno(instr->mu.fp));
			}
			else if (instr->mu.str.count > 0) {
				register int	c;
				register FILE	*infp;
				FILE	*tempfile();

				if ((infp = tempfile(NULL, "w+")) == NULL)
					exit(1);
				while (c = getac(instr))
					putc(c, infp);
				close(0);
				dup(fileno(infp));
				fclose(infp);
			}
			execvp(argv[0], argv);
			perror();
			exit(1);
		}
		break;
	default:
		{
			register int	c;
			register int	retval;
			FILE	*fp;
			char retval_str[30];

			close(pfd[1]);
			if ((fp = fdopen(pfd[0], "r")) == NULL)
				return FAIL;
			while ((c = getc(fp)) != EOF)
				putac(c, outstr);
			fclose(fp);
			if ((retval = waitspawn(pid)) && errfd > 0) {
				char	buf[80];

				if ((c = read(errfd, buf, sizeof(buf) - 1)) > 0) {
					buf[c] = '\0';
					if (p = strchr(buf, '\n'))
						*p = '\0';
					mess_temp(buf);
				}
			}
			if (errfd > 0)
				close(errfd);
			sprintf(retval_str, "RET=%d", retval);
			putAltenv(retval_str);
			return retval;
		}
		break;
	}
	/*
	 * lint will complain about it, but there's actually no way to
	 * reach here because of the exit(), so this is not a return
	 * without an expression
	 */
}

get_wdw(argc, argv, instr, outstr)
int	argc;
char	*argv[];
IOSTRUCT	*instr;
IOSTRUCT	*outstr;
{
	int wdwnum;

	wdwnum = ar_ctl(ar_get_current(), CTGETWDW);
	putastr(itoa(wdwnum, 10), outstr);
	return(SUCCESS);
}

getmod(argc, argv, instr, outstr)
int	argc;
char	*argv[];
IOSTRUCT	*instr;
IOSTRUCT	*outstr;
{
	static long	modes;
	int i;
	struct stat	statbuf;
	long	strtol();
	char	*path_to_title();

	if (argc < 2)
		return FAIL;
	i = 1;
	if (argc > 2) {
		i++;
		if (strcmp(argv[1], "-u") == 0) {
			register char	*p;
			char	*getepenv();

			if ((p = getepenv("UMASK")) == NULL || *p == '\0')
				modes = 0775;
			else
				modes = ~(strtol(p, NULL, 8)) & 0777; 
		} else if (stat(argv[1], &statbuf) == -1) {
			mess_temp(nstrcat("Could not access object ", path_to_title(argv[1], NULL), NULL));
			return FAIL;
		} else
			modes = statbuf.st_mode;
	}
	if (strtol(argv[i], NULL, 8) & modes)
		putastr("yes", outstr);
	else
		putastr("no", outstr);
	return SUCCESS;
}

setmod(argc, argv, instr, outstr)
int	argc;
char	*argv[];
IOSTRUCT	*instr;
IOSTRUCT	*outstr;
{
	register int	i;
	register int	mode;

	if (argc < 2)
		return FAIL;
	for (i = 2, mode = 0; argv[i]; i++)
		mode = (mode << 1) | !strCcmp(argv[i], "yes");

	if ((mode & 0600) != 0600)
		mess_temp("WARNING: You are denying some permissions to yourself!");

	if (strcmp(argv[1], "-u") == 0) {
		char buf[20];

		mode = ~mode & 0777;
		(void) umask(mode);
		sprintf(buf, "0%o", mode);
		return chgepenv("UMASK", buf);
	} else if (chmod(argv[1], mode) < 0) {
		mess_temp(nstrcat("Unable to change security on ", path_to_title(argv[1], NULL), NULL));
		return(FAIL);
	} else
		return(SUCCESS);
}

int Long_line = -1;

long_line(argc, argv, instr, outstr)
int	argc;
char	*argv[];
IOSTRUCT	*instr;
IOSTRUCT	*outstr;
{
	register int maxcount, c, count;
	FILE *fp;

	if (argv[1]) {		/* if there is a file argument */
		if ((fp = fopen(argv[1], "r")) == NULL)
			return(FAIL);
		for (maxcount = 0, count = 0; (c = getc(fp)) != EOF; count++) {
			if (c == '\n') {
				maxcount = max(maxcount, count);
				count = -1;
			}
		}
		fclose(fp);
		Long_line = max(maxcount, count);
	}
	else if (Long_line < 0)
		return(FAIL);
	putastr(itoa(Long_line + 1, 10), outstr);
	return(SUCCESS);
}

read_file(argc, argv, instr, outstr)
int	argc;
char	*argv[];
IOSTRUCT	*instr;
IOSTRUCT	*outstr;
{
	char	*p, *f;
	register int c, count, maxcount;
	FILE	*fp;
	char	*path_to_full();

	if (argc > 1)
		f = path_to_full(argv[1]);
	else {
		f = path_to_full(p = io_string(instr));
		free(p);
	}
	if ((fp = fopen(f, "r")) == NULL)
		return(FAIL);
	free(f);
	for (count = 0, maxcount = 0; (c = getc(fp)) != EOF; count++) {
		if (c == '\n') {
			maxcount = max(maxcount, count);
			count = -1;
		}
		putac(c, outstr);
	}
	Long_line = max(maxcount, count);
	fclose(fp);
	return(SUCCESS);
}

cmd_echo(argc, argv, instr, outstr)
int	argc;
char	*argv[];
IOSTRUCT	*instr;
IOSTRUCT	*outstr;
{
	register char	*p;
	register int	i;
	char	*strrchr();

	for (i = 1; i < argc; i++) {
		if (i > 1)
			putac(' ', outstr);
		if (i == argc - 1 && (p = strrchr(argv[i], '\\')) && strcmp(p, "\\c'") == 0) {
			*p = '\0';
			putastr(argv[i], outstr);
			return SUCCESS;
		}
		putastr(argv[i], outstr);
	}
	putac('\n', outstr);
	return SUCCESS;
}

#ifndef TEST
extern int	cocreate();
extern int	cosend();
extern int	codestroy();
extern int	cocheck();
extern int	coreceive();
extern int	genfind();
extern int	cmd_pathconv();
#endif 
extern int	cmd_set();
extern int	cmd_run();
extern int	cmd_regex();
extern int	cmd_getlist();
extern int	cmd_setcolor();
extern int	cmd_reinit();
extern int	cmd_message();
extern int	cmd_indicator();
extern int	cmd_unset();
extern int	cmd_getodi();
extern int	cmd_setodi();

#define NUM_FUNCS	(sizeof(func) / sizeof(*func))

static struct {
	char	*name;
	int	(*function)();
} func[] = {
	{ "extern",	execute },
	{ "shell",	shell },
	{ "echo",	cmd_echo },
	{ "getmod",	getmod },
	{ "getodi",	cmd_getodi },
	{ "getfrm",	get_wdw },
	{ "getwdw",	get_wdw },	/* alias to getfrm */
	{ "setmod",	setmod },
	{ "setodi",	cmd_setodi },
	{ "readfile",	read_file },
	{ "longline",	long_line },
	{ "message",	cmd_message },
	{ "indicator", 	cmd_indicator },
	{ "set",	cmd_set },
	{ "unset",	cmd_unset },
	{ "run",	cmd_run },
	{ "regex",	cmd_regex },
	{ "getitems",	cmd_getlist },
	{ "genfind",	genfind },
	{ "pathconv",	cmd_pathconv },
	{ "setcolor",	cmd_setcolor},
	{ "reinit",	cmd_reinit},
#ifndef TEST
	{ "cocreate",	cocreate },
	{ "cosend",	cosend },
	{ "codestroy",	codestroy },
	{ "cocheck",	cocheck },
	{ "coreceive",	coreceive }
#endif 
};

evalargv(argc, argv, instr, outstr)
int	argc;
char	*argv[];
IOSTRUCT	*instr;
IOSTRUCT	*outstr;
{
	register int	n;

	if (argc < 1)
		return SUCCESS;
	for (n = 0; n < NUM_FUNCS; n++)
		if (strcmp(argv[0], func[n].name) == 0)
			break;
	if (n >= NUM_FUNCS)
		n = 0;
	return (*func[n].function)(argc, argv, instr, outstr);
}
