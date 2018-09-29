/* SCCS @(#)structs.h	1.3 */

# include "dev.h"

# define NEL(x)		(sizeof(x)/sizeof(x[0]))
# define min(x,y)	((x)<(y)?(x):(y))

# define unsign		unsigned

# define BUFSIZE	100
# define MAX_LINE_CHARS	130
# define PLOTFLAG	-1
# define PLOTINDEX	'!'
# define POOLSIZE	1500
# define STRIPSIZE	150
# define WIDTH_INCR	25

# define NOFFSETS	1
# define NSLOPES	2
# define NVERTICES	10
# define P_RES		2

struct	chars
{
	char	character ;
	short	width ;
	short	position ;
} ;

struct	line
{
	short		char_count ;
	short		max_count ;
	char		fontindexchar ;
	struct	chars	*chars ;
	struct	line	*next_line ;
} ;

struct	charinfo
{
	char	width ;
	char	kerning ;
	char	code ;
} ;

extern	struct	Font
{
	struct	font		font ;
	struct	charinfo	*info ;
	unsign	char		*fitab ;
} *Fonts ;

extern	int		errno ;

extern	int		size ;
extern	int		cur_font ;
extern	int		xpos ;
extern	int		ypos ;
extern	int		page ;

extern	struct	dev	dev ;
extern	short		*sizes ;
extern	short		*indices ;
extern	char		*charnames ;

extern	struct	line	**lines ;
extern	struct	line	*line_pool ;
extern	int		line_avail ;

extern	int		input_line_num ;
extern	int		font_map[15] ;
extern	char		pde_name[BUFSIZE] ;

extern	char		*prog_name ;

extern  int		plotmap[NOFFSETS][NSLOPES][NVERTICES][NVERTICES] ;
