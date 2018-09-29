/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libns:uidmap.c	1.5"
#include <stdio.h>
#include "idload.h"
#include <sys/utsname.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <nserve.h>
#include <sys/rfsys.h>
#include <errno.h>

extern	char *strtok();
extern	char *strchr();
extern	char *malloc();

static	char *cmd_name = "idload";
static	char *rulesfile;
static	char *passdir;
static	int   mapping;
static	int   line;
static	int   print_err_msg;
static	char *etcpass;
static	char *machpass;
static	struct lname	*host_head = NULL;

static	char *m_malloc();
static	char *read_file();
static	char *get_mname();
static	unsigned short getid();

uidmap(i_map, i_rules, i_passdir, map_mach, n_update)
int    i_map;
char  *i_rules;
char  *i_passdir;
char  *map_mach;
int    n_update;
{
	FILE	*fp;
	char	 curr_str[256];
	char	*token;
	char	*this_mach;
	struct	 mach	*head = NULL;
	struct	 mach	*curr_mach, *prev;
	struct	 lname	*name;
	int	 error=0;
	int	 global_set = 0, default_set = 0, order = 0;
	int 	 count, ignore;

	mapping = i_map;
	if (i_rules != NULL) {
		rulesfile = i_rules;
	} else {
		rulesfile = (mapping == UID_MAP)? DEF_USR_RULES : DEF_GRP_RULES;
	}

	passdir = (i_passdir == NULL)? DEF_PASSDIR : i_passdir;

	etcpass = NULL;
	machpass = NULL;
	host_head = NULL;

	if (map_mach == NULL)
		print_err_msg = 1;
	else
		print_err_msg = 0;

	if ((fp = fopen(rulesfile, "r")) == NULL) {
		if (print_err_msg)
			fprintf(stderr, "%s: cannot open rules file <%s>\n", cmd_name, rulesfile);
		return(FAILURE);
	}

	/*
	 *	Parse the rules file.
	 */

	line = 0;
	while (fgets(curr_str, sizeof(curr_str), fp) != NULL) {

		line ++;
		if ((token = strtok(curr_str, " \t\n")) == NULL)
			continue;

		if (*token == COMMENT_CHR)
			continue;

		/*
		 *	   GLOBAL
		 *	or HOST list_of_clients
		 */

		if (EQ(token, "global") || EQ(token, "host")) {
			if (head == NULL)
				head = curr_mach = (struct mach *)m_malloc(sizeof(struct mach));
			else {
				prev = curr_mach;
				curr_mach->m_next = (struct mach *)m_malloc(sizeof(struct mach));
				curr_mach = curr_mach->m_next;
			}

			curr_mach->m_def    = OTHERID;
			curr_mach->m_count  = 0;
			curr_mach->m_nmlist = NULL;
			curr_mach->m_name   = NULL;
			curr_mach->m_next   = NULL;

			default_set = 0;
			order = 0;

			/*
			 *	free up the area that held the machine
			 *	password for the previously mapped host.
			 */

			if (machpass != NULL) {
				free(machpass);
				machpass = NULL;
			}

			/*
		 	 *	Process the global specification.
			 */

			ignore = 0;
			if (EQ(token,"global")) {
				if (global_set) {
					parse_err(P_DUPNAME, "global", NULL);
					error = 1;
					continue;
				}
				global_set = 1;
				place_name(".", curr_mach);
				if (strtok(NULL, " \t\n") != NULL) {
					parse_err(P_XGLOB, NULL, NULL);
					error = 1;
				}
				continue;
			}

			/*
			 *	Process the host specification.
			 */

			this_mach = get_mname();
			count = 0;
			while ((token = strtok(NULL, " \t\n")) != NULL) {
				count ++;
				if (EQ(token, this_mach) 
				|| (map_mach != NULL && !EQ(token, map_mach)))
					ignore ++;
				else if (checkname(token) == SUCCESS)
					place_name(token, curr_mach);
				else {
					error = 1;
					ignore ++;
				}
			}

			if (count == 0) {
				parse_err(P_NOHOST, NULL, NULL);
				error = 1;
			}

			/*
			 *	If all hosts listed on the host
			 *	line are to be ignored, free up the
			 *	entry.
			 */

			if (ignore == count) {
				free(curr_mach);
				if (curr_mach == head)
					head = NULL;
				else {
					prev->m_next = NULL;
					curr_mach = prev;
				}
			} else {
				ignore = 0;
			}
			continue;
		}

		/*
		 *	   DEFAULT TRANSPARENT
		 *	or DEFAULT user_id
		 *	or DEFAULT user_name
		 */

		if (EQ(token, "default")) {
			if (ignore)
				continue;

			if (head == NULL) {
				parse_err(P_ORDER, token, NULL);
				error = 1;
				continue;
			}
			if ((token = strtok(NULL, " \t\n")) == NULL) {
				parse_err(P_NOVAL, "default", NULL);
				error = 1;
				continue;
			}

			if (default_set) {
				parse_err(P_XDEF, NULL, NULL);
				error = 1;
				continue;
			}
			default_set = 1;

			if ((order & A_MAP) || (order & A_EXCLUDE)) {
				parse_err(P_DEFORD, NULL, NULL);
				error = 1;
				continue;
			}

			if (EQ(token, "transparent")) {
				curr_mach->m_def = 0;
			} else {
				if (isnum(token)) {
					curr_mach->m_def = atoi(token);
				} else {
					if ((curr_mach->m_def = getid(NULL, token)) == GETID_FAIL) {
						parse_err(P_LNONAME, token, NULL);
						error = 1;
					}
				}
				/*
				 *	The default value cannot be
				 *	that of 0.
				 */
				if (curr_mach->m_def == 0) {
					parse_err(P_NOROOT, NULL, NULL);
					error = 1;
				}
			}

			/*
			 *	No more tokens can be specified on the
			 *	default line.
			 */

			if ((token = strtok(NULL, " \t\n")) != NULL) {
				parse_err(P_XDEF, NULL, NULL);
				error = 1;
			}
			continue;
		}

		/*
		 *	MAP list_of_values
		 */

		if (EQ(token, "map")) {
			if (ignore)
				continue;

			if (head == NULL) {
				parse_err(P_ORDER, token, NULL);
				error = 1;
				continue;
			}
			if ((token = strtok(NULL, " \t\n")) == NULL) {
				parse_err(P_NOVAL, "map", NULL);
				error = 1;
				continue;
			}

			order |= A_MAP;

			if (map_rtn(curr_mach, token) == FAILURE)
				error = 1;

			while ((token = strtok(NULL, " \t\n")) != NULL) {
				if (map_rtn(curr_mach, token) == FAILURE)
					error = 1;
			}
			continue;
		}

		/*
		 *	EXCLUDE list_of_values
		 */

		if (EQ(token, "exclude")) {
			if (ignore)
				continue;

			if (head == NULL) {
				parse_err(P_ORDER, token, NULL);
				error = 1;
				continue;
			}
			if ((token = strtok(NULL, " \t\n")) == NULL) {
				parse_err(P_NOVAL, "exclude", NULL);
				error = 1;
				continue;
			}

			if (order & A_MAP) {
				parse_err(P_EXORD, NULL, NULL);
				error = 1;
				continue;
			}

			order |= A_EXCLUDE;

			if (excl_rtn(curr_mach, token) == FAILURE)
				error = 1;

			while ((token = strtok(NULL, " \t\n")) != NULL) {
				if (excl_rtn(curr_mach, token) == FAILURE)
					error = 1;
			}
			continue;
		}

		parse_err(P_INVTOK, token, NULL);
		error = 1;
	}

	if (error)
		return(FAILURE);

	/*
	 *	If the -n was specified, simply print out the parsed
	 *	information to stdout.
	 */

	if (n_update)
		return(pr_out(head));

	/*
	 *	Populate the translation tables in kernel.
	 */

	curr_mach = head;
	while (curr_mach != NULL) {
		name = curr_mach->m_name;
		while (name != NULL) {
			pr_trtab(name->l_val, curr_mach);
			name = name->l_next;
		}
		curr_mach = curr_mach->m_next;
	}
	return(SUCCESS);
}

static
isnum(str)
char	*str;
{
	while (*str != '\0') {
		if (!DIGIT(*str))
			return(0);
		str ++;
	}
	return(1);
}

/*
 *	place_name() takes a machine name and places it at the end of
 *	the list of machines on a HOST line.
 */

static
place_name(name, curr_mach)
char 	     *name;
struct mach  *curr_mach;
{
	struct lname *new;
	struct lname *ptr;

	new = (struct lname *)m_malloc(sizeof(struct lname));
	strncpy(new->l_val, name, MAXSNAME);
	new->l_next = NULL;

	if (curr_mach->m_name == NULL)
		curr_mach->m_name = new;
	else {
		ptr = curr_mach->m_name;
		while (ptr->l_next != NULL)
			ptr = ptr->l_next;
		ptr->l_next = new;
	}
}

static
map_rtn(curr_mach, token)
struct mach *curr_mach;
char	    *token;
{
	char	*remote, *ptr, *local;
	char	*mach;

	struct nmlist *nmlist;

	mach = curr_mach->m_name->l_val;

	if (EQ(token, "all")) {
		return(map_all(mach, curr_mach));
	}

	/*
	 *	If the input token is of the form X:Y, then
	 *	X is remote and Y is local.  If the input token
	 *	is just X, then this is equivalent to X:X.
	 */

	remote = local = ptr = token;
	while (*ptr != '\0') {
		if (*ptr == ':') {
			*ptr = '\0';
			local = ptr + 1;
			break;
		}
		ptr ++;
	}

	nmlist = (struct nmlist *)m_malloc(sizeof(struct nmlist));
	nmlist->idmap = (struct idtab *)m_malloc(sizeof(struct idtab));
	nmlist->nm_map = (struct nm_tab *)m_malloc(sizeof(struct nm_tab));

	/*
	 *	If the remote value is numeric, then simply insert
	 *	the number as the remote map.  Otherwise, get the
	 *	user's id and place that as the remote map.
	 */

	if (isnum(remote)) {
		nmlist->idmap->i_rem = (unsigned short)atoi(remote);
		strncpy(nmlist->nm_map->n_rem, "n/a", MAX_USERNAME);
	} else {
		/*
		 *	Only numeric values are accepted for global
		 *	maps.
		 */
		if (EQ(curr_mach->m_name->l_val, ".")) {
			parse_err(P_NUMONLY, NULL, NULL);
			return(FAILURE);
		}
		if ((nmlist->idmap->i_rem = getid(mach, remote)) == GETID_FAIL) {
			parse_err(P_RNONAME, remote, mach);
			return(FAILURE);
		}
		strncpy(nmlist->nm_map->n_rem, remote, MAX_USERNAME);
	}

	/*
	 *	Process the local value...
	 */

	if (isnum(local)) {
		nmlist->idmap->i_loc = (unsigned short)atoi(local);
		strncpy(nmlist->nm_map->n_loc, "n/a", MAX_USERNAME);
	} else {
		if (EQ(curr_mach->m_name->l_val, ".")) {
			parse_err(P_NUMONLY, NULL, NULL);
			return(FAILURE);
		}
		if ((nmlist->idmap->i_loc = getid(NULL, local)) == GETID_FAIL) {
			parse_err(P_LNONAME, local, NULL);
			return(FAILURE);
		}
		strncpy(nmlist->nm_map->n_loc, local, MAX_USERNAME);
	}

	curr_mach->m_count ++;
	nmlist->next = NULL;

	return(place_map(curr_mach, nmlist));
}

static
place_map(curr_mach, nmlist)
struct mach *curr_mach;
struct nmlist *nmlist;
{
	struct	nmlist *save, *place, *prev;
	char	num[32];

	place = curr_mach->m_nmlist;

	/*
	 *	Find  the place to put the new mapped value.  If the
	 *	new mapped value is already in the list of mapped
	 *	values, issue a warning message, but return SUCCESS.
	 */

	while (place != NULL) {
		if (place->idmap->i_rem == nmlist->idmap->i_rem) {
			sprintf(num, "%d", nmlist->idmap->i_rem);
			parse_err(P_EXISTS, num, NULL);
			return(SUCCESS);
		}
		if (place->idmap->i_rem > nmlist->idmap->i_rem)
			break;
		prev = place;
		place = place->next;
	}

	if (place == curr_mach->m_nmlist) {
		save = curr_mach->m_nmlist;
		curr_mach->m_nmlist = nmlist;
	} else {
		/*
		 *	Check to make sure that the current map value
		 *	doesn't come in between an exclude range.
		 *	If it does, issue a warning message but return
		 *	SUCCESS.  The new map value won't be placed in
		 *	the list in this case.
		 */
		if (prev->idmap->i_loc == CONTINUATION) {
			sprintf(num, "%d", nmlist->idmap->i_rem);
			parse_err(P_MAPERR, num, NULL);
			return(SUCCESS);
		}
		save = prev->next;
		prev->next = nmlist;
	}
	nmlist->next = save;

	return(SUCCESS);
}

static
excl_rtn(curr_mach, token)
struct mach *curr_mach;
char	    *token;
{
	char	*first, *ptr;
	char	*last = NULL;
	char	*mach;
	char	 num[32];

	struct nmlist *save, *place, *prev, *nmlist;

	mach = curr_mach->m_name->l_val;

	/*
	 *	Determine if a range is specified.
	 */

	first = ptr = token;
	while (*ptr != '\0') {
		if (*ptr == '-') {
			*ptr = '\0';
			last  = ptr + 1;
			break;
		}
		ptr ++;
	}

	/*
	 *	If a '-' was seen, then both first and last must
	 *	be numeric and first must be less than last.
	 */

	if (last != NULL) {
		if (!isnum(first) || !isnum(last) || 
 	      	    atoi(first) >= atoi(last)) {
			parse_err(P_RANGE, NULL, NULL);
			return(FAILURE);
		}
	}

	nmlist = (struct nmlist *)m_malloc(sizeof(struct nmlist));
	nmlist->idmap = (struct idtab *)m_malloc(sizeof(struct idtab));
	nmlist->nm_map = (struct nm_tab *)m_malloc(sizeof(struct nm_tab));

	if (isnum(first)) {
		nmlist->idmap->i_rem = (unsigned short)atoi(first);
		strncpy(nmlist->nm_map->n_rem, "n/a", MAX_USERNAME);
	} else {
		/*
		 *	Only numeric values may be specified for global
		 *	mappings.
		 */
		if (EQ(curr_mach->m_name->l_val, ".")) {
			parse_err(P_NUMONLY, NULL, NULL);
			return(FAILURE);
		}
		if ((nmlist->idmap->i_rem = getid(mach, first)) == GETID_FAIL) {
			parse_err(P_RNONAME, first, mach);
			return(FAILURE);
		}
		strncpy(nmlist->nm_map->n_rem, first, MAX_USERNAME);
	}

	nmlist->idmap->i_loc = OTHERID;
	strncpy(nmlist->nm_map->n_loc, "guest_id", MAX_USERNAME);
	nmlist->next = NULL;

	curr_mach->m_count ++;

	/*
	 *	Place in another node in the list if a range was
	 *	specified.
	 */

	if (last != NULL) {
		nmlist->idmap->i_loc = CONTINUATION;
		strncpy(nmlist->nm_map->n_loc, "-", MAX_USERNAME);
		nmlist->next = (struct nmlist *)m_malloc(sizeof(struct nmlist));
		nmlist->next->idmap = (struct idtab *)m_malloc(sizeof(struct idtab));
		nmlist->next->idmap->i_rem = (unsigned short)atoi(last);
		nmlist->next->idmap->i_loc = OTHERID;
		nmlist->next->nm_map = (struct nm_tab *)m_malloc(sizeof(struct nm_tab));
		strncpy(nmlist->next->nm_map->n_rem, "n/a", MAX_USERNAME);
		strncpy(nmlist->next->nm_map->n_loc, "guest_id", MAX_USERNAME);
		nmlist->next->next = NULL;
		curr_mach->m_count ++;
	}

	/*
	 *	Place the information into the map list.
	 *	If the vaue has already been excluded, print out
	 *	a warning message but return SUCCESS.
	 */

	place = curr_mach->m_nmlist;
	prev = NULL;

	while (place != NULL) {
		if (place->idmap->i_rem == nmlist->idmap->i_rem) {
			sprintf(num, "%d", nmlist->idmap->i_rem);
			parse_err(P_EXCERR, num, NULL);
			return(SUCCESS);
		}
		if (place->idmap->i_rem > nmlist->idmap->i_rem)
			break;
		prev = place;
		place = place->next;
	}

	/*
	 *	Check to make sure the excluded value does not come
	 *	between another excluded range.  If it does, ignore
	 *	this exclude and return SUCCESS.
	 */

	if (prev != NULL && prev->idmap->i_loc == CONTINUATION) {
		sprintf(num, "%d", nmlist->idmap->i_rem);
		parse_err(P_EXCERR, num, NULL);
		return(SUCCESS);
	}

	/*
	 *	Check to make sure that an exclude range does not 
	 *	override a previously excluded value.
	 *	If it does, ignore this entry and return SUCCESS.
	 */

	if (last != NULL) {
		if (place != NULL
		 && place->idmap->i_rem < nmlist->next->idmap->i_rem) {
			parse_err(P_EXOVER, NULL, NULL);
			return(SUCCESS);
		}
	}

	/*
	 *	Insert the new range or value into the list.
	 */

	if (place == curr_mach->m_nmlist) {
		save = curr_mach->m_nmlist;
		curr_mach->m_nmlist = nmlist;
	} else {
		save = prev->next;
		prev->next = nmlist;
	}

	if (last != NULL) {
		nmlist = nmlist->next;
	}
	nmlist->next = save;

	return(SUCCESS);
}

static
pr_trtab(name, mach_info)
char	*name;
struct mach *mach_info;
{
	char *buf;

	struct idtable header, *hp;
	struct idtab   *ip;
	struct nmlist  *lptr;

	int	defval = mach_info->m_def;
	int	count = mach_info->m_count;
	struct	nmlist	*list = mach_info->m_nmlist;

	buf = m_malloc((count + 1) * sizeof(struct idtab));

	hp = (struct idtable *)buf;
	hp->h_defval = defval;
	hp->h_size = count;

	if (count == 0) {
		if (rfsys(RF_SETIDMAP, name, mapping, buf) < 0) {
			if (errno != ENOENT) {
				if (print_err_msg) {
					fprintf(stderr, "%s: cannot write translation information\n", cmd_name);
					perror("idload");
				}
				exit(1);
			}
		}
		return(0);
	}

	lptr = list;
	hp->h_idtab[0].i_rem = lptr->idmap->i_rem;
	hp->h_idtab[0].i_loc = lptr->idmap->i_loc;
	hp ++;
	ip = (struct idtab *)hp;
	lptr = lptr->next;

	while (lptr != NULL) {
		ip->i_rem = lptr->idmap->i_rem;
		ip->i_loc = lptr->idmap->i_loc;
		ip ++;
		lptr = lptr->next;
	}

	if (rfsys(RF_SETIDMAP, name, mapping, buf) < 0) {
		if (errno != ENOENT) {
			if (print_err_msg) {
				fprintf(stderr, "%s: cannot write translation information\n", cmd_name);
				perror("idload");
			}
			exit(1);
		}
	}
	return(0);
}

static
char *
m_malloc(size)
int size;
{
	char *ptr;

	if ((ptr = malloc(size)) == 0) {
		if (print_err_msg)
			fprintf (stderr, "%s: FATAL: cannot allocate memory\n", cmd_name);
		exit(1);
	}
	return(ptr);
}

static
unsigned short
getid(mach, name)
char	*mach;
char	*name;
{
	char *ptr, *val, *p_name, *replace;
	int  found = 0;
	int  inval = 0;
	unsigned short rtn = GETID_FAIL;

	/*
	 *	If the password file for the machine has not been read
	 *	in, read it into the static area for machine passwords
	 */

	if (mach == NULL) {
		if ((etcpass == NULL)
		  && ((etcpass = read_file(mach)) == NULL)) {
				return(rtn);
		}
		ptr = etcpass;
	} else {
		if ((machpass == NULL)
		  && ((machpass = read_file(mach)) == NULL)) {
				return(rtn);
		}
		ptr = machpass;
	}

	/*
	 *	Search the password file for the given name.
	 */

	while (*ptr != '\0') {
		p_name = ptr;
		if ((ptr = strchr(p_name, ':')) == NULL) {
			inval = 1;
			break;
		}
		*ptr = '\0';
		if (EQ(p_name, name)) {
			*ptr = ':';
			ptr ++;
			if ((ptr = strchr(ptr, ':')) == NULL) {
				inval = 1;
				break;
			}
			val = ++ptr;
			if ((ptr = strchr(val, ':')) == NULL) {
				inval = 1;
				break;
			}
			*ptr = '\0';
			if (isnum(val))
				rtn = (unsigned short)atoi(val);
			*ptr = ':';
			break;
		}
		*ptr = ':';
		if ((ptr = strchr(ptr, '\n')) == NULL) {
			inval = 1;
			break;
		}
		if (!inval)
			ptr ++;
	}

	if (inval && print_err_msg)
		fprintf(stderr, "%s: invalid format of %s %s", cmd_name, mapping == UID_MAP? "passwd":"group", mach==NULL?"local machine":mach);

	return(rtn);
}

static
char *
read_file(mach)
char *mach;
{
	char	file[512];
	char	*dom;
	char	*name;
	char	*buf;
	int	 fd;
	struct stat sbuf;

	if (mach == NULL)
		strcpy(file, "/etc");
	else {
		/*
		 *	seperate into domain and machine name.
		 */
		dom = name = mach;
		while (*name != '.') {
			if (*name == '\0') {
				parse_err(P_NODOM, mach, NULL);
				return(NULL);
			}
			name ++;
		}
		*name = '\0';
		name ++;
		strcpy(file, passdir);
		strcat(file, "/");
		strcat(file, dom);
		strcat(file, "/");
		strcat(file, name);
		/*
		 *	Return the machine to its original state.
		 */
		*(name - 1) = '.';
	}

	if (mapping == UID_MAP)
		strcat(file, "/passwd");
	else
		strcat(file, "/group");

	if (stat(file, &sbuf) == -1 ||
	    (fd = open(file, O_RDONLY)) == -1 ||
	    (buf = m_malloc(sbuf.st_size + 1)) == NULL ||
	    read(fd, buf, sbuf.st_size) != sbuf.st_size) {
		if (print_err_msg)
			fprintf(stderr, "%s: cannot get information from password file <%s>\n", cmd_name, file);
		return(NULL);
	}

	*(buf + sbuf.st_size) = '\0';
	return(buf);
}

static
map_all(mach, curr_mach)
char	*mach;
struct mach *curr_mach;
{
	struct nmlist *nmlist;
	char	*buf, *ptr;
	char	*p_name;
	char	*rid;
	unsigned short	loc, rem;
	int	inval = 0;

	/*
	 *	Only numeric values are allowed for global mappings.
	 */

	if (EQ(curr_mach->m_name->l_val, ".")) {
		parse_err(P_NUMONLY, NULL, NULL);
		return(FAILURE);
	}

	/*
	 *	Open the password file for the machine and map
	 *	every entry in that password file to the corresponding
	 *	entry in the local /etc/passwd file.
	 */

	if ((ptr = buf = read_file(mach)) == NULL)
		return(FAILURE);

	while (*ptr != '\0') {
		p_name = ptr;
		if ((ptr = strchr(p_name, ':')) == NULL) {
			inval = 1;
			break;
		}
		*ptr = '\0';

		/*
		 *	get the remote id.
		 */

		ptr ++;
		if ((ptr = strchr(ptr, ':')) == NULL) {
			inval = 1;
			break;
		}
		ptr ++;
		rid = ptr;
		if ((ptr = strchr(ptr, ':')) == NULL) {
			inval = 1;
			break;
		}
		*ptr = '\0';
		if (isnum(rid))
			rem = (unsigned short)atoi(rid);
		else {
			inval = 1;
			break;
		}

		/*
		 *	Print only a warning if the entry is not found
		 *	in the local password file, and ignore this user
		 *	(i.e., do not place the user in the map table).
		 */

		if ((loc = getid(NULL, p_name)) == GETID_FAIL) {
			if (print_err_msg)
				fprintf(stderr, "%s: warning: cannot get id of <%s> from /etc/%s when mapping machine <%s>\n", cmd_name, p_name, mapping == UID_MAP? "passwd":"group", mach);
		} else {

			/*
			 *	Place the new mapping into the map list.
			 */

			nmlist = (struct nmlist *)m_malloc(sizeof(struct nmlist));
			nmlist->idmap = (struct idtab *)m_malloc(sizeof(struct idtab));
			nmlist->nm_map = (struct nm_tab *)m_malloc(sizeof(struct nm_tab));
			nmlist->idmap->i_rem = rem;
			strncpy(nmlist->nm_map->n_rem, p_name, MAX_USERNAME);
			nmlist->idmap->i_loc = loc;
			strncpy(nmlist->nm_map->n_loc, p_name, MAX_USERNAME);
			curr_mach->m_count ++;

			place_map(curr_mach, nmlist);
		}

		/*
		 *	Read to the next line of the password file.
		 */

		ptr ++;
		if ((ptr = strchr(ptr, '\n')) == NULL) {
			inval = 1;
			break;
		}
		if (!inval)
			ptr ++;
	}

	if (inval) {
		if (print_err_msg)
			fprintf(stderr, "%s: invalid format of %s file for <%s>\n", cmd_name, mapping == UID_MAP? "passwd":"group", mach);
		return(FAILURE);
	}
	return(SUCCESS);
}

static
checkname(name)
char	*name;
{
	char	*ptr;
	struct  lname *curr;

	/*
	 *	Make sure that the name was specified as
	 *	domain.host.
	 */

	ptr = strchr(name, '.');
	if (ptr == NULL || ptr == name || *(ptr + 1) == '\0') {
		parse_err(P_NODOM, name, NULL);
		return(FAILURE);
	}

	/*
	 *	Make sure that the name has not already been specified.
	 */

	curr = host_head;
	while (curr != NULL) {
		if (EQ(curr->l_val, name)) {
			parse_err(P_DUPNAME, name, NULL);
			return(FAILURE);
		}
		curr = curr->l_next;
	}

	curr = (struct lname *)m_malloc(sizeof(struct lname));
	strncpy(curr->l_val, name, MAXSNAME);

	curr->l_next = host_head;
	host_head = curr;

	return(SUCCESS);
}

static
char *
get_mname()
{
	struct	utsname utsname;
	static	char	name[SZ_MACH + MAXDNAME + 1];

	char *dname, *getdname();

	if ((dname = getdname()) == NULL) {
		if (print_err_msg)
			fprintf(stderr, "%s: cannot obtain the domain name\n", cmd_name);
		exit(1);
	}
	strcpy(name, dname);
	strcat(name, ".");

	if (uname(&utsname) < 0) {
		if (print_err_msg)
			fprintf(stderr, "%s: cannot obtain the machine name\n", cmd_name);
		exit(1);
	}
	strcat(name, utsname.nodename);

	return(name);
}

static
char	*
getdname()
{
	static char dname[MAXDNAME];
	FILE	*fp;

	/*
	 *	If the domain name cannot be obtained from the system,
	 *	get it from the domain file.
	 */

	if (rfsys(RF_GETDNAME, dname, MAXDNAME) < 0) {
		if (((fp = fopen(NSDOM,"r")) == NULL)
		|| (fgets(dname,MAXDNAME,fp) == NULL))
			return(NULL);
		/*
		 *	get rid of trailing newline, if there
		 */
		if (dname[strlen(dname)-1] == '\n')
			dname[strlen(dname)-1] = '\0';
		fclose(fp);
	}
	return(dname);
}

static
pr_out(curr_mach)
struct mach	*curr_mach;
{
	struct lname *name;
	struct nmlist *list;

	char *machname, *machtype;
	char mach_idrem[BUFSIZ], machdef[BUFSIZ];
	int temp;
	static int pr_headings = 1;

	/*
	 *	Print out the parsed information to standard out.
	 */
	if (pr_headings) {
		printf("%-5s %-14s %-11s %-14s %-14s %-14s\n",
		"TYPE", "MACHINE", "REM_ID", "REM_NAME", "LOC_ID", "LOC_NAME");
		pr_headings = 0;
	}
	printf("\n");

	while (curr_mach != NULL) {
		if (mapping == UID_MAP)
			machtype = "USR";
		else
			machtype = "GRP";

		name = curr_mach->m_name;
		while (name != NULL) {
			if (EQ(name->l_val, "."))
				machname = "GLOBAL";
			else
				machname = name->l_val;

			if (curr_mach->m_def == 0)
				strcpy(machdef, "transparent");
			else
				sprintf(machdef, "%d", curr_mach->m_def);

			printf("%-5s %-14s %-11s %-14s %-14s %-14s\n",
			       machtype, machname, "DEFAULT", "n/a", machdef,
			       curr_mach->m_def == OTHERID? "guest_id":"n/a");

			list = curr_mach->m_nmlist;
			while (list != NULL) {
				if (EQ(list->nm_map->n_loc, "-")) {
					temp = list->idmap->i_rem;
					list = list->next;
					sprintf(mach_idrem, "%d-%d", temp, list->idmap->i_rem);
				} else {
					sprintf(mach_idrem, "%d", list->idmap->i_rem);
				}
				printf("%-5s %-14s %-11s %-14s %-14d %-14s\n",
				machtype, machname, mach_idrem,
				list->nm_map->n_rem, list->idmap->i_loc,
				list->nm_map->n_loc);
				list = list->next;
			}
			name = name->l_next;
		}
		curr_mach = curr_mach->m_next;
	}
	return(SUCCESS);
}

static
parse_err(key, token1, token2)
int	key;
char	*token1;
char	*token2;
{
	if (print_err_msg == 0)
		return;

	fprintf(stderr, "%s: line %d of %s: ", cmd_name, line, rulesfile);
	switch (key) {
	case P_ORDER:
		fprintf(stderr, "must specify host before %s\n", token1);
		break;
	case P_DEFORD:
		fprintf(stderr, "must specify default before map or exclude\n");
		break;
	case P_EXORD:
		fprintf(stderr, "must specify exclude before map\n");
		break;
	case P_NOVAL:
		fprintf(stderr, "must specify value of %s\n", token1);
		break;
	case P_XDEF:
		fprintf(stderr, "cannot have more than one default\n");
		break;
	case P_INVTOK:
		fprintf(stderr, "illegal token <%s>\n", token1);
		break;
	case P_RANGE:
		fprintf(stderr, "error in range of exclude\n");
		break;
	case P_RNONAME:
		fprintf(stderr, "cannot get name <%s> from <%s>\n", token1, token2);
		break;
	case P_LNONAME:
		fprintf(stderr, "cannot get name <%s> from local %s file\n", token1, mapping == UID_MAP? "passwd": "group");
		break;
	case P_MAPERR:
		fprintf(stderr, "warning: maped value of %s is within previously exclude range; map ignored\n", token1);
		break;
	case P_EXCERR:
		fprintf(stderr, "warning: value of %s previously excluded; exclud ignored\n", token1);
		break;
	case P_EXOVER:
		fprintf(stderr, "warning: excluded range inconsistent with previously excluded value; exclude ignored\n");
		break;
	case P_NODOM:
		fprintf(stderr, "host <%s> must be specified as domain.hostname\n", token1);
		break;
	case P_EXISTS:
		fprintf(stderr, "warning: value of %s previously mapped; map ignored\n", token1);
		break;
	case P_NUMONLY:
		fprintf(stderr, "must specify only numeric values for global mappings\n");
		break;
	case P_NOHOST:
		fprintf(stderr, "must specify host name\n");
		break;
	case P_DUPNAME:
		fprintf(stderr, "host <%s> previously mapped\n", token1);
		break;
	case P_NOROOT:
		fprintf(stderr, "cannot specify a value of 0 as default\n");
		break;
	case P_XGLOB:
		fprintf(stderr, "cannot have extra information on the global specification\n");
	}
}
