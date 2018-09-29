/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)help:interact.c	1.17"

/*  Preprocessor Include Files  */
#include <stdio.h>
#include <curses.h>
#include <fcntl.h>
#include <errno.h>
#include <term.h>
#include <string.h>
#include <signal.h>
#include "switch.h"

/*  Preprocessor Macros  */
#define EQ(x, y)	(strcmp(x, y) == 0)
#define NE(x, y)	(strcmp(x, y) != 0)
#define	TOGGLE(sw, on, off)	if(sw) { sw = 0; putp(off); } else { sw = 1; putp(on); }
#define OPEN(fp, file, mode)	if((fp = fopen(file, mode)) == NULL) \
				{ fprintf(stderr, "%s:  can't open %s,\n", Cmd, file); exit(1); }
#define BOOLEAN		short
#define EXISTS(path)	(access(path, 0)==0)
#define CANWRIT(path)	(access(path, 2)==0)

/*  Strings  */
#define MENU_CHAR	'_'
#define ENTER_STRING	"Enter choice > "
#define ERROR_STRING	"\\S %r \\S is an invalid choice.  Enter a choice shown above."
#define WRITE_ERROR	"Cannot write on file \\S %r \\S."
#define DIR_ERROR	"Cannot create files in directory \\S %r \\S."
#define NDIR_ERROR	"Directory \\S %r \\S does not exist."
#define CREAT_ERROR	"Cannot create file \\S %r \\S."
#define NEXT_STRING	"\\S n \\S(next pg)"
#define BACK_STRING	"\\S b \\S(back 1 pg)"
#define HELP_STRING	", \\S h \\S(restart help)"
#define LOCATE_STRING	", \\S l \\S(back to locate)"
#define QUIT_STRING	", \\S q \\S(quit)"
#define MORE_STRING	"MORE"

/*  Response Results  */
#define BACKUP		1
#define NEXTPG		2
#define REDRAW		3
#define VALID		4
#define	INVALID		5
#define REPROMPT	6
#define REDIRECT	7

/*  Menu Type Mnemonics  */
#define NON_PAGED	100
#define BACK_1		101
#define BACK_2		102
#define BN_1		103
#define BN_2		104
#define BN_3		105
#define NEXT_1		106
#define NEXT_2		107

/*  Page Data Structure  */
typedef struct page {
	long		seek_point;
	int		no_of_lines;
	int		menu_type;
	struct page	*next;
	struct page	*last;
} pagenode, *pageptr;

/*  Global Variables  */
int	redirect = 0;
int	endflag = 0;
int	npageflg = 0;
int	moreflag = 0;
char	*Cmd;
char	*First_name;
char	Buf[BUFSIZ];
char	errormsg[79];
char	Command[79];
char	Keywords[79];
long	tmpseek_point;
BOOLEAN	Scroll;
struct {
	char	name[14];
	char	last_name[14];
	char	command[256];
	char	menu[256];
	int	menu_type[3];
	int	menu_length[4];
	char	prompt[79];
	char	error[79];
	char	response[79];
	char	last_response[79];
	int	(*filter)();
	int	input_size;
	pageptr	page_ptr;
} Display;
FILE	*Tty_fp;
FILE	*Text_fp;
FILE	*Disp_fp;
FILE	*Resp_fp;
FILE	*Log_fp;
FILE	*rptr;

/*  Internal function declarations  */
void	get_options();
void	initialize();
void	get_entry();
char	*expand();
void	usage();
int	display();
int	prompt();
void	build_menu();
int	text();
void	menu();
int	check_response();
void	escape_to_shell();
void	output();
int	count();
pageptr	newpage();
int	file_redirect();
int	pipe_redirect();
int	reset();

/*  External functions decalarations  */
char	*fgets();
char 	*strrchr();
char	*getcwd();


/*The program interact, when supplied with the appropriate information,
will manage a user's session from beginning to end.
The hook between interact and the appropriate information needed for
the user's interaction is called an interaction name.
The interaction name is the key into a table of interaction information,
called the display table.
The display table contains commands, either static or dynamic, menu
entries and prompt and error information for all interaction names
available in the help facility.
This information is used as the input to the interact program.

Another table, which is used to validate user's responses in the help facility,
is the response table.
The response table consists of entries with
three sections;  response, current interaction name, and new interaction
name.
Every possible response in the facility is included in this table.
If anything other than what is in the table is typed, an error will be
printed.

Interact has two filters that can be used, display and
prompt.  Display prints the full screen and prompt
redraws the prompt line leaving the screen intact.*/

main(argc, argv)
int	argc;
char	**argv;
{
	FILE	*pipe_fp;

	/* Check to see if output is redirected from shell level */
	if(isatty(1) != 1)
		redirect=1;

	/*  Get name of command  */
	Cmd = argv[0];

	/*  Get command-line options  */
	get_options(argc, argv);

	/* Open users tty to read responses from */
	Tty_fp = stdin;

	/* Set up terminal */
	if(redirect != 1)
		setupterm(0, 1, 0);

	/*  Loop until the interaction name is "quit"  */
	while(NE(Display.name, "quit")) {
		initialize();
		get_entry();
		if((pipe_fp = popen(Display.command, "r")) == NULL) {
			fprintf(stderr, "%s:  can't create input for display handler.\n", Cmd);
			exit(1);
		}
		Text_fp = tmpfile();
		for(Display.input_size = 0; fgets(Buf, BUFSIZ, pipe_fp) != NULL;Display.input_size++) 
			fputs(Buf, Text_fp);
		rewind(Text_fp);
		pclose(pipe_fp);
		(*Display.filter)();
		fclose(Text_fp);
		if(redirect == 1)
			break;
	}

	/*  Reset terminal  */
	if(redirect != 1)
		resetterm();

	/*  Leave Gracefully  */
	exit(0);
}


/*The function get_options is called from the main function
and is passed, as input, the number of arguments and the contents
of the command line.
Get_options then parses the command line, initializes the
appropriate variables and opens appropriate display and
response table entries.
Nothing is returned from this function.*/

void
get_options(argc, argv)
int	argc;
char	**argv;
{
	extern int	optind;
	extern char	*optarg;
	static int	c;

	if(argc == 1)
		usage();

	while((c = getopt(argc, argv, "sd:r:l:")) != EOF) {
		switch(c) {
		case 's':
			Scroll = TRUE;
			break;
		case 'd':
			OPEN(Disp_fp, optarg, "r");
			break;
		case 'r':
			OPEN(Resp_fp, optarg, "r");
			break;
		case 'l':
			Log_fp = fopen(optarg, "a");
			break;
		case '?':
			usage();
		}
	}
	strcpy(Display.name, (First_name = argv[optind]));
	if (EQ(Display.name, "define")) {
		char *temp;
		if ((temp = strtok(argv[optind+1]," \t\n")) == NULL) {
			argv[optind+1][79] = '\0';
		}
		else {
			argv[optind+1] = temp;
		}
	}
	strcpy(Display.response, argv[++optind]);
	SWITCH(First_name)
	CASE3("getdesc","getexample","getoption")
		strcpy(Command, Display.response);
	CASE("keysrch")
		strcpy(Keywords, Display.response);
	DEFAULT
		strcpy(Display.last_response, Display.response);
	ENDSW
}


/*The function initialize is called from the main function
and is not passed any input.  This function initializes
the prompt and error strings, sets the interaction filter name and nulls
out page attributes.
Nothing is returned from this function.*/

void
initialize()
{
	if(EQ(Display.name, "help"))
		Keywords[0] = '\0';
	Display.filter = display;
	Display.menu[0] = Display.command[0] = '\0';
	Display.menu_length[0] = 0;
	strcpy(Display.prompt, ENTER_STRING);
	strcpy(Display.error,ERROR_STRING);
	Display.page_ptr = NULL;
	endflag = npageflg = 0;
}


/*The function get_entry is called from the main function and
is not passed any input.
Get_entry reads through the display table looking for
a match to the interaction name.  Once a match is found, all
information associated with the interaction name is gathered.
If no match is found, an error message is printed and the program exits.
This function
does not return anything.*/

void
get_entry()
{
	BOOLEAN	found_name = FALSE;

	/*  Rewind display table  */
	rewind(Disp_fp);

	/*  Look for name in table and get everything associated with it  */
	while(fgets(Buf, BUFSIZ, Disp_fp) != NULL) {
		Buf[strlen(Buf) - 1] = '\0';
		SWITCH(Buf)
		PCASE("name:")
			if(found_name)
				break;
			else if(EQ(&Buf[5], Display.name))
				found_name = TRUE;
		PCASE("filter:")
			if(found_name && EQ(&Buf[7], "prompt"))
				Display.filter = prompt;
		PCASE("command:")
			if(found_name)
				strcpy(Display.command, expand(&Buf[8]));
		PCASE("menu:")
			if(found_name)
				strcpy(Display.menu, expand(&Buf[5]));
		PCASE("prompt:")
			if(found_name)
				strcpy(Display.prompt, expand(&Buf[7]));
		PCASE("error:")
			if(found_name)
				strcpy(Display.error, &Buf[6]);
		ENDSW
	}
	if(!found_name) {
		fprintf(stderr, "%s:  interaction name %s is not in display table.\n", Cmd, Display.name);
		exit(1);
	}
}


/*The function expand is called from the get_entry and prompt
functions.  It is passed the string that may need to have expansion
performed on it.
In a shell script, variables such as $HOME are expanded by the shell.
Expand does the same thing.  There are six cases, each
preceded by a %.  An n means the current name; N, previous
name; r, current response; R, previous response; c, command
name and k, keywords.  These symbols all reside in the display
table.  This function returns the original string with any expansions
performed included.*/

char *
expand(string)
char	*string;
{
	static int	i, j, k;
	static char	*p, buf[BUFSIZ];

	strcpy(buf, string);
	for(i = 0; buf[i] != '\0'; i++) {
		if(buf[i] == '%') {
			switch(buf[i + 1]) {
			case 'n':	/*  Current name  */
				p = Display.name;
				break;
			case 'N':	/*  Previous name  */
				p = Display.last_name;
				break;
			case 'r':	/*  Current response  */
				p = Display.response;
				break;
			case 'R':	/*  Previous response  */
				p = Display.last_response;
				break;
			case 'c':	/*  Command name  */
				p = Command;
				break;
			case 'k':	/*  Keywords  */
				p = Keywords;
				break;
			default:
				continue;
			}
			k = strlen(p);
			switch(k) {
			case 0:
			case 1:
				for(j = i + k; buf[j + (2 - k)] != '\0'; j++)
					buf[j] = buf[j + (2 - k)];
				buf[j] = '\0';
				break;
			case 2:
				break;
			default:
				for(j = strlen(buf) + 1; j > i + 1; j--)
					buf[j + k - 2] = buf[j];
			}
			while(*p != '\0')
				buf[i++] = *(p++);
			i--;
		}
	}
	return(buf);
}


/*The function display is called from the main function and is 
passed no input.
Display controls the printing of the screen and the menu.  It
determines how many lines are needed for the text space
of the screen.
The number used for the initial screen
length is the variable lines in the curses/terminfo database.
If the text does not fill more than one screen, then it is displayed
with its corresponding menu and a response is requested from the user.
If the user escaped to the shell to execute a command, the screen will be
redrawn.  If the response is valid, the function returns,
otherwise, an error message is printed and the user is reprompted.

If the text will fill up more than one screen, then a data structure
is populated to keep track of various attributes of a page (e.g., text size,
menu type and starting address).  Pages are printed as long as the user's
response is valid.  If the user responds with a b for back-up or n
for next, display will print the previous or next screen respectively.
If the response is n, then the new page attributes are populated.  
The population of the data structure is only done once for each page.  Checking
is done to determine if it has been populated already.  If it has, then
pointers are automatically set.  Before leaving the function, if any
error messages were printed because of invalid responses, then the message
is cleared, and the page linked list is freed of its space.
Nothing is returned from display.*/

int
display()
{
	static int	result, text_size;
	static pageptr	head, ptr;

	/*  Build menus  */
	if(redirect != 1)
		build_menu();

	/*  Determine size of text area  */
	if(redirect == 1)
		text_size = -1;
	else
		text_size = hard_copy ? -1 : lines - Display.menu_length[0] - 2;

	/*  Display the non-paged screen  */
	while(Display.input_size <= text_size || text_size == -1) {
		npageflg = 1;
		text(0, text_size);
		if(redirect != 1){
			menu(NON_PAGED);
			if(prompt() != REDRAW)
				return;
		} else
			return;
	}

	/*  Create the first page node  */
	head = Display.page_ptr = newpage();
	Display.page_ptr->seek_point = 0;
	Display.page_ptr->menu_type = Display.menu_type[0];
	Display.page_ptr->no_of_lines = lines - Display.menu_length[1] - 2;
	Display.page_ptr->next = Display.page_ptr->last = NULL;

	/*  Display the paged screen  */
	result = INVALID;
	while(result != VALID) {
		moreflag = 0;
		if(text(Display.page_ptr->seek_point, Display.page_ptr->no_of_lines))  {
			moreflag = 1;
			if(endflag == 0) {
				endflag = 1;
				Display.page_ptr->menu_type = Display.menu_type[2];
				Display.page_ptr->no_of_lines = lines - Display.menu_length[3] - 2;
			}
		}
		menu(Display.page_ptr->menu_type);
		if((result = prompt()) == BACKUP)
			Display.page_ptr = Display.page_ptr->last;
		else if(result == NEXTPG) {
			if(Display.page_ptr->next == NULL) {
				ptr = newpage();
				ptr->seek_point = ftell(Text_fp);
				ptr->last = Display.page_ptr;
				ptr->next = NULL;
				Display.page_ptr->next = ptr;
				ptr->menu_type = Display.menu_type[1];
				ptr->no_of_lines = lines - Display.menu_length[2] - 2;
			}
			Display.page_ptr = Display.page_ptr->next;
		}
	}

	/*  Free page linked list  */
	Display.page_ptr = head;
	while(Display.page_ptr->next != NULL) {
		Display.page_ptr = Display.page_ptr->next;
		free(Display.page_ptr->last);
	}
	free(Display.page_ptr);
	
}



/*The function prompt is called from the main and
the display functions.
It is not passed any input.
Prompt redraws the prompt line with either a new prompt or the same
prompt.  It leaves the current text space information intact.
Prompt then reads the user's input and has it validated by calling check_response.
If the response is invalid and the logging
mechanism is turned on, then the response, interaction name and an error message
are printed in the specified logfile.  Then a bell is sounded and an error 
message is printed on the last line of the screen and the user is reprompted.
.P
If the response is valid and the logging mechanism is turned on, then the
response and interaction name are added to the logfile, otherwise, the error
message is cleared if a previous one occurred and the response is returned.
The function returns a valid user's response.*/

int
prompt()
{
	static int	result;

	strcpy(Display.last_response, Display.response);
	while(TRUE) {
		putp(tparm(cursor_address, lines - 2, 0));
		putp(clr_eol);
		output(Display.prompt,stdout);
		fgets(Display.response, 79, Tty_fp);
		if(result == REDIRECT && Display.response[0] == '\n') {
			strcpy(Display.prompt, ENTER_STRING);
			continue;
		}
		Display.response[strlen(Display.response) - 1] = '\0';
		putp(clr_eol);
		fflush(stdout);
		result = check_response();
		if(Log_fp != NULL)
			fprintf(Log_fp, "name=%s\tresponse='%s'\tstatus=%s\n",
				Display.name, Display.response,
				result == INVALID ? "ERROR" : "OK");
		if(result == REDIRECT) {
			strcpy(Display.prompt, "Enter > file, | cmd(s), or RETURN to continue > ");
			continue;
		}
		else if(result == REPROMPT)
			continue;
		else if(result != INVALID)
			return(result);
		else {
			putp(bell);
		}
		if(strlen(Display.response) == 1)
			strcpy(errormsg, ERROR_STRING);
		else
			strcpy(errormsg, Display.error);
		output(expand(errormsg),stdout);
		if (hard_copy)
			putchar('\n');
	}
}


/*The function check_response is called from the prompt function.
It is not passed any input.
Check_response validates the user's response and saves
the last interaction name.
Check_response calls 
escape_to_shell if the response begins with an "!".  After
the command finishes
it returns with a response to redraw the screen.
If a user typed a b or n, check_response validates that
they are allowed.  It also checks to see if h for help is
a valid response.
If the user did not enter the help facility through the command help,
then h is not valid.
Check_response then reads through the response table until it matches
the response.  If the second field (interaction name) is not the same
as the current interaction name, then the response is invalid.
If the names do match, then the third field is retrieved and used as the next
interaction name.  
There are multiple-character builtin responses that are validated also.
This function returns valid or invalid.*/

int
check_response()
{
	static int	namelen, resplen, i;
	static int	ans;
	char *lptr, *ptr, *comptr, *intptr;
	char SBuf[BUFSIZ];
	static int	len;

	/*  Save last interaction name  */
	strcpy(Display.last_name, Display.name);

	/*  Handle Shell Escape Response  */
	if(Display.response[0] == '!') {
		escape_to_shell(&Display.response[1]);
		return(REDRAW);
	}

	/* Handle Redirection Responses */

	if(Display.response[0] == '>') {
		file_redirect(strtok(&Display.response[1], " \t\n"));
		reset();
		return(REPROMPT);
	}

	if(Display.response[0] == '|' &&  ans == REDIRECT) {
		pipe_redirect(&Display.response[1]);
		reset();
		ans=0;
		return(REDRAW);
	}

	/*  Handle Single-character Builtin Responses  */
	if(Display.response[1] == '\0') {
		switch(Display.response[0]) {
		case 'q':
			strcpy(Display.name, "quit");
			return(VALID);
		case 'h':
			if(EQ(First_name, "help") && (NE(Display.name, "help"))) {
				strcpy(Display.name, "help");
				return(VALID);
			} else
				return(INVALID);
		case 'b':
			if(Display.page_ptr == NULL || Display.page_ptr->last == NULL)
				return(INVALID);
			else
				return(BACKUP);
		case 'n':
			if(Display.page_ptr == NULL || Display.page_ptr->menu_type == BACK_1 || Display.page_ptr->menu_type == BACK_2)
				return(INVALID);
			else
				return(NEXTPG);
		case 'r':
			ans=REDIRECT;
			return(REDIRECT);
		}
	}

	/*  Handle Multiple-character Builtin Responses  */
	SWITCH(Display.name)
	CASE("getkey")
		if(Display.response[0] == '\0')
			return(INVALID);
		strcpy(Keywords, Display.response);
		strcpy(Display.name, "keysrch");
		return(VALID);
	CASE("define")
		strcpy(Display.name, "glossary");
	CASE("getdoe")
		strcpy(Command, Display.last_response);
	ENDSW

	/*  Look for Key in Response Table  */
	len = 0;
	resplen = strlen(Display.response);
	namelen = strlen(Display.name);
	rewind(Resp_fp);
	while(fgets(Buf, BUFSIZ, Resp_fp) != NULL) {
		Buf[strlen(Buf) - 1] = '\0';
		if(strncmp(Buf, Display.response, resplen) == 0 && Buf[resplen] == ':'){
			if((ptr = strchr(&Buf[resplen + 1], ',')) == NULL){
				if(strncmp(&Buf[resplen + 1], Display.name, namelen) == 0){
 					if(EQ(Display.response,"l") && (NE(Display.name,"help")) && ( Keywords[0] == '\0') && (NE(Display.name,"starter"))){
 						return(INVALID);
 					}
					strcpy(Display.name, &Buf[resplen + namelen + 2]);
					if(strncmp(Display.name, "define", 6) == 0){
						strcpy(Display.response, Display.name);
						Display.name[6] = '\0';
					}
					return(VALID);
				}
			}
			else {
				strcpy(SBuf, Buf);
				lptr = &Buf[resplen + 1];
				while((comptr = strtok(lptr, ",:")) != NULL){
					if(strncmp(comptr, Display.name, namelen) == 0){
						intptr = strrchr(SBuf, ':');
						strcpy(Display.name, ++intptr);
						return(VALID);
					}
					else {
						lptr = NULL;
						len = strlen(comptr) + len + 1;
						if(SBuf[resplen + len] == ':')
							break;
					}
				}
			}
		}
	}
	return(INVALID);
}


/*The build_menu function is called from the display function.
Build_menu breaks up the menu into physical lines.  It counts
through the original menu up to 79.  Once it hits 79, then it backs up to the
last comma and adds a new line one past the comma.  It continues to do this
through the string.  A line count is kept.  If the screen needs multi-
paging, then a n and b must be added on the correct pages.  
If the number of characters in the n option is added to the number
of characters on the last line of the original menu and the total is greater
than 79, then the n option must be put on a lines by itself,
otherwise it will be added to the last line of the menu.
If it does fit, the same thing is done with the b
option.  If the line with the n and b options is greater than
79 then the b must be on its own line.  If the n option does
not fit on the last line, then the n and b options will be on
a separate line.
The last test is for just the b option.  If it is added to the last line
and the count is greater than 79, then the b option is on its own line,
otherwise it will be added to the last line of the menu.  
Two arrays are populated with the menu type for each possible menu and the
length in lines for each possible menu.  Build_menu does not return
anything.*/

void
build_menu()
{
	static int cnt, i;

	/*  Return if no menu  */
	if(Display.menu[0] == '\0')
		return;

	if((EQ(Display.name, "getdesc") ||
		EQ(Display.name, "getoption") ||
		EQ(Display.name, "getexample")) && Keywords[0] != '\0')
		strcat(Display.menu, LOCATE_STRING);
	if(EQ(First_name, "help"))
		strcat(Display.menu, HELP_STRING);
	strcat(Display.menu, QUIT_STRING);
	for(Display.menu_length[0] = 3, cnt = i = 0; Display.menu[i] != '\0'; i++) {
		if(Display.menu[i] == '\\')
			i++;
		else if(cnt++ > 79) {
			for( ; Display.menu[i] != ','; i--)
				;
			Display.menu[++i] = '\n';
			Display.menu_length[0]++;
			cnt = 0;
		}
	}
	for(i = 1; i < 4; i++)
		Display.menu_length[i] = Display.menu_length[0];
	if(count(NEXT_STRING) + cnt + 2  <= 79) {
		Display.menu_type[0] = NEXT_1;
		if(count(NEXT_STRING) + count(BACK_STRING) + cnt + 2 <= 79)
			Display.menu_type[1] = BN_1;
		else {
			Display.menu_type[1] = BN_2;
			Display.menu_length[2] += 1;
		}
	}
	else {
		Display.menu_type[0] = NEXT_2;
		Display.menu_type[1] = BN_3;
		Display.menu_length[1] = Display.menu_length[2] += 1;
	}
	if(count(BACK_STRING) + cnt + 2 <= 79)
		Display.menu_type[2] = BACK_1;
	else {
		Display.menu_type[2] = BACK_2;
		Display.menu_length[3] += 1;
	}
}


/*The function text is called from the display function.
The function is passed two arguments;  where to start in the file and how 
many lines to print.
Text reads through the file for the specified number of lines
sending the text to the
output function to be
printed.  If the scrolling option is not turned on then the screen is cleared
first.
If the text
does not fill up the screen, the rest will be padded with new-lines.
If the last line of the text ends up on the last line of the text space,
then two checks are done.
If the next character in the file is the EOF, then a different menu must
be printed.  A check must be done to see if the difference between the two
menu lengths is equal to one.  If it is then a newline must be
printed, otherwise, it does nothing.
If the terminal is a hardcopy, then all the text is sent,
disregarding boundaries.  This function returns either a 0 or an EOF if the
end-of-file has occurred.*/

int
text(start, total_lines)
long	start;
int	total_lines;
{
	static char	*ptr;
	static int	i;
	int c;

	if(total_lines == -1)
		while((ptr = fgets(Buf, BUFSIZ, Text_fp)) != NULL)
			output(Buf,stdout);
	else {
		if(!Scroll)
			putp(clear_screen);
		fseek(Text_fp, start, 0);
		for(i = 1; i <= total_lines; i++) {
			if((ptr = fgets(Buf, BUFSIZ, Text_fp)) != NULL)
				output(Buf,stdout);
			else
				putchar('\n');
		}
	}

	if(npageflg == 1)
		return;

	if(endflag == 0) {
		if((c=getc(Text_fp)) == EOF) {
			ptr = NULL;
			if(Display.menu_length[2] - Display.menu_length[3] == 1)
				putchar('\n');
		}
		else
			ungetc(c, Text_fp);
	}
	return(ptr == NULL ? EOF : 0);
}


/*The function newpage is called from the display
function.  No arguments are passed to this function.
Newpage allocates the space needed for the new page structure.
If no space can be allocated, then an error message is printed
stating this fact.
A pointer to the allocated space is returned.*/

pageptr
newpage()
{
	static pageptr	ptr;

	if((ptr = (pageptr) malloc((unsigned) sizeof(pagenode))) == NULL) {
		fprintf(stderr, "%s:  not enough space to allocate page structure.\n");
		exit(1);
	}
	return(ptr);
}


/*The menu function is called from the display function.  It is
passed the menu type to print.
Menu prints the original menu, surrounded by underscores.
If the n or b options are needed, then menu will print the 
appropriate menu.
Nothing is returned from menu.*/

void
menu(type)
int	type;
{
	static int	i, n;

	if(Display.menu[0] != '\0') {
		for(i = 0; i < 64; i++)
			putchar(MENU_CHAR);
		if(npageflg != 1) {
			if(moreflag == 0) {
				output(MORE_STRING,stdout);
				n = 11;
			}
			else
				n = 15;
		}
		else
			n = 15;

		for(i = 0; i < n; i++)
			putchar(MENU_CHAR);
		putchar('\n');
		output(Display.menu,stdout);
		if(type ==  NEXT_1 || type == BN_1 || type == BN_2) {
			putchar(',');
			putchar(' ');
			output(NEXT_STRING,stdout);
		}
		if(type == NEXT_2 || type == BN_3) {
			putchar(',');
			putchar('\n');
			output(NEXT_STRING,stdout);
		}
		if(type == BACK_1 || type == BN_1 || type == BN_3) {
			putchar(',');
			putchar(' ');
			output(BACK_STRING,stdout);
		}
		if(type == BACK_2 || type == BN_2) {
			putchar(',');
			putchar('\n');
			output(BACK_STRING,stdout);
		}
		putchar('\n');
		for(i = 0; i < 79; i++)
			putchar(MENU_CHAR);
		putchar('\n');
	}
	putchar(Scroll ? '\n' : '\0');
}


/*The function output is called from the prompt,
escape_to_shell, text and menu functions.
It is passed the line that is to be printed.
Output prints the lines and handles any
highlighting needed.  The three modes supported are bold, standout and
underline.  Only standout mode is used in the facility.  The highlighting
is done through curses/terminfo.
Output does not return anything.*/

void
output(line,stream)
char *line;
FILE *stream;
{
	static int i;		/* temporary variables */
	BOOLEAN so_mode = 0;	/* standout mode toggle */
	BOOLEAN ul_mode = 0;	/* underline mode toggle */
	BOOLEAN bold_mode = 0;	/* bold mode toggle */

	for(i = 0; line[i] != '\0'; i++) {
		if(line[i] == '\\') {
			switch(line[++i]) {
			case 'B':		/* Enter/Exit Bold Mode */
				if(redirect != 1)
					TOGGLE(bold_mode, enter_bold_mode, exit_attribute_mode)
				continue;
			case 'S':		/* Enter/Exit Standout Mode */
				if(redirect != 1)
					TOGGLE(so_mode, enter_standout_mode, exit_standout_mode)
				continue;
			case 'U':		/* Enter/Exit Underline Mode */
				if(redirect != 1)
					TOGGLE(ul_mode, enter_underline_mode, exit_underline_mode)
				continue;
			}
			putc(line[i - 1], stream);
		}
		putc(line[i], stream);
	}
}


/*The escape_to_shell function is called from the check_response
function.
It is passed the command that should be executed.
Escape_to_shell handles the user's request to execute a command
via the shell.  Once the command is executed, the user is prompted to press
return to continue.  If they wish to execute another command, then escape_to_shell
is called again.  Otherwise, if they type anything but a new-line, a 
bell will be sounded indicating an error.
This function does not return anything.*/


void
escape_to_shell(cmd)
char	*cmd;
{
	static int	(*oldhup)(), (*oldquit)(), (*signal())(), (*savint)();
	static int	pid, rpid, retcode;

	oldhup = signal(SIGHUP, SIG_IGN);
	oldquit = signal(SIGQUIT, SIG_IGN);
	resetterm();
	if((pid = fork()) == 0) {
		signal(SIGHUP, oldhup);
		signal(SIGQUIT, oldquit);
		execl("/bin/sh", "sh", "-c", cmd, (char *)0);
		exit(0100);
	}
	if (pid > 0) {
		savint = signal(SIGINT, SIG_IGN);
		while((rpid = wait(&retcode)) != pid && rpid != -1);
		signal(SIGINT, savint);
	}
	signal(SIGHUP, oldhup);
	signal(SIGQUIT, oldquit);
	fixterm();

	putchar('!');
	while(TRUE) {
		output("\n\\SPress <RETURN> to continue\\S:  ", stdout);
		fgets(Buf, BUFSIZ, Tty_fp);
		if(Buf[0] == '\n')
			break;
		else
			putp(bell);
	}
}


/*The function count is called from the build_menu function.
It is passed as input a string.
Count counts the number of characters in the string, ignoring any
highlighting modes.  This function returns the number of characters in the
string.*/

int
count(str)
char *str;
{
	static char	*cptr;
	static int	ctr;

	for(ctr = 0, cptr = str; *cptr != '\0'; cptr++) {
		if(*cptr == '\\')
			cptr++;
		else
			ctr++;
	}
	return(ctr);
}


/*The function usage is called from the get_options function and
not passed any input.
Usage prints out a usage message if any bad options
are used.
The function exits the program when finished.*/
void
usage()
{
	fprintf(stderr,"%s usage:  %s [options] [first name] [first response]\n\
	options available:\t-d<display table name>\n\
				-r<response table name>\n\
				-l<session log file name>\n\
				-s   scroll display\n", Cmd, Cmd);
	exit(1);
}



/*The function file_redirect is called from check_response and is passed
as input the name of the file to be opened for input.  If successfully
opened the data for the current interaction is copied to the
output file.*/

int
file_redirect(file)
char	*file;
{
		char sbuf[BUFSIZ];
		char *i;
		int fd;
		char *cwd;

		tmpseek_point=ftell(Text_fp);
		if ((fd=open(file, O_WRONLY|O_CREAT|O_TRUNC, 0644)) == -1) {
			if(errno == EACCES) {
					/* file mode okay? */
				if(EXISTS(file) && !CANWRIT(file)) {
					strcpy(Display.response, file);
					strcpy(errormsg, WRITE_ERROR);
					output(expand(errormsg), stdout);
					return;
				}

				/* directory mode okay? */
				i = strrchr(file, '/');
			/* Check to see if current directory */
				if(i == NULL) {
					i = file;
					*i++ = '.';
					*i = '\0';
				} else if (i != file)
					*i = '\0';

				if (!CANWRIT(file)) {
					if(strcmp(file, ".") == 0) {
						cwd=getcwd((char *)NULL, 64);
						strcpy(Display.response, cwd);
					} else
						strcpy(Display.response, file);
					strcpy(errormsg, DIR_ERROR);
					output(expand(errormsg), stdout);
					return;
				}
			} else if(errno == ENOTDIR){
				if((i=strrchr(file, '/')) != NULL) {
					i='\0';
					strcpy(Display.response, file);
					strcpy(errormsg, NDIR_ERROR);
					output(expand(errormsg), stdout);
					return;
				}
			} else{
				strcpy(Display.response, file);
				strcpy(errormsg, CREAT_ERROR);
				output(expand(errormsg), stdout);
				return;
			}
		} else if((rptr=fdopen(fd, "w")) == NULL) {
				output("help: cannot open output redirection stream", stdout);
				return;
			}

		rewind(Text_fp);

		while(fgets(sbuf, BUFSIZ, Text_fp) != NULL) 
			output(sbuf,rptr);
		fclose(rptr);
		close(fd);
}



/*The function pipe_redirect is called from check_response and is passed
as input the command name that will be executed on the data for
the current interaction.  Once the command is executed the screen will
be redrawn.*/

int
pipe_redirect(pipe)
char *pipe;
{
		char pbuf[BUFSIZ];

		tmpseek_point=ftell(Text_fp);

		if((rptr=popen(pipe, "w")) == NULL) {
			output("help: cannot open output pipe",stdout);
			return;
		}

		rewind(Text_fp);

		while(fgets(pbuf, BUFSIZ, Text_fp) != NULL)
			output(pbuf, rptr);
		pclose(rptr);

		while(TRUE) {
			output("\n\\SPress <RETURN> to continue\\S:  ", stdout);
			fgets(Buf, BUFSIZ, Tty_fp);
			if(Buf[0] == '\n')
				break;
			else 
				putp(bell);
		}
}



/*The function reset is called from check_response and is not passed any
input.  It resets the prompt to the previous one 
and moves the pointer back to where it was in the file before
any redirection was done.*/

int
reset()
{
	strcpy(Display.prompt, ENTER_STRING);
	fseek(Text_fp, tmpseek_point, 0);
}
