char mainxver[] = "@(#)main.c	1.1";		/* SCCS */

# include <stdio.h>
# include "structs.h"

main(argc,argv)
int	argc ;
char	*argv[] ;
{
	int	c ;
	int	i ;
	char	*p ;
	char	*m_alloc() ;
	char	*get_str() ;

	prog_name = argv[0] ;
	if ( argc != 2 )
		error("usage: %s descfile\n",prog_name) ;

	read_desc(argv[1]) ;

	lines = (struct line **)m_alloc(dev.paperlength*sizeof(struct line *)) ;
	line_pool = (struct line *)m_alloc(POOLSIZE*sizeof(struct line)) ;
	page_init() ;

	input_line_num = 1 ;

	djd_start(pde_name) ;
	while ( (c=getchar()) != EOF )
		switch(c)
		{
			case 's' :
				size = get_num() ;
				for ( i=0 ; i<dev.nsizes ; i++ )
					if ( sizes[i] == size )
						break ;
				if ( i == dev.nsizes )
					error("size %d not available\n",size) ;
				size = i ;
				break ;

			case 'f' :
				cur_font = font_map[get_num()] ;
				break ;

			case 'c' :
				norm_char(getchar()-32,cur_font) ;
				break ;

			case 'C' :
				funny_char(get_str()) ;
				break ;

			case 'D' :
				D() ;
				break ;

			case 'H' :
				xpos = get_num() ;
				break ;

			case 'V' :
				ypos = get_num() ;
				break ;

			case 'h' :
				xpos += get_num() ;
				break ;

			case 'n' :
				get_num() ;
				get_num() ;
				break ;

			case 'p' :
				page = get_num() ;
				ypos = 0 ;
				put_page() ;
				break ;

			case 'w' :
				break ;

			case 'v' :
				ypos += get_num() ;
				break ;

			case 'x' :
				x() ;
				break ;

			case '0' :
			case '1' :
			case '2' :
			case '3' :
			case '4' :
			case '5' :
			case '6' :
			case '7' :
			case '8' :
			case '9' :
				xpos += 10*(c-'0') + getchar() - '0' ;
				norm_char(getchar()-32,cur_font) ;
				break ;

			case '\n' :
				input_line_num++ ;
				break ;

			case '\t' :
			case ' ' :
				break ;

			case '#' :
				arctest(889,275,964,275,889,275) ;
				break ;

			default :
				error("unknown troff directive: %c(0%o)\n",c,c) ;
		}

	put_page() ;
	djd_end() ;
	exit(0) ;
}
x()
{
	int	c ;
	char	*get_str() ;

	c = *get_str() ;
	switch(c)
	{
		case 'i' :
			break ;

		case 'T' :
			get_str() ;
			break ;

		case 'r' :
			get_num() ;
			get_num() ;
			get_num() ;
			break ;

		case 'p' :
			error("help! no info for case p\n") ;
			break ;

		case 's' :
			break ;

		case 't' :
			break ;

		case 'f' :
		{
			int	i ;
			int	index ;
			char	*name ;

			index = get_num() ;
			name = get_str() ;
			for ( i=0 ; i<dev.nfonts ; i++ )
				if ( strcmp(name,Fonts[i].font.namefont) == 0 )
					break ;
			if ( i == dev.nfonts )
				error("font %s not mounted\n",name) ;
			font_map[index] = i ;
			break ;
		}

		case 'H' :
			get_num() ;
			break ;

		case 'S' :
			get_num() ;
			break ;

		default :
			error("bad device control: %c(0%o)\n",c,c) ;
	}
}
D()
{
	int	c ;
	int	dx ;
	int	dy ;
	int	dx2 ;
	int	dy2 ;
	int	r ;
	char	*get_str() ;

	c = *get_str() ;
	switch(c)
	{
		case 'l' :
			dx = get_num() ;
			dy = get_num() ;
			get_str() ;
			draw_line(dx,dy) ;
			break ;

		case 'c' :
			circle(get_num()) ;
			break ;

		case 'e' :
			dx = get_num() ;
			dy = get_num() ;
			ellipse(dx,dy) ;
			break ;

		case 'a' :
			dx = get_num() ;
			dy = get_num() ;
			dx2 = get_num() ;
			dy2 = get_num() ;
			arc(dx,dy,dx2,dy2) ;
			break ;

		case '~' :
			while ( getchar() != '\n' )
				;
			break ;

		default :
			error("bad drawing specification: %c(0%o)\n",c,c) ;
	}
}
