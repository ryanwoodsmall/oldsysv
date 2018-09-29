char charxver[] = "@(#)char.c	1.1";		/* SCCS */

# include <stdio.h>
# include "structs.h"

funny_char(p)
char	*p ;
{
	int	index ;
	int	font ;

	for ( index=0 ; index<dev.nchtab ; index++ )
		if ( strcmp(p,&charnames[indices[index]]) == 0 )
			break ;
	if ( index == dev.nchtab )
		error("special character name %s not defined\n",p) ;
	index += 128-32 ;
	for ( font=0 ; font<dev.nfonts ; font++ )
		if
		(
			(Fonts[font].font.specfont == 1  ||  font == cur_font)
			&&
			Fonts[font].fitab[index] != 0
		)
			break ;
	if ( font == dev.nfonts )
		error("can't find character %s on the current font or any special font\n",p) ;
	norm_char(index,font) ;
}
norm_char(index,font)
int	index ;
int	font ;
{
	int		c_ndex ;

	int		c_code ;
	struct	line	*p ;
	char		*re_alloc() ;

/*
	printf("%d\t%d\t%d\t(%d,%d)\n",index,size,font,xpos,ypos) ;
*/
				/* index==0 -> char is space */
	if ( index == 0 )
		return ;
	c_ndex = Fonts[font].fitab[index] ;
						/* TEMPORARY KLUDGE!!!!! */
						/* DO THIS RIGHT!! */
	if ( c_ndex == 0 )
	{
		for ( font=0 ; font<dev.nfonts ; font++ )
			if
			(
				Fonts[font].font.specfont == 1
				&&
				Fonts[font].fitab[index] != 0
			)
				break ;
		if ( font == dev.nfonts )
			error("can't find ascii character index %d on the current font or any special font\n",index) ;
		c_ndex = Fonts[font].fitab[index] ;
	}
	c_code = Fonts[font].info[c_ndex].code ;
					/* if c_code==0, no char to output
					   as in \| or \^
					*/
	if ( c_code == 0 )
		return ;
	store_char
	(
		c_code,
		font,
		size,
		Fonts[font].info[c_ndex].width,
		xpos,
		ypos
	) ;
}

store_char(charcode,font,size,width,xpos,ypos)
int	charcode ;
int	font ;
int	size ;
int	width ;
int	xpos ;
int	ypos ;
{
	struct	line	*p ;
	char		fontindexchar ;
	int		i ;
	int		best ;
	char		*re_alloc() ;

	if ( ypos >= dev.paperlength )
	{
		warning("character below bottom of page\n") ;
		return ;
	}
	if ( ypos < 0 )
	{
		warning("character above top of page\n") ;
		return ;
	}
	if ( xpos >= dev.paperwidth )
	{
		warning("character off page to right\n") ;
		return ;
	}
	if ( xpos < 0 )
	{
		warning("character off page to left\n") ;
		return ;
	}
	if ( font == PLOTFLAG )
		fontindexchar = PLOTINDEX ;
	else
		fontindexchar = Fonts[font].font.intname[size] ;
	if ( fontindexchar == '0' )
	{
		for ( best=0 ; best<dev.nsizes ; best++ )
			if ( Fonts[font].font.intname[best] != '0' )
				break ;
		if ( best == dev.nsizes )
			error("font %s not available in any size\n",Fonts[font].font.namefont) ;	
		for ( i=best+1 ; i<dev.nsizes ; i++ )
			if
			(
				Fonts[font].font.intname[i] != '0'  &&
				abs(sizes[i]-sizes[size]) < abs(sizes[best]-sizes[size])
			)
				best = i ;
		warning("font %s not available in size %d: using %d\n",
		Fonts[font].font.namefont,sizes[size],sizes[best]) ;
		size = best ;
		fontindexchar = Fonts[font].font.intname[size] ;
	}

	for ( p=lines[ypos] ; p!=NULL ; p=p->next_line )
		if ( p->fontindexchar == fontindexchar )
			break ;
	if ( p == NULL )
	{
		if ( line_avail >= POOLSIZE )
			error("number of lines exceeds %d on page %d\n",POOLSIZE,page) ;
		p = &line_pool[line_avail++] ;
		p->char_count = 0 ;
		p->max_count = WIDTH_INCR ;
		p->fontindexchar = fontindexchar ;
		p->chars = (struct chars *)m_alloc(WIDTH_INCR*sizeof(struct chars)) ;
		p->next_line = lines[ypos] ;
		lines[ypos] = p ;
	}
	if ( p->char_count == p->max_count )
	{
		p->max_count += WIDTH_INCR ;
		p->chars = (struct chars *)re_alloc(p->chars,p->max_count*sizeof(struct chars)) ;
	}

	if ( width == 0 )
		width = 1 ;
	else if ( fontindexchar != PLOTINDEX )
		width = (width * sizes[size] + dev.unitwidth/2) / dev.unitwidth ;

	p->chars[p->char_count].character = charcode ;
	p->chars[p->char_count].width = width ;
	p->chars[p->char_count++].position = xpos ;
}
