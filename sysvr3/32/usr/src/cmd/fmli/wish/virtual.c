/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.wish:src/wish/virtual.c	1.9"

#include	<stdio.h>
#include	<ctype.h>
#include	"wish.h"
#include	"token.h"
#include	"vtdefs.h"
#include	"actrec.h"
#include	"slk.h"
#include	"moremacros.h"

/*
 * Caution: MAX_ARGS is defined in several files and should ultimately reside
 * in wish.h 
 */
#define MAX_ARGS	25
extern char	*Args[MAX_ARGS];
extern int	Arg_count;

static bool	command_mode;

token
virtual_stream(t)
register token	t;
{
	char	*s;
	register token	c;
	char	*tok_to_cmd();

	if ( t > 037 && t < 0177 )
		return t;

	Arg_count = 0;
	if (s = tok_to_cmd(t))
		t = cmd_to_tok(s);
	if (t == TOK_COMMAND) {
		/* single equals sign is correct, here */
		if (command_mode = !command_mode) {
			token	done_cmd();
			char	*cur_cmd();

			get_string(done_cmd, "--> ", cur_cmd(), 0, TRUE, "$VMSYS/OBJECTS/Menu.h6.list", NULL);
			t = TOK_NOP;
		}
		else
			t = TOK_CANCEL;
	}
	else {
		if (t >= TOK_SLK1 && t <= TOK_SLK8)
			t = slk_token(t);
	}
	return t;
}

static token
done_cmd(s, t)
char	*s;
token	t;
{
	char	*strchr();

	if (t == TOK_CANCEL)
		t = TOK_NOP;
	else {
		int i;
		if (s[0] == '!') {
			char	*tok_to_cmd();

			t = TOK_OPEN;
			for (i=0; i < 5; i++)
				if (Args[i])
					free(Args[i]); /* les */

			Args[0] = strsave("OPEN");
			Args[1] = strsave("EXECUTABLE");
			Args[2] = strsave("${SHELL:-/bin/sh}");
			Args[3] = strsave("-c");
			Args[4] = strsave(&s[1]);
			Arg_count = 5;
		}
		else {
			set_Args(s);
			if ((t = cmd_to_tok(Args[0])) == TOK_NOP)
				t = TOK_UNK_CMD;
			else if (t == TOK_NUNIQUE) {
				char msg[80];

				sprintf(msg, "Command '%s' not unique.  Type more of its name.", Args[0]);
				mess_temp(msg);
				t = TOK_NOP;
			}
			else if (t < 0) {
				/*
				 * Application defined command
				 */
				t = do_app_cmd();
			}
		}
	}
	command_mode = FALSE;
	return t;
}

set_Args(s)
char *s;
{
	for (Arg_count = 0; Arg_count < (MAX_ARGS - 1); Arg_count++) {
		while (*s && isspace(*s))
			s++;
		if (*s == '\0')
			break;

		if (Args[Arg_count])
			free(Args[Arg_count]); /* les */

		Args[Arg_count] = s;

		while (*s && !isspace(*s))
			s++;
		if (*s != '\0')
			*s++ = '\0';
		Args[Arg_count] = strsave(Args[Arg_count]);
#ifdef _DEBUG
		_debug(stderr, "Args[%d] = '%s'\n", Arg_count, Args[Arg_count]);
#endif
	}

	if (Args[Arg_count])
		free(Args[Arg_count]); /* les */

	Args[Arg_count] = NULL;
}
