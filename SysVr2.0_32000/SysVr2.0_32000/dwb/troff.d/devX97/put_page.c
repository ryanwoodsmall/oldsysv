char putxver[] = "@(#)put_page.c	1.1";		/* SCCS */

# include <stdio.h>
# include "structs.h"

put_page()
{
	int		i ;
	int		j ;
	int		x_current ;
	int		line_chars ;
	int		i_last ;
	int		char_count ;
	int		carriage ;
	struct	line	*p ;
	int		compare() ;
	char		v_space() ;


	i_last = -STRIPSIZE ;
	for ( i=0 ; i<dev.paperlength ; i++ )
		if ( lines[i] != NULL )
		{
			carriage = v_space(i_last,i) ;
			i_last = i ;
			for ( p=lines[i] ; p!=NULL ; p=p->next_line )
			{
				qsort(p->chars,p->char_count,sizeof(struct chars),compare) ;
				for ( char_count=p->char_count ; char_count>0 ; )
				{
					printf("%c%c",carriage,p->fontindexchar) ;
					carriage = '+' ;
					x_current = 0 ;
					line_chars = 0 ;
					for ( j=0 ; j<p->char_count ; j++ )
						if ( p->chars[j].position >= x_current )
						{
							line_chars += h_space(p->chars[j].position-x_current ) ;
							if ( line_chars >= MAX_LINE_CHARS )
								break ;
							x_current = p->chars[j].position ;
							putchar(p->chars[j].character) ;
							p->chars[j].position = -1 ;
							char_count-- ;
							x_current += p->chars[j].width ;
							line_chars++ ;
						}
					putchar('\n') ;
				}
				f_ree(p->chars) ;
			}
			lines[i] = NULL ;
		}
	v_space(i_last,dev.paperlength/STRIPSIZE*STRIPSIZE) ;
	line_avail = 0 ;
}
int
compare(c1,c2)
struct	chars	*c1 ;
struct	chars	*c2 ;
{
	return( c1->position - c2->position ) ;
}
