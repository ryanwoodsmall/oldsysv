char readxver[] = "@(#)read_desc.c	1.1";		/* SCCS */

# include <stdio.h>
# include <ctype.h>
# include "structs.h"

read_desc(name)
char	*name ;
{
	int	i ;
	int	j ;
	char	buf[BUFSIZE] ;
	FILE	*fp ;
	char	*m_alloc() ;

	sprintf(buf,"/usr/lib/font/dev%s/DESC.out",name) ;
	fp = fopen(buf,"r") ;
	if ( fp == NULL )
		error("cannot open %s (%d)\n",buf,errno) ;
	r_ead(fp,&dev,sizeof(dev),"struct dev") ;
	sizes = (short *)m_alloc((dev.nsizes+1)*sizeof(short)) ;
	r_ead(fp,sizes,(dev.nsizes+1)*sizeof(short),"size array") ;
	indices = (short *)m_alloc(dev.nchtab*sizeof(short)) ;
	r_ead(fp,indices,dev.nchtab*sizeof(short),"indices") ;
	charnames = m_alloc(dev.lchname*sizeof(char)) ;
	r_ead(fp,charnames,dev.lchname*sizeof(char),"char names") ;
	Fonts = (struct Font *)m_alloc(dev.nfonts*sizeof(struct Font)) ;
	for ( i=0 ; i<dev.nfonts ; i++ )
	{
		r_ead(fp,&Fonts[i].font,sizeof(struct font),"font") ;
		Fonts[i].info = (struct charinfo *)m_alloc((Fonts[i].font.nwfont&0xff)*sizeof(struct charinfo)) ;
		for ( j=0 ; j<(Fonts[i].font.nwfont&0xff) ; j++ )
			r_ead(fp,&Fonts[i].info[j].width,sizeof(Fonts[i].info[j].width),"width") ;
		for ( j=0 ; j<(Fonts[i].font.nwfont&0xff) ; j++ )
			r_ead(fp,&Fonts[i].info[j].kerning,sizeof(Fonts[i].info[j].kerning),"kerning") ;
		for ( j=0 ; j<(Fonts[i].font.nwfont&0xff) ; j++ )
			r_ead(fp,&Fonts[i].info[j].code,sizeof(Fonts[i].info[j].code),"code") ;
		Fonts[i].fitab = (unsign char *)m_alloc((dev.nchtab+128-32)*sizeof(char)) ;
		r_ead(fp,Fonts[i].fitab,(dev.nchtab+128-32)*sizeof(char),"fitab") ;
	}
	fclose(fp) ;

	for ( i=0 ; (name[i] && (name[i] != '.')) ; i++ )
		;

	if (!name[i])  i = 0;	/* skip early part of name before period */

	for ( j=0, i++ ; name[i]!='\0' ; i++, j++ )
		pde_name[j] = toupper(name[i]) ;
	pde_name[i] = '\0' ;
}
