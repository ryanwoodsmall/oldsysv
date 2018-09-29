/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcc2:common/pftn.c	1.42"

# include "mfile1.h"

unsigned int maxoffset;

struct instk 
{
	int in_sz;   /* size of array element */
	int in_x;    /* current index for structure member in initializations */
	int in_n;    /* number of initializations seen */
	int in_s;    /* sizoff */
	int in_d;    /* dimoff */
	TWORD in_t;    /* type */
	int in_id;   /* stab index */
	int in_fl;   /* flag:  number of {'s remaining open at this level */
	OFFSZ in_off;  /* offset of the beginning of this level */
};

static struct instk * pstk;
/* static */ struct instk instk_init[INI_INSTK];

#ifndef STATSOUT
static
#endif
	TD_INIT(td_instack, INI_INSTK, sizeof(struct instk),
		TD_ZERO, instk_init, "initialization stack");

#define instack ((struct instk *)td_instack.td_start)

int ddebug = 0;

struct symtab * mknonuniq();
static unsigned short st_hash[HASHTSZ];	/* symbol hash buckets */
/* hash function based on name pointer */
#define st_hashf(p) /* (char *) p */ (((unsigned long) (p)) % HASHTSZ)

defid( q, class )
register NODE *q; 
{
	register struct symtab *p;
	int idp;
	register TWORD type;
	register TWORD stp;
	register scl;
	register dsym, ddef;
	register slev, temp;
	int wantedclass = class;	/* class originally wanted */

	if( q == NIL ) return;  /* an error was detected */
	if (class == LABEL || class == ULABEL)
	    q->tn.rval = looklab(q->tn.rval);	/* re-lookup as label */
	idp = q->tn.rval;
	if( idp < 0 ) cerror( "tyreduce" );
	p = &stab[idp];
# ifndef NODBG
	if( ddebug )
	{
		printf( "defid( %s (%d), ", p->sname, idp );
		tprint( q->in.type );
		printf( ", %s, (%d,%d) ), level %d\n", scnames(class),
		q->fn.cdim, q->fn.csiz, blevel );
	}
# endif
	fixtype( q, class );
	type = q->in.type;
	class = fixclass( class, type );
	stp = p->stype;
	slev = p->slevel;
# ifndef NODBG
	if( ddebug )
	{
		printf( "	modified to " );
		tprint( type );
		printf( ", %s\n", scnames(class) );
		printf( "	previous def'n: " );
		tprint( stp );
		printf( ", %s, (%d,%d) ), level %d\n", scnames(p->sclass),
		p->dimoff, p->sizoff, slev );
	}
# endif
	if( stp == FTN && p->sclass == SNULL )goto enter;
	/* name encountered as function, not yet defined */
	/* BUG here!  can't incompatibly declare func. in inner block */
	if( stp == UNDEF|| stp == FARG )
	{
		if( blevel==1 && stp!=FARG ) switch( class )
		{
		default:
			if(!(class&FIELD)) uerror(
			"declared argument %s is missing", p->sname );
		case MOS:
		case STNAME:
		case MOU:
		case UNAME:
		case MOE:
		case ENAME:
		case TYPEDEF:
			;
		}
		goto enter;
	}
	if( type != stp ) goto mismatch;
	/* test (and possibly adjust) dimensions */
	dsym = p->dimoff;
	ddef = q->fn.cdim;
	for( temp=type; temp&TMASK; temp = DECREF(temp) )
	{
		if( ISARY(temp) )
		{
			if( dimtab[dsym] == 0 ) dimtab[dsym] = dimtab[ddef];
			else if( dimtab[ddef]!=0 && dimtab[dsym] != dimtab[ddef] )
			{
				goto mismatch;
			}
			++dsym;
			++ddef;
		}
	}
	/* check that redeclarations are to the same structure */
	if( (temp==STRTY||temp==UNIONTY||temp==ENUMTY) && p->sizoff != q->fn.csiz
	    && class!=STNAME && class!=UNAME && class!=ENAME )
	{
		goto mismatch;
	}
	scl = ( p->sclass );
# ifndef NODBG
	if( ddebug )
	{
		printf( "	previous class: %s\n", scnames(scl) );
	}
# endif
	if( class&FIELD )
	{
		/* redefinition */
		if( !falloc( p, class&FLDSIZ, 1, NIL ) ) 
		{
			/* successful allocation */
			psave( idp );
			return;
		}
		/* blew it: resume at end of switch... */
	}
	else switch( class )
	{
	case EXTERN:
		switch( scl )
		{
		case STATIC:
		case USTATIC:
			if( slev==0 ) return;
			break;
		case EXTDEF:
		case EXTERN:
		case FORTRAN:
		case UFORTRAN:
#ifdef IN_LINE
		case INLINE:
#endif
			return;
		}
		break;
	case STATIC:
		if( scl==USTATIC || (scl==EXTERN && blevel==0))
		{
			/* warn if not a function */
			if (!ISFTN(type))
			    werror("%s previously declared extern, becomes static",
					p->sname);
			p->sclass = STATIC;
			if( ISFTN(type) ) curftn = idp;
			return;
		}
		break;
	case USTATIC:
		if( scl==STATIC || scl==USTATIC ) return;
		break;
	case LABEL:
		if( scl == ULABEL )
		{
			p->sclass = LABEL;
			deflab( p->offset );
			return;
		}
		break;
	case TYPEDEF:
		if( scl == class ) return;
		break;
	case UFORTRAN:
		if( scl == UFORTRAN || scl == FORTRAN ) return;
		break;
	case FORTRAN:
		if( scl == UFORTRAN )
		{
			p->sclass = FORTRAN;
			if( ISFTN(type) ) curftn = idp;
			return;
		}
		break;

#ifdef IN_LINE
	case INLINE:
		curftn = idp;
		break;
#endif

	case MOU:
	case MOS:
		if( scl == class ) 
		{
			if( oalloc( p, &strucoff ) ) break;
			if( class == MOU ) strucoff = 0;
			psave( idp );
			return;
		}
		break;
	case MOE:
		if( scl == class )
		{
			if( p->offset!= strucoff ) break;
			psave( idp );
		}
		break;
	case EXTDEF:
		if( scl == EXTERN ) 
		{
			p->sclass = EXTDEF;
			if( ISFTN(type) ) curftn = idp;
			return;
		}
		else if ( scl == USTATIC && ISFTN(type) ) {
			p->sclass = STATIC;
			curftn = idp;
			return;
		}
		break;
	case STNAME:
	case UNAME:
	case ENAME:
		if( scl != class ) break;
		if( dimtab[p->sizoff] == 0 )
			return;  /* previous entry just a mention */
		break;
	case ULABEL:
		if( scl == LABEL || scl == ULABEL ) return;
	case PARAM:
	case AUTO:
	case REGISTER:
		;  /* mismatch.. */
	}
mismatch:
	/* allow nonunique structure/union member names */
	if( class==MOU || class==MOS || class & FIELD )
	{
		/* make a new entry */
		register * memp;
		p->sflags |= SNONUNIQ;  /* old entry is nonunique */
		/* determine if name has occurred in this structure/union */
		for( memp = &paramstk[paramno-1];
			/* while */ *memp>=0 && stab[*memp].sclass != STNAME
				&& stab[*memp].sclass != UNAME;
			/* iterate */ --memp)
		{
			if( stab[*memp].sflags & SNONUNIQ )
			{
				if ( p->sname != stab[*memp].sname )
					continue;
				uerror("redeclaration of: %s",
						p->sname);
				break;
			}
		}
		p = mknonuniq( &idp ); /* update p and idp to new entry */
		goto enter;
	}
	/* let slev reflect what level will be after this block's exit
	** to gauge redeclaration correctly
	*/
	switch (p->sclass) {
	case LABEL:
	case ULABEL:
	    slev = 2;
	    break;

	case EXTERN:
	case UFORTRAN:
	case FORTRAN:
	    slev = 0;
	    break;
	}

	/* check whether a hiding definition is appropriate */
	switch (class) {
#ifdef IN_LINE
	case INLINE:	break;		/* asm func. can't use same name */
#endif

	case LABEL:
	    if (slev >= 2) break;	/* error if old def. is within func. */

	default:
	    if (blevel <= slev) break;	/* error if at same or lower level */

	    q->tn.rval = idp = hide( idp );
	    if (class == EXTERN)
		checkext(idp, type);	/* check for duplicate extern */
	    p = &stab[idp];
	    goto enter;
	}

	uerror( "redeclaration of %s", p->sname );
	if( class==EXTDEF && ISFTN(type) ) curftn = idp;
	return;
enter:  /* make a new entry */
# ifndef NODBG
	if( ddebug ) printf( "	new entry made\n" );
# endif
	if( type == UNDEF ) uerror("void type for %s",p->sname);
	p->stype = type;
	p->sclass = class;
	p->slevel = blevel;
	p->offset = NOOFFSET;
	p->suse = lineno;
	if (wantedclass == REGISTER && class != REGISTER)
	    p->sflags |= SISREG;	/* note decl. was for REG */

	if( class == STNAME || class == UNAME || class == ENAME ) 
	{
		p->sizoff = curdim;
		dstash( 0 );  /* size */
		dstash( -1 ); /* index to members of str or union */
		dstash( ALSTRUCT );  /* alignment */
		dstash( idp );  /* name index */
	}
	else 
	{
		switch( BTYPE(type) )
		{
		case STRTY:
		case UNIONTY:
		case ENUMTY:
			p->sizoff = q->fn.csiz;
			break;
		default:
			p->sizoff = BTYPE(type);
		}
	}
	/* copy dimensions */
	p->dimoff = q->fn.cdim;
	/* allocate offsets */
	if( class&FIELD )
	{
		falloc( p, class&FLDSIZ, 0, NIL );  /* new entry */
		psave( idp );
	}
	else switch( class )
	{
	case AUTO:
		oalloc( p, &autooff );
		break;
	case STATIC:
	case EXTDEF:
		p->offset = getlab();
#ifdef IN_LINE
	case INLINE:
#endif
		if( ISFTN(type) ) curftn = idp;
		break;
	case ULABEL:
	case LABEL:
		p->offset = getlab();
		if( class == LABEL )
		{
			locctr( PROG );
			deflab( p->offset );
		}
		break;
	case EXTERN:
	case UFORTRAN:
	case FORTRAN:
		p->offset = getlab();
		break;
	case MOU:
	case MOS:
		oalloc( p, &strucoff );
		if( class == MOU ) strucoff = 0;
		psave( idp );
		break;
	case MOE:
		p->offset = strucoff++;
		psave( idp );
		break;
	case REGISTER:
		/* nextrvar is left set by cisreg when it returns 1 */
		p->offset = nextrvar;
		if( blevel == 1 ) p->sflags |= SSET;
		break;
	}
	/* user-supplied routine to fix up new definitions */

# ifdef FIXDEF
#ifdef TMPSRET		/* don't try to put out sdb stuff for .stfake! */
	if (strcmp(p->sname, ".stfake"))
#endif
		FIXDEF(p);
# endif
 	if (blevel >= MAXNEST)		/* expand table as needed */
	    td_enlarge(&td_scopestack, blevel+1);
#ifdef STATSOUT
	if (td_scopestack.td_max < blevel) td_scopestack.td_max = blevel;
#endif

 	p->st_scopelink = scopestack[slev = p->slevel];
 	scopestack[slev] = idp;

# ifndef NODBG
	if( ddebug )
		printf( "	dimoff, sizoff, offset: %d, %d, %d\n",
			p->dimoff, p->sizoff, p->offset );
# endif
}

asave( i )
{
	if( argno >= ARGSZ ) {
	    /* expand the three args-related tables */
	    td_enlarge(&td_argstk, 0);
	    td_enlarge(&td_argsoff, 0);
	    td_enlarge(&td_argty, 0);
	}
	argstk[ argno++ ] = i;
#ifdef STATSOUT
	if (td_argstk.td_max < argno) td_argstk.td_max = argno;
#endif
}

psave( i )
{
	if( paramno >= PARAMSZ )
	    td_enlarge(&td_paramstk,0);
	paramstk[ paramno++ ] = i;
#ifdef STATSOUT
	if (td_paramstk.td_max < paramno) td_paramstk.td_max = paramno;
#endif
}

/* maximium size of outgoing arguments */
int maxarg;

ftnend()
{
	 /* end of function */
	if( retlab != NOLAB )
	{
		/* inside a real function */

		efcode();
# ifdef SDB
		{
			register i;
			register struct symtab *p;
			register TWORD t;
			for( i=0; i<argno; ++i )
			{
				/* use the saved types */
				p = &stab[argstk[i]];
				t = p->stype;
				p->stype = argty[i];
				if( p->sclass == REGISTER )
				{
					p->sclass = PARAM;
					p->offset = argsoff[i];
				}
				/*ppstab(p);*/
				p->stype = t;
			}
		}
# endif

		p2bend();
	}
	checkst(0);
	retstat = 0;
	tcheck();
	curclass = SNULL;
	brklab = contlab = retlab = NOLAB;
	flostat = 0;
	strftn = 0;
	argno = 0;
	if( nerrors == 0 )
	{
		if( psavbc != 0 ) cerror("bcsave error");
		if( paramno != 0 ) cerror("parameter reset error");
		if( sw_beg != 0 ) cerror( "switch error");
	}
	psavbc = 0;
	paramno = 0;
	autooff = AUTOINIT;
	maxarg = 0;
	reached = 1;
	sw_beg = 0;
	swidx = 0;
	locctr(DATA);
}

dclargs()
{
	register i, j;
	register struct symtab *p;
	register NODE *q;
	register TWORD temp;
	extern TWORD simpstr();

	argoff = ARGINIT;

# ifndef NODBG
	if( ddebug > 2) printf("dclargs()\n");
# endif
	for( i=0; i<argno; ++i )
	{
		if( (j = argstk[i]) < 0 ) continue;
		p = &stab[j];
# ifndef NODBG
		if( ddebug > 2 )
		{
			printf("\t%s (%d) ",p->sname, j);
			tprint(p->stype);
			printf("\n");
		}
# endif
		if( p->stype == FARG ) 
		{
			q = block(FREE,NIL,NIL,INT,0,INT);
			q->tn.rval = j;
			defid( q, PARAM );
		}
		if( p->sclass == REGISTER )
		{
			/* must set aside space, fill argsoff */
			int tmp = p->offset;
			p->offset = NOOFFSET;
			p->sclass = PARAM;
			oalloc( p, &argoff );
			argsoff[i] = p->offset;
			argty[i] = p->stype;
			p->sclass = REGISTER;
			p->offset = tmp;
		}
		else 
		{
			oalloc( p, &argoff );  /* always set aside space */
			argsoff[i] = p->offset;
			argty[i] = p->stype;
			/* struct/union PARAMs have minimum ALINT alignment */
			if (p->stype == STRTY || p->stype == UNIONTY) {
			    SETOFF( argoff, ALINT );
			}
		}
	}
	autooff = AUTOINIT;
# ifdef CENDARG
	CENDARG();
# endif
#ifdef IN_LINE		
	if (curclass != INLINE)  /* No code gen'ed for INLINE asm 'function' */
#endif
	{		/* start INLINE conditional block */
	locctr(PROG);
	defalign(ALINT);
	++ftnno;
	p = &stab[curftn];

	if( p->slevel>1 && p->sclass == STATIC )
	    deflab( p->offset );
	else {
	    defnamdbg( p );		/* do debug info for function name */
	    defnam( p );
	}
	temp = p->stype;
	temp = DECREF( temp );

	/* optimization: functions returning short structs */
	/* strftn != 0 if function returns structure */
	strftn = (temp==STRTY) || (temp==UNIONTY);
	if( strftn && simpstr( p->dimoff, p->sizoff ) != STRTY )
		strftn = 0;

	bfcode( argstk, argno );
# ifndef NOREGARGS
	regargs();
# endif
	/* code for the case where returning a structure uses a static */
	if( strftn ) 
	{
		NODE * regnode;
# ifdef STATSRET
		  /* scalars ok */
		/* define something the proper size */
		i = tsize( temp, p->dimoff, p->sizoff);
# ifdef SRETNAME
		SRETNAME(i);
# else
		locctr( DATA );
		defalign( talign( temp, p->sizoff ) );
		deflab ( strftn = getlab() );
		zecode( (i+SZINT-1)/SZINT );  /* set aside integer zeros */
		locctr( PROG );
# endif
# endif
# ifdef TMPSRET
	/* code to stash auxiliary reg (return addr for struct) into auto */
		idname = lookup(hash(".stfake"), 0);
		q = bdty(NAME, NIL, idname);
		q->tn.type = PTR|STRTY;
		++blevel; defid(q, AUTO); --blevel; /* sleazy, aren't we? */
		regnode = block(REG, NIL, NIL, PTR|STRTY, q->fn.cdim, q->fn.csiz);
		regnode->tn.rval = AUXREG;	/* set register number */
		ecomp(buildtree(ASSIGN, buildtree(NAME, NIL, NIL),regnode));
		q->tn.op = FREE;
# endif
	}
	}  	/* end INLINE conditional block */
}

# ifndef NOREGARGS
regargs()
{
	register i;
	register NODE *p, *q;
	register struct symtab *s;
	/* output the copy assignements for register arguments */
	for( i=0; i<argno; ++i )
	{
		s = &stab[argstk[i]];
		if( s->sclass == REGISTER )
		{
			int temp;
			idname = argstk[i];
			p = buildtree( NAME, NIL, NIL );
			temp = s->offset;
			s->offset = argsoff[i];
			s->sclass = PARAM;
			q = buildtree( NAME, NIL, NIL );
			p = buildtree( ASSIGN, p, q );
			ecomp( p );
			s->offset = temp;
			s->sclass = REGISTER;
		}
	}
}
# endif

NODE *
rstruct( idn, soru )
register idn,soru;
{
	 /* reference to a struct or union, with no definition */
	register struct symtab *p;
	register NODE *q;
	p = &stab[idn];
	switch( p->stype )
	{
	case UNDEF:
def:
		q = block( FREE, NIL, NIL, 0, 0, 0 );
		q->tn.rval = idn;
		q->in.type = (soru&INSTRUCT) ? STRTY : ( (soru&INUNION) ? UNIONTY : ENUMTY );
		defid( q, (soru&INSTRUCT) ? STNAME : ( (soru&INUNION) ? UNAME : ENAME ) );
		break;
	case STRTY:
		if( soru & INSTRUCT ) break;
		goto def;
	case UNIONTY:
		if( soru & INUNION ) break;
		goto def;
	case ENUMTY:
		if( !(soru&(INUNION|INSTRUCT)) ) break;
		goto def;
	}
	stwart = instruct;
	return( mkty( p->stype, 0, p->sizoff ) );
}

moedef( idn )
register idn;
{
	register NODE *q;

	q = block( FREE, NIL, NIL, MOETY, 0, 0 );
	q->tn.rval = idn;
	if( idn>=0 ) defid( q, MOE );
}

bstruct( idn, soru )
register idn,soru;
{
	 /* begining of structure or union declaration */
	register NODE *q;

	psave( instruct );
	psave( curclass );
	psave( strucoff );
	strucoff = 0;
	instruct = soru;
	q = block( FREE, NIL, NIL, 0, 0, 0 );
	q->tn.rval = idn;
	if( instruct==INSTRUCT )
	{
		curclass = MOS;
		q->in.type = STRTY;
		if( idn >= 0 ) defid( q, STNAME );
	}
	else if( instruct == INUNION ) 
	{
		curclass = MOU;
		q->in.type = UNIONTY;
		if( idn >= 0 ) defid( q, UNAME );
	}
	else 
	{
		 /* enum */
		curclass = MOE;
		q->in.type = ENUMTY;
		if( idn >= 0 ) defid( q, ENAME );
	}
	psave( idn = q->tn.rval );
	/* A potential problem is that the user will attempt to define
	** another struct/union/enum of the same name as the current
	** item.  Put a dummy value in sizoff so defid() will complain.
	** The dummy value will get clobbered by dclstruct() when we're
	** all done.
	*/
	/* the "real" definition is where the members are seen */
	if( idn >= 0 ) {
	    stab[idn].suse = lineno;
	    dimtab[ stab[idn].sizoff ] = -1;
	}
	return( paramno-4 );
}

# ifndef ENUMSIZE
# define ENUMSIZE(h,l) INT
# endif

NODE *
dclstruct( oparam )
register oparam;
{
	register struct symtab *p;
	register i, al, sa, j, sz, szindex;
	register TWORD temp;
	register high, low;

	/* paramstack contains:
	** paramstack[ oparam ] = previous instruct
	** paramstack[ oparam+1 ] = previous class
	** paramstk[ oparam+2 ] = previous strucoff
	** paramstk[ oparam+3 ] = structure name
	** 
	** paramstk[ oparam+4, ... ]  = member stab indices
	** 
	*/


	if( (i=paramstk[oparam+3]) < 0 )
	{
		szindex = curdim;
		dstash( 0 );  /* size */
		dstash( -1 );  /* index to member names */
		dstash( ALSTRUCT );  /* alignment */
		dstash( -lineno );  /* name of structure */
	}
	else 
	{
		szindex = stab[i].sizoff;
	}


# ifndef NODBG
	if( ddebug )
	{
		printf( "dclstruct( %s, szindex = %d\n)",
			(i>=0)? stab[i].sname : "??", szindex );
	}
# endif
	temp = (instruct&INSTRUCT)?STRTY:((instruct&INUNION)?UNIONTY:ENUMTY);
	stwart = instruct = paramstk[ oparam ];
	curclass = paramstk[ oparam+1 ];
	dimtab[ szindex+1 ] = curdim;
	al = ALSTRUCT;

	high = low = 0;

	for( i = oparam+4;  i< paramno; ++i )
	{
		dstash( j=paramstk[i] );
		if( j<0 || j>= SYMTSZ ) cerror( "gummy structure member" );
		p = &stab[j];
		if( temp == ENUMTY )
		{
			if( p->offset < low ) low = p->offset;
			if( p->offset > high ) high = p->offset;
			p->sizoff = szindex;
			continue;
		}
		sa = talign( p->stype, p->sizoff );
		if( p->sclass & FIELD )
		{
			sz = p->sclass&FLDSIZ;
		}
		else 
		{
			sz = tsize( p->stype, p->dimoff, p->sizoff );
		}
		if( sz == 0 )
		{
			werror( "structure member has size 0: %s", p->sname );
		}
		if( sz > strucoff ) strucoff = sz;  /* for use with unions */
		SETOFF( al, sa );
		/* al, the alignment, to the LCM of the members' alignments */
	}
	dstash( -1 );  /* endmarker */
	SETOFF( strucoff, al );

	if( temp == ENUMTY )
	{
		register TWORD ty;

		ty = ENUMSIZE(high,low);
		strucoff = tsize( ty, 0, (int)ty );
		dimtab[ szindex+2 ] = al = talign( ty, (int)ty );
	}

	if( strucoff == 0 ) uerror( "zero sized structure" );
	dimtab[ szindex ] = strucoff;
	dimtab[ szindex+2 ] = al;
	dimtab[ szindex+3 ] = paramstk[ oparam+3 ];  /* name index */

#ifdef	FIXSTRUCT
	FIXSTRUCT( szindex, oparam );	/* local hook for debugging */
#endif

# ifndef NODBG
	if( ddebug>1 )
	{
		printf( "\tdimtab[%d,%d,%d,%d] = %d,%d,%d,%d\n",
		szindex,szindex+1,szindex+2,szindex+3,
		dimtab[szindex],dimtab[szindex+1],dimtab[szindex+2],
		dimtab[szindex+3] );
		for( i = dimtab[szindex+1]; dimtab[i] >= 0; ++i )
		{
			printf( "\tmember %s(%d)\n", stab[dimtab[i]].sname, dimtab[i] );
		}
	}
# endif

	strucoff = paramstk[ oparam+2 ];
	paramno = oparam;

	return( mkty( temp, 0, szindex ) );
}

/* VARARGS */
yyerror( s ) char *s; 

{
	 /* error printing routine in parser */
	uerror( s );

}

yyaccpt()
{
	ftnend();
}

#ifdef IN_LINE
/*
/* inlargs[] collects the asm function argument names from the formal
/* parameter list.  The names will be written into the  asm expansion
/* storage file by squirrelmacro().  inlargs[] will also be used
/* to read the formal parameter names back in when an asm call is expanded.
*/
int ninlargs = 0;
#endif


ftnarg( idn ) 
register idn;
{
	register struct symtab *p;
	p = &stab[idn];
	switch( p->stype )
	{

	case UNDEF:
		/* this parameter, entered at scan */
		break;
	case FARG:
		uerror("redeclaration of formal parameter, %s",p->sname);
		/* fall thru */
	case FTN:
		/* the name of this function matches parm */
		/* fall thru */
	default:
		idn = hide(idn);
		p = &stab[idn];
		/* Eventually defid() gets called with this ID at blevel
		** 1, and it then gets linked onto scopestack at that
		** level.  Linking it now leads to a mess.
		*/
		break;
	case TNULL:
		/* unused entry, fill it */
		;
	}
	p->stype = FARG;
	p->sclass = PARAM;
	asave( idn );

#ifdef IN_LINE
/* collect the asm function argument names, delimited by '#',
/* to be written into the expansion storage file by squirrelmacro()
*/
	if (curclass == INLINE)
	{
	    /* Want to create:
	    **	<oldnames> # <newname> NUL
	    */
	    int needsize = sz_inlargs + 1 + strlen(p->sname) + 1;
	    if ( needsize > SZINLARGS )
		td_enlarge(&td_inlargs, needsize - SZINLARGS);
	    
	    sprintf( &inlargs[sz_inlargs], "%s#", p->sname);
	    sz_inlargs = needsize - 1;	/* don't include NUL here */
#ifdef	STATSOUT
	    if (needsize > td_inlargs.td_max)
		td_inlargs.td_max = needsize;
#endif
	    ninlargs++;
	}
#endif
	    
}	

talign( ty, s)
register unsigned ty; 
register s; 
{
	/* compute the alignment of an object with type ty, sizeoff index s */

	register i;
# ifdef FUNNYFLDS
	if( s<0 && ty!=INT && ty!=CHAR && ty!=SHORT && ty!=UNSIGNED && ty!=UCHAR && ty!=USHORT 
#ifdef LONGFIELDS
	    && ty!=LONG && ty!=ULONG
#endif
	    )
	{
		return( fldal( ty ) );
	}
# endif

	for( i=0; i<=(SZINT-BTSHIFT-1); i+=TSHIFT )
	{
		switch( (ty>>i)&TMASK )
		{

		case FTN:
			cerror( "compiler takes alignment of function");
		case PTR:
			return( ALPOINT );
		case ARY:
			continue;
		case 0:
			break;
		}
	}

	switch( BTYPE(ty) )
	{

	case UNIONTY:
	case ENUMTY:
	case STRTY:
		return( dimtab[ s+2 ] );
	case CHAR:
	case UCHAR:
		return( ALCHAR );
	case FLOAT:
		return( ALFLOAT );
	case DOUBLE:
		return( ALDOUBLE );
	case LONG:
	case ULONG:
		return( ALLONG );
	case SHORT:
	case USHORT:
		return( ALSHORT );
	default:
		return( ALINT );
	}
}

OFFSZ
tsize( ty, d, s )
register TWORD ty; 
{
	/* compute the size associated with type ty,
	** dimoff d, and sizoff s 
	*/
	/* BETTER NOT BE CALLED WHEN t, d, and s REFER TO A BIT FIELD... */

	register i;
	register OFFSZ mult;
	OFFSZ size;

	mult = 1;

	for( i=0; i<=(SZINT-BTSHIFT-1); i+=TSHIFT )
	{
		switch( (ty>>i)&TMASK )
		{

		case FTN:
			cerror( "compiler takes size of function");
		case PTR:
			return( SZPOINT * mult );
		case ARY:
			mult *= dimtab[ d++ ];
			continue;
		case 0:
			break;

		}
	}

	if( dimtab[s]==0 ) 
	{
		uerror( "unknown size");
		return( SZINT );
	}
	size = dimtab[ s ] * mult;
	/* check for size overflow (array too large) */
	if (size/dimtab[s] != mult) {
	    uerror("array too large");
	    return( SZINT );
	}
	return( size );
}

inforce( n )
register OFFSZ n; 
{
	  /* force inoff to have the value n */
	/* inoff is updated to have the value n */
	register OFFSZ wb;
	register rest;
	/* rest is used to do a lot of conversion to ints... */

	if( inoff == n ) return;
	if( inoff > n ) 
	{
		/* this seems only to arise if there are excess
		** initializers for a single struct:
		**	struct {int a;} x = {1,2};
		**
		** former message: "initialization alignment error"
		*/
		uerror("too many initializers; missing } ?");
		inoff = n;		/* make alignment look okay */
		return;
	}

	wb = inoff;
	SETOFF( wb, SZINT );

	/* wb now has the next higher word boundary */
	if( wb >= n )
	{
		 /* in the same word */
		rest = n - inoff;
		vfdzero( rest );
		return;
	}

	/* otherwise, extend inoff to be word aligned */

	rest = wb - inoff;
	vfdzero( rest );

	/* now, skip full words until near to n */

	rest = (n-inoff)/SZINT;
	zecode( rest );

	/* now, the remainder of the last word */

	rest = n-inoff;
	vfdzero( rest );
	if( inoff != n ) cerror( "inoff error");

}

vfdalign( n )
{
	 /* make inoff have the offset the next alignment of n */
	register OFFSZ m;

	m = inoff;
	SETOFF( m, n );
	inforce( m );
}


#ifndef NODBG
extern int idebug;
#endif

int iclass;  /* storage class of thing being initialized */

int ilocctr = 0;  /* location counter for current initialization */

static int defer_def = -1;	/* >= 0 to defer producing debug information for
				** stab[ defer_def ] until after all initializers
				** have been seen.
				*/
beginit(curid)
{
	/* beginning of initilization; set location ctr and set type */
	register struct symtab *p;

# ifndef NODBG
	if( idebug >= 3 ) printf( "beginit(), curid = %d\n", curid );
# endif

	defer_def = -1;			/* no deferred definition */
	p = &stab[curid];

	if ( ISFTN( p->stype ) ) {
	    uerror("function name %s in bad context", p->sname);
	    return;
	}

	iclass = p->sclass;
	if( curclass == EXTERN || curclass == FORTRAN ) iclass = EXTERN;
#ifdef IN_LINE
	if (curclass == INLINE) iclass = EXTERN;
#endif
	switch( iclass )
	{

	case UNAME:
	case EXTERN:
		return;
	case AUTO:
	case REGISTER:
		break;
	case EXTDEF:
	case STATIC:
		ilocctr = ISARY(p->stype)?ADATA:DATA;
		locctr( ilocctr );
		defalign( talign( p->stype, p->sizoff ) );
		if( p->slevel>1 && p->sclass == STATIC )
		    deflab( p->offset );
		else {
		    /* array with unspecified dimension? */
		    if (ISARY( p->stype ) && dimtab[ p->dimoff ] == 0)
			defer_def = curid;	/* remember this ID for later */
		    else
			defnamdbg( p );		/* otherwise generate debug info */
		    defnam( p );		/* always define name now */
		}
	}

	inoff = 0;

	pstk = 0;

	instk( curid, p->stype, p->dimoff, p->sizoff, inoff );

}

instk( id, t, d, s, off )
register OFFSZ off; 
register TWORD t; 

{
	/* make a new entry on the parameter stack to initialize id */

	if (t == STRTY && dimtab[s] == 0) {
	    uerror( "undefined structure" );
	    return;
	}
	    

	/* save information on the stack */

	if( !pstk ) pstk = instack;
	else if (++pstk >= &instack[td_instack.td_allo]) {
	    int oldindex = pstk-instack;

	    td_enlarge(&td_instack, 0);	/* enlarge table */
	    pstk = &instack[oldindex];	/* reset pointer */
	}
#ifdef STATSOUT
	if (pstk-instack > td_instack.td_max)	/* track max. value */
	    td_instack.td_max = pstk-instack;
#endif

# ifndef NODBG
	if( idebug ) {
	    printf( "instk(), level %d, offset %d, symbol %s, type ",
		pstk-instack, off, stab[id].sname);
	    tprint(t);
	    printf(", dim %d, siz %d\n", d, s);
	}
# endif

	pstk->in_fl = 0;	/* { flag */
	pstk->in_id =  id ;
	pstk->in_t =  t ;
	pstk->in_d =  d ;
	pstk->in_s =  s ;
	pstk->in_n = 0;  /* number seen */
	pstk->in_x =  t == STRTY ? dimtab[s+1] : 0 ;
	pstk->in_off = off;  /* offset at beginning of this element */
	/* if t is an array, DECREF(t) can't be a field */
	/* INS_sz has size of array elements, and -size for fields */
	if( ISARY(t) )
	{
		pstk->in_sz = tsize( DECREF(t), d+1, s );
		if (pstk->in_sz == 0)	/* sometimes happens on error */
		    pstk->in_sz = 1;	/* prevent core on divide by zero */
	}
	else if( stab[id].sclass & FIELD )
	{
		pstk->in_sz = - ( stab[id].sclass & FLDSIZ );
	}
	else 
	{
		pstk->in_sz = 0;
	}

	if( (iclass==AUTO || iclass == REGISTER ) &&
	    (ISARY(t) || t==STRTY) )
		uerror( "no automatic aggregate initialization" );

	return;
}

NODE *
getstr()
{
	 /* decide if the string is external or an initializer,
	    and get the contents accordingly */
	register l, temp;
	register NODE *p;
	TWORD t;

	if (pstk) nextelem(0);		/* move to next scalar element if
					** initializing
					*/

	if(    (iclass==EXTDEF||iclass==STATIC)
	    && ((t = pstk->in_t) == CHAR || t == UCHAR)
	    && ISARY( (pstk[-1].in_t) )
	    )
	{
		/* treat "abc" as { 'a', 'b', 'c', 0 } */
		strflg = 1;
		++pstk[-1].in_fl;	/* simulate { */
		inforce( pstk->in_off );

		/* if the array is inflexible (not top level), pass in the size
		** Be prepared to throw away unwanted initializers 
		*/

		/* get the contents */
		lxstr( pstk != instack ? dimtab[pstk[-1].in_d]:0);
		irbrace();  /* simulate } */
		return( NIL );
	}
	else 
	{
		 /* make a label, and get the contents and stash them away */
		if( iclass != SNULL )
		{
			 /* initializing */
			/* fill out previous word, to permit pointer */
			vfdalign( ALPOINT );
		}
		/* set up location counter */
		temp = locctr( blevel==0?ISTRNG:STRNG );
		deflab( l = getlab() );
		strflg = 0;
		lxstr(0); /* get the contents */
		locctr( blevel==0?ilocctr:temp );
		p = buildtree( STRING, NIL, NIL );
		p->tn.rval = -l;
		return(p);
	}
}

putbyte( v )
{
	 /* simulate byte v appearing in a list of integer values */
	register NODE *p;
	p = bcon(v);
	nextelem(0);
	incode( p, SZCHAR );
	tfree( p );
	endelem();
}

endinit()
{
	register TWORD t;
	register d, s, n, d1;

# ifndef NODBG
	if( idebug ) printf( "endinit(), inoff = %d\n", inoff );
# endif

	switch( iclass )
	{

	case EXTERN:
	case AUTO:
	case REGISTER:
		iclass = SNULL;
		return;
	}

	pstk = instack;

	t = pstk->in_t;
	d = pstk->in_d;
	s = pstk->in_s;
	n = pstk->in_n;

	if( ISARY(t) )
	{
		d1 = dimtab[d];

		/* fill out part of the last element, if needed */
		vfdalign( pstk->in_sz );

		n = inoff/pstk->in_sz;  /* real number of initializers */
		if( d1 >= n )
		{
			/* once again, t is an array, so no fields */
			inforce( tsize( t, d, s ) );
			n = d1;
		}
		if( d1!=0 && d1!=n ) uerror( "too many array initializers");
		if( n==0 ) werror( "empty array declaration");
		dimtab[d] = n;
	}

	else if( t == STRTY || t == UNIONTY )
	{
		/* clearly not fields either */
		inforce( tsize( t, d, s ) );
	}
	else if( n > 1 ) uerror( "bad scalar initialization");
	/* this will never be called with a field element... */
	else inforce( tsize(t,d,s) );

	paramno = 0;
	vfdalign( ALINIT );
	inoff = 0;
	iclass = SNULL;
	/* if we have an ID for which we've been deferring debug output,
	** produce the debug info now.  defer_def is stab number.
	*/
	if ( ISARY(t) && defer_def >= 0)
	    defnamdbg( &stab[defer_def] );

	pstk = 0;			/* no init stack */
}

doinit( p )
register NODE *p; 

{

	/* take care of generating a value for the initializer p */
	/* inoff has the current offset (last bit written)
	** in the current word being generated 
	*/

	register sz, d, s;
	register TWORD t;
	register NODE *q;

	/* note: size of an individual initializer assumed to fit into an int */

	if( iclass < 0 ) goto leave;
	if( iclass == EXTERN || iclass == UNAME )
	{
		uerror( "cannot initialize extern or union" );
		iclass = -1;
		goto leave;
	}

	if( iclass == AUTO || iclass == REGISTER )
	{
		/* do the initialization and get out, without regard 
		** for filling out the variable with zeros, etc. 
		*/
		bccode();
		idname = pstk->in_id;
		p = buildtree( ASSIGN, buildtree( NAME, NIL, NIL ), p );
		ecomp(p);
		return;
	}

	if( p == NIL ) return;  /* throw away strings already made into lists */


# ifndef NODBG
	if( idebug > 1 ) printf( "doinit(%o)\n", p );
# endif

	nextelem(0);		/* get to next scalar */

	t = pstk->in_t;  	/* type required */
	d = pstk->in_d;
	s = pstk->in_s;
	q = block( NAME, NIL, NIL, t, d, s);
	if( t == ENUMTY )	/* change enums to an integer type */
	{
		econvert( q );
		t = q->in.type;
	}
	if( pstk->in_sz < 0 )
	{
		  /* bit field */
		sz = -pstk->in_sz;
	}
	else 
	{
		sz = tsize( t, d, s );
	}

	inforce( pstk->in_off );

	p = buildtree( ASSIGN, q, p );
	p->in.left->in.op = FREE;
	p->in.left = p->in.right;
	p->in.right = NIL;
	p->in.left = optim( p->in.left );
	if( p->in.left->in.op == UNARY AND )
	{
		p->in.left->in.op = FREE;
		p->in.left = p->in.left->in.left;
	}
	p->in.op = INIT;
	p = optim(p);

# ifndef NOFLOAT
	if( p->in.left->in.op == FCON )
	{
		fincode( p->in.left->fpn.dval, sz );
	}
	else
# endif
		if( p->in.left->in.op != ICON )
		{
			uerror( "illegal initialization" );
			inoff += sz;
		}
		else 
		{
			/* a constant is being used as initializer */
			if( sz < SZINT )
			{
				 /* special case: bit fields, etc. */
			incode( p->in.left, sz );
			}
			else 
			{
# ifdef MYINIT
				MYINIT( optim(p), sz );
# else
				ecode( optim(p) );
				inoff += sz;
# endif
			}
		}

	endelem();

leave:
	tfree(p);
}


/* done initializing one scalar; adjust initialization stack */

endelem()
{
    TWORD t;

#ifndef NODBG
    if (idebug) printf("endelem() enter at level %d\n", pstk-instack);
#endif

    /* back up to last unfilled aggregate, but not past need for } */
    while (pstk->in_fl == 0 && pstk > instack) {
	--pstk;				/* back up stack */
	t = pstk->in_t;
	if (t == STRTY) {
	    int ix = ++pstk->in_x;	/* bump structure member number */

	    if (dimtab[ix] >= 0) break;
	}
	else if (ISARY(t)) {
	    int n = ++pstk->in_n;	/* bump array element number */

	    if (n < dimtab[pstk->in_d]) break;
	}
	/* can never have backed up stack to scalar */
    }

#ifndef NODBG
    if (idebug) printf("endelem() exit at level %d\n", pstk-instack);
#endif
    return;
}
    

nextelem(onelevel)
int onelevel;				/* non-zero to go down just one level */
{


    do {
	TWORD t;
	int id;
	struct symtab *p;

#ifndef NODBG
	if (idebug) printf("nextelem() enter at level %d\n", pstk-instack);
#endif

	/* if nothing on stack */
	if (pstk < instack)
	    cerror("confused nextelem()");

	t = pstk->in_t;

	if( t == STRTY )
	{
	    /* next element of struct */
	    if( (id=dimtab[pstk->in_x]) < 0 ) {
		if (pstk->in_fl) {	/* awaiting } ? */
		    uerror("too many struct initializers");
		    break;
		}
		else if (pstk <= instack)
		    break;		/* don't back up too far */
		--pstk;			/* done struct */
		continue;
	    }

	    /* otherwise, put next element on the stack */

	    p = &stab[id];
	    if (p->sclass != MOS && !(p->sclass&FIELD))
		cerror("insane structure member list");

	    instk( id, p->stype, p->dimoff, p->sizoff, p->offset + pstk->in_off );
	}
	else if( ISARY(t) )
	{
	    OFFSZ temp;
	    int n;
	    int dim;

	    /* ready for n-th stack element (0-(n-1)) */
	    n = pstk->in_n;
	    dim = pstk->in_d;
	    if( dimtab[dim] > 0 && n >= dimtab[dim] ) {
		uerror("too many array(2) initializers");
		break;
	    }
		
	    /* put the new element onto the stack */

	    temp = pstk->in_sz;
	    instk( pstk->in_id, (TWORD)DECREF(pstk->in_t), dim+1,
		pstk->in_s, pstk->in_off+n*temp );
	}
	else
	    break;			/* done when hit scalar */
    } while (!onelevel);
#ifndef NODBG
	if (idebug) printf("nextelem() exit at level %d\n", pstk-instack);
#endif
    return;
}

ilbrace()
{
#ifndef NODBG
    if (idebug) printf("ilbrace() enter at level %d\n", pstk-instack);
#endif

    nextelem(1);			/* go down one level only */
    ++pstk->in_fl;			/* bump flag at new level */
#ifndef NODBG
    if (idebug) printf("ilbrace() exit at level %d\n", pstk-instack);
#endif
    return;
}


irbrace()
{
#ifndef NODBG
    if (idebug) printf("irbrace() at level %d\n", pstk-instack);
#endif

    /* back up stack until a flagged { is found */

    for (;;) {
	if (pstk->in_fl) {
	    --pstk->in_fl;		/* one less } to find */
	    endelem();
	    return;
	}
	if (pstk <= instack) return;	/* don't back up beyond beginning */
	--pstk;
    }
}


upoff( size, alignment, poff )
register alignment, *poff; 
{
	/* update the offset pointed to by poff; return the
	** offset of a value of size `size', alignment `alignment',
	** given that off is increasing 
	*/

	register off;

	off = *poff;
	SETOFF( off, alignment );
	if( (maxoffset-off) < size )
	{
		if( instruct!=INSTRUCT )cerror("too many local variables");
		else cerror("Structure too large");
	}
	*poff = off+size;
	return( off );
}

oalloc( p, poff )
register struct symtab *p; 
register *poff; 
{
	/* allocate p with offset *poff, and update *poff */
	register al, off, tsz;
	int noff;

	al = talign( p->stype, p->sizoff );
	noff = off = *poff;
	tsz = tsize( p->stype, p->dimoff, p->sizoff );
#ifdef BACKAUTO
	if( p->sclass == AUTO )
	{
		if( (maxoffset-off) < tsz ) cerror("too many local variables");
		noff = off + tsz;
		SETOFF( noff, al );
		off = -noff;
	}
	else
#endif
#ifdef BACKPARAM
		/* this is rather nonstandard, and may be buggy in some cases */
		/* in particular, won't work with ARGSRET and ARGALLRET */
		if( p->sclass == PARAM )
		{
			if( (maxoffset-off) < tsz ) cerror("too many local variables");
			if( tsz < SZINT ) 
			{
				noff = off + SZINT;
				SETOFF( noff, ALINT );
				off = -noff;
# ifndef RTOLBYTES
				off += tsz;
#endif
			}
			else 
			{
				noff = off + tsz;
				SETOFF( noff, al );
				off = -noff;
			}
		}
		else
#endif
			if( p->sclass == PARAM && ( tsz < SZINT ) )
			{
				off = upoff( SZINT, ALINT, &noff );
# ifndef RTOLBYTES
				off = noff - tsz;
#endif
			}
			else
			{
				off = upoff( tsz, al, &noff );
			}

	if( p->sclass != REGISTER )
	{
		 /* in case we are allocating stack space for register arguments */
		if( p->offset == NOOFFSET ) p->offset = off;
			else if( off != p->offset ) return(1);
	}

	*poff = noff;
	return(0);
}

falloc( p, w, new, pty )
register struct symtab *p; 
register NODE *pty; 
{
	/* allocate a field of width w */
	/* new is 0 if new entry, 1 if redefinition, -1 if alignment */

	register al,sz,type;

	type = (new<0)? pty->in.type : p->stype;

	/* this must be fixed to use the current type in alignments */
	switch( new<0?pty->in.type:p->stype )
	{

	case ENUMTY:
		{
			register s;
			s = new<0 ? pty->fn.csiz : p->sizoff;
			al = dimtab[s+2];
			sz = dimtab[s];
			break;
		}

	case CHAR:
	case UCHAR:
		al = ALCHAR;
		sz = SZCHAR;
		break;

	case SHORT:
	case USHORT:
		al = ALSHORT;
		sz = SZSHORT;
		break;

	case INT:
	case UNSIGNED:
		al = ALINT;
		sz = SZINT;
		break;
#ifdef LONGFIELDS

	case LONG:
	case ULONG:
		al = ALLONG;
		sz = SZLONG;
		break;
#endif

	default:
# ifdef FUNNYFLDS
		if( new < 0 ) 
		{
# endif
			uerror( "illegal field type" );
			al = ALINT;
# ifdef FUNNYFLDS
		}
		else 
		{
			al = fldal( p->stype );
			sz =SZINT;
		}
# endif
	}

	if( w > sz ) 
	{
		uerror( "field too big");
		w = sz;
	}

	if( w == 0 )
	{
		 /* align only */
		SETOFF( strucoff, al );
		if( new >= 0 ) uerror( "zero size field");
		return(0);
	}

	if( strucoff%al + w > sz ) SETOFF( strucoff, al );
	if( new < 0 ) 
	{
		if( (maxoffset-strucoff) < w )
			cerror("structure too large");
		strucoff += w;  /* we know it will fit */
		return(0);
	}

	/* establish the field */

	if( new == 1 ) 
	{
		 /* previous definition */
		if( p->offset != strucoff || p->sclass != (FIELD|w) ) return(1);
	}
	p->offset = strucoff;
	if( (maxoffset-strucoff) < w ) cerror("structure too large");
	strucoff += w;
	p->stype = type;
	return(0);
}

# ifndef EXCLASS
# define EXCLASS EXTERN
# endif

nidcl( p )
register NODE *p; 
{
	 /* handle unitialized declarations */
	/* assumed to be not functions */
	register class;
	register commflag;  /* flag for labelled common declarations */

	commflag = 0;

	/* compute class */
	if( (class=curclass) == SNULL )
	{
		if( blevel > 1 ) class = AUTO;
		else if( blevel != 0 || instruct ) cerror( "nidcl error" );
		else 
		{
			 /* blevel = 0 */
			if( (class = EXCLASS) == EXTERN ) commflag = 1;
		}
	}

	defid( p, class );

	/* if the class for an external definition changes, there must
	** have been a previous definition.  Don't call commdec().
	*/

	if ( commflag &&  class != stab[p->tn.rval].sclass )
		commflag = 0;
	else if( class==EXTDEF || class==STATIC ) 
	{

# ifndef ALLCOMM
		/* simulate initialization by 0 */
		beginit( p->tn.rval );
		endinit();
# else
		commflag = 1;
# endif

	}
	if( commflag && !ISFTN( p->in.type ) ) commdec( p->tn.rval );
}

TWORD
types( t1, t2, t3 )
register TWORD t1, t2, t3; 
{
	/* return a basic type from basic types t1, t2, and t3 */

	TWORD t[3];
	register TWORD  noun, adj, unsg;
	register i;

	t[0] = t1;
	t[1] = t2;
	t[2] = t3;

	unsg = INT;  /* INT or UNSIGNED */
	noun = UNDEF;  /* INT, CHAR, or FLOAT */
	adj = INT;  /* INT, LONG, or SHORT */

	for( i=0; i<3; ++i )
	{
		switch( t[i] )
		{

		default:
bad:
			uerror( "illegal type combination" );
			return( INT );

		case UNDEF:
			continue;

		case UNSIGNED:
			if( unsg != INT ) goto bad;
			unsg = UNSIGNED;
			continue;

		case LONG:
		case SHORT:
			if( adj != INT ) goto bad;
			adj = t[i];
			continue;

		case INT:
		case CHAR:
		case FLOAT:
			if( noun != UNDEF ) goto bad;
			noun = t[i];
			continue;
		}
	}

	/* now, construct final type */
	if( noun == UNDEF ) noun = INT;
	else if( noun == FLOAT )
	{
		if( unsg != INT || adj == SHORT ) goto bad;
		return( adj==LONG ? DOUBLE : FLOAT );
	}
	else if( noun == CHAR && adj != INT ) goto bad;

	/* now, noun is INT or CHAR */
	if( adj != INT ) noun = adj;
	if( unsg == UNSIGNED ) return( noun + (UNSIGNED-INT) );
	else return( noun );
}

NODE *
tymerge( typ, idp )
register NODE *typ, *idp; 

{
	/* merge type typ with identifier idp  */

	register unsigned t;
	register i;

	if( typ->in.op != TYPE ) cerror( "tymerge: arg 1" );
	if(idp == NIL ) return( NIL );

# ifndef NODBG
	if( ddebug > 2 ) eprint(idp);
# endif

	idp->in.type = typ->in.type;
	idp->fn.cdim = curdim;
	tyreduce( idp );
	idp->fn.csiz = typ->fn.csiz;

	for( t=typ->in.type, i=typ->fn.cdim; t&TMASK; t = DECREF(t) )
	{
		if( ISARY(t) ) dstash( dimtab[i++] );
	}

	/* now idp is a single node: fix up type */

	idp->in.type = ctype( idp->in.type );

	if( (t = BTYPE(idp->in.type)) != STRTY && t != UNIONTY && t != ENUMTY )
	{
		idp->fn.csiz = t;  /* in case ctype has rewritten things */
	}

	return( idp );
}

tyreduce( p )
register NODE *p; 

{

	/* build a type, stash away dimensions, from a declaration parse tree */
	/* the type is built top down, the dimensions bottom up */

	register o, temp;
	register unsigned t;

	o = p->in.op;
	p->in.op = FREE;

	if( o == NAME ) return;

	t = INCREF( p->in.type );
	if( o == UNARY CALL ) t += (FTN-PTR);
	else if( o == LB )
	{
		t += (ARY-PTR);
		temp = p->in.right->tn.lval;
		p->in.right->in.op = FREE;
		if ( temp == 0 && p->in.left->tn.op == LB )
		    uerror("null dimension");
	}

	p->in.left->in.type = t;
	tyreduce( p->in.left );

	if( o == LB ) dstash( temp );

	p->tn.rval = p->in.left->tn.rval;
	p->in.type = p->in.left->in.type;

}

fixtype( p, class )
register NODE *p; 
register class;
{
	register unsigned t, type;
	register mod1, mod2;
	/* fix up the types, and check for legality */

	if( (type = p->in.type) == UNDEF ) return;
	if( mod2 = (type&TMASK) )
	{
		t = DECREF(type);
		while( mod1=mod2, mod2 = (t&TMASK) )
		{
			if( mod1 == ARY && mod2 == FTN )
			{
				uerror( "array of functions is illegal" );
				type = 0;
			}
			else if( mod1==FTN && ( mod2==ARY || mod2==FTN ) )
			{
				uerror( "function returns illegal type" );
				type = 0;
			}
			t = DECREF(t);
		}
	}

	/* detect function arguments, watching out for structure declarations */
	/* for example, beware of f(x) struct [ int a[10]; } *x; { ... } */
	/* the danger is that "a" will be converted to a pointer */

	if( class==SNULL && blevel==1 && !(instruct&(INSTRUCT|INUNION)) )
		class = PARAM;
	if( class == PARAM || ( class==REGISTER && blevel==1 ) )
	{
		if( type == FLOAT ) type = DOUBLE;
		else if( ISARY(type) )
		{
			++p->fn.cdim;
			type += (PTR-ARY);
		}
		else if( ISFTN(type) )
		{
			werror( "a function is declared as an argument" );
			type = INCREF(type);
		}
	}

	if( instruct && ISFTN(type) )
	{
		uerror( "function illegal in structure or union" );
		type = INCREF(type);
	}
	p->in.type = type;
}

# ifndef MYCTYPE
TWORD
ctype( t )
register TWORD t; 
{
	register TWORD bt;
	bt = BTYPE(t);

# ifdef NOFLOAT
	if( bt==FLOAT || bt==DOUBLE ) cerror( "ctype:f" );
# endif

# ifdef NOSHORT
# ifdef NOUNSIGNED
	if( bt==SHORT || bt==USHORT ) MODTYPE(t,INT);
# else
	if( bt==SHORT ) MODTYPE(t,INT);
	else if( bt==USHORT) MODTYPE(t,UNSIGNED);
# endif
# endif

# ifdef NOLONG
# ifdef NOUNSIGNED
	if( bt==LONG || bt==ULONG ) MODTYPE(t,INT);
# else
	if( bt==LONG ) MODTYPE(t,INT);
	else if( bt==ULONG) MODTYPE(t,UNSIGNED);
# endif
# endif

	return( t );
}
# endif

uclass( class ) 
register class; 
{
	/* give undefined version of class */
	if( class == SNULL ) return( EXTERN );
	else if( class == STATIC ) return( USTATIC );
	else if( class == FORTRAN ) return( UFORTRAN );
	else return( class );
}

fixclass( class, type )
register TWORD type; 
register class;

{

	/* first, fix null class */

	if( class == SNULL )
	{
		if( instruct&INSTRUCT ) class = MOS;
		else if( instruct&INUNION ) class = MOU;
		else if( blevel == 0 ) class = EXTDEF;
		else if( blevel == 1 ) class = PARAM;
		else class = AUTO;

	}

	/* now, do general checking */

	if( ISFTN( type ) )
	{
		switch( class ) 
		{
		default:
			uerror( "function has illegal storage class" );
		case AUTO:
			class = EXTERN;
		case EXTERN:
		case EXTDEF:
		case FORTRAN:
#ifdef IN_LINE
		case INLINE:
#endif
		case TYPEDEF:
		case STATIC:
		case UFORTRAN:
		case USTATIC:
			;
		}
	}

	if( class&FIELD )
	{
		if( !(instruct&INSTRUCT) ) uerror( "illegal use of field" );
		return( class );
	}

	switch( class )
	{

	case MOU:
		if( !(instruct&INUNION) ) uerror( "illegal class" );
		return( class );

	case MOS:
		if( !(instruct&INSTRUCT) ) uerror( "illegal class" );
		return( class );

	case MOE:
		if( instruct & (INSTRUCT|INUNION) ) uerror( "illegal class" );
		return( class );

	case REGISTER:
		if( blevel == 0 ) uerror( "illegal register declaration" );
		/* if cisreg returns 1, nextrvar has the reg. number */
		else if( cisreg( type ) ) return( class );
		if( blevel == 1 ) return( PARAM );
		else return( AUTO );

	case AUTO:
	case LABEL:
	case ULABEL:
		if( blevel < 2 ) uerror( "illegal class" );
		return( class );

	case PARAM:
		if( blevel != 1 ) uerror( "illegal class" );
		return( class );

#ifdef IN_LINE
	case INLINE:
		if (!ISFTN(type))
			uerror("inline declaration must apply to function");
		else
		{
			type = DECREF(type);
			if (ISFTN(type) || ISARY(type))
				uerror("inline function has bad type");
		}
		return (class);
#endif


	case UFORTRAN:
	case FORTRAN:
# ifdef NOFORTRAN
		NOFORTRAN;    /* a condition which can regulate the FORTRAN usage */
# endif
		if( !ISFTN(type) ) uerror( "fortran declaration must apply to function" );
		else 
		{
			type = DECREF(type);
			if( ISFTN(type) || ISARY(type) || ISPTR(type) ) 
			{
				uerror( "fortran function has wrong type" );
			}
		}
		return( class );

	case EXTERN:
	case STATIC:
	case EXTDEF:
	case USTATIC:
		/* these aren't legal when we're looking at function arguments */
		if (blevel == 1) {
		    uerror("illegal storage class for argument");
		    class = PARAM;		/* just to keep things going */
		}
		/*FALLTHRU*/
	case TYPEDEF:
	case STNAME:
	case UNAME:
	case ENAME:
		return( class );

	default:
		cerror( "illegal class: %d", class );
		/* NOTREACHED */
	}
}

struct symtab *
mknonuniq(idindex)
register *idindex; 
{
	/* locate a symbol table entry for */
	/* an occurrence of a nonunique structure member name */
	/* or field */
	register struct symtab * new;
	register struct symtab * old;
	int i = st_getnew();

	new = &stab[ i ];		/* point at new entry */
	old = &stab[ *idindex ];
	new->sflags = SNONUNIQ | SMOS;
	new->sname = old->sname;	/* old entry name */
# ifndef NODBG
	if( ddebug )
	{
		printf( "\tnonunique entry for %s from %d to %d\n",
		new->sname, *idindex, i );
	}
# endif
	/* set up linkages */
	new->st_next = *old->st_own;	/* link to beginning of chain */
	*old->st_own = i;
	new->st_own = old->st_own;
	*idindex = i;
	return ( new );
}

lookup( name, s)
register char *name; 
{
	/* look up name: must agree with s w.r.t. STAG and SMOS */

	register i;
	register struct symtab *sp;
	unsigned int hash = st_hashf(name);

	/* compute initial hash index */
# ifndef NODBG
	if( ddebug > 2 )
	{
		printf( "lookup( %s, %d ), stwart=%d, instruct=%d\n",
			name, s, stwart, instruct );
	}
# endif

	for ( i = st_hash[ hash ]; i > 0; i = sp->st_next ) {
	    sp = &stab[i];

	    /* look for name */
	    if( (sp->sflags & (STAG|SMOS|SLABEL)) != s ) continue;
	    if ( sp->sname == name )
		return( i );
	}
	/* Entry not found:  get one, fill in partially.
	** (Beware!! Must NOT do &stab[ i = st_getnew() ],
	** because the value of "stab" could change with the
	** st_getnew() ).
	*/
	i = st_getnew();
	sp = &stab[ i ];

	sp->sflags = s;  /* set STAG, SMOS if needed, turn off all others */
	sp->sname = name;
	sp->stype = UNDEF;
	sp->sclass = SNULL;
	/* link in on current chain */
	sp->st_own = &st_hash[hash];
	sp->st_next = st_hash[hash];
	st_hash[hash] = i;
	return( i );
}

/* Look up symbol as label.  Assume that we may be given a new
** symbol table entry because the symbol wasn't found on the
** previous lookup.  Free it, look for a symbol with flag SLABEL,
** and return it.
*/

static int
looklab(id)
register int id;			/* stab id of original symbol */
{
    if (stab[id].sclass == SNULL) {	/* new entry? */
	if (*stab[id].st_own != id)
	    cerror("confused looklab()");
	*stab[id].st_own = stab[id].st_next; /* unlink from bucket */
	stab[id].stype = TNULL;		/* make entry available again */
#ifdef STATSOUT
	--td_stab.td_used;		/* one less entry used */
#endif
    }
    /* look the symbol up as a label, and return result */
    return( idname = lookup(stab[id].sname, SLABEL) );
}


#ifndef checkst
/* if not debugging, make checkst a macro */
checkst(lev)
{
	register int i, j;
	register struct symtab *p, *q;

	for( i=0, p=stab; i<SYMTSZ; ++i, ++p )
	{
		if( p->stype == TNULL ) continue;
		j = lookup( p->sname, p->sflags&(SMOS|STAG) );
		if( j != i )
		{
			q = &stab[j];
			if (   q->stype == UNDEF
			    /* can have same structure/union member at same level */
			    || (q->slevel <= p->slevel && !(p->sflags&q->sflags&SMOS))
			    )
			{
				void st_print();
#ifndef	NODBG
				st_print();
#endif
				cerror( "check error: %s", q->sname );
			}
		}
		else if( p->slevel > lev ) cerror( "%s check at level %d", p->sname, lev );
	}
}
#endif


clearst( lev )
{
	/* clear entries of internal scope from the symbol table */
	int i;
	int savehead = 0;		/* list header of entries to move */

	if (lev >= MAXNEST) return;	/* no symbols at this level yet */

# ifdef STABS
	/* do this first, so structure members don't get clobbered
	** before they are printed out... 
	*/
	aobeg();
	for ( i = scopestack[lev]; i > 0; i = stab[i].st_scopelink )
		aocode(&stab[i]);
	aoend();
# endif

	/* For each entry at a given scope level, go back to the owning
	** hash table entry and eliminate all entries on that list that
	** have scope >= the scope level in question.
	** NOTE:  It is possible, with this algorithm, to free a symtab
	** entry that is further along on the scopestack queue.  Since
	** the various queue pointers remain intact, we can still follow
	** the queue.
	** C wart:  save function definitions and labels and move them to
	** appropriate outer scopes.
	*/

	for (i = scopestack[lev]; i > 0; i = stab[i].st_scopelink) {
	    int j;
	    unsigned short * hashhead = stab[i].st_own;

	    /* eat up entries at higher levels on this hash bucket */
	    while ((j = *hashhead) > 0) {
		register struct symtab * sp = &stab[j];

		/* TNULL entries have already been eaten */
		if (sp->stype != TNULL && sp->slevel < lev)
		    break;
# ifndef NODBG
		if (ddebug)
		    printf("removing %s = stab[ %d], flags %o level %d\n",
				sp->sname, j, sp->sflags, sp->slevel);
# endif
		if (sp->stype == UNDEF)
		    cerror("%s undefined", sp->sname);
		if ( sp->sclass == ULABEL && lev <= 2)
		    uerror("undefined label %s", sp->sname);
		*hashhead = sp->st_next;

		/* remember entries to move down the queue */
		switch( sp->sclass ) {
		case LABEL:
		case ULABEL:
		    sp->slevel = 2;
		    break;

		case EXTERN:
		case UFORTRAN:
		case FORTRAN:
		    sp->slevel = 0;
		    break;
		}
		if (sp->slevel < lev) { /* changed the level above */
		    sp->st_next = savehead;
		    savehead = j;
		}
		else {
#ifdef STATSOUT
		    --td_stab.td_used;
#endif
		    sp->stype = TNULL;	/* make this entry free */
		}
	    }
	}
	scopestack[lev] = 0;		/* empty scope list */

	/* Now link in the proper place all of the entries we saved
	** off to the side.  Note that we couldn't have done this at
	** the same time we "removed" the entry because the linkages
	** (mainly scopelink) would have gotten badly mangled.
	*/
	for (i = savehead; i > 0; ) {
	    int j = i;			/* hang onto current entry number */
	    struct symtab * sp = &stab[j];
	    int slev = sp->slevel;	/* get this entry's level */
	    unsigned short * behind;	/* back pointer for walking buckets */

	    i = sp->st_next;		/* for the next iteration */

	    /* thread on appropriate scope linkage */
	    sp->st_scopelink = scopestack[slev];
	    scopestack[slev] = j;

	    /* thread on bucket list */
	    for ( behind = &st_hash[st_hashf(sp->sname)]; *behind != 0; ) {
		struct symtab * q = &stab[*behind];

		if (q->slevel <= slev) break;
		behind = &q->st_next;
	    }
	    /* *behind is either 0 or the pointed-to entry is lower (or same)
	    ** scope level than the one we want to add.
	    */
	    sp->st_next = *behind;
	    *behind = j;
	}
}

/* Hide an entry:  used only under error conditions.
** Caller links on scopestack.
*/

static int
hide( idn )
int idn;				/* symbol table entry to hide */
{
	int new = st_getnew();		/* get another entry */
	register struct symtab *p = &stab[idn]; 
	register struct symtab *q = &stab[new];

	*q = *p;
	q->sflags &= ~SISREG;		/* new symbol isn't necessarily reg. */
	/* link on appropriate bucket and scope list */
	q->st_next = *p->st_own;
	*p->st_own = new;

	if( hflag ) werror( "%s redefinition hides earlier one", p->sname );
# ifndef NODBG
	if( ddebug ) printf( "	%d hidden in %d\n", p-stab, q-stab );
# endif
	return( idname = new );
}

/* Check an entry for duplicate externs with a different type. */

static
checkext(idn, type)
int idn;				/* new entry (at head of list */
TWORD type;
{
    register struct symtab * pnew = &stab[idn];
    register int i;

    for (i = pnew->st_next; i > 0; i = stab[i].st_next) {
	/* stop at first extern of same name; warn if mismatch */
	if (stab[i].sclass == EXTERN && stab[i].sname == pnew->sname) {
	    if(stab[i].stype != type)
		werror("inconsistent redeclaration of external %s\n",
			pnew->sname);
	    break;
	}
    }
    return;
}

/* get new symbol table entry */

static int
st_getnew()
{
    static int nextfree = 0;		/* place before place to check first */
    int oldsize;

    register int try = nextfree;

    do {
	/* never allocate table entry 0:  used as flag */
	if (++try >= SYMTSZ) try = 1;
	if (stab[try].stype == TNULL) {
#ifdef STATSOUT
	    if (td_stab.td_max < ++td_stab.td_used)
		td_stab.td_max = td_stab.td_used;
#endif
	    return( nextfree = try );
	}
    } while (try != nextfree);

    /* if enlarging works, will want to allocated one beyond current size */
    nextfree = SYMTSZ;			/* value of SYMTSZ will change */
    oldsize = td_enlarge(&td_stab,0);	/* enlarge symbol table */
#if TNULL != 0
    {					/* must make entries available */
	int i;
	for (i = oldsize+1; i < SYMTSZ; ++i)
	    stab[i].stype = TNULL;
    }
#endif
#ifdef STATSOUT
    if (td_stab.td_max < ++td_stab.td_used)
	td_stab.td_max = td_stab.td_used;
#endif
    return( nextfree);
}

/* print symbol table information for debugging purposes */

#ifndef NODBG
static void
st_print()
{
    register int i;

    printf("+++++++\n\n");

    for (i = 0; i < MAXNEST; ++i) {
	if (scopestack[i] != 0)
	    printf("scopestack[%d]	%d\n", i, scopestack[i]);
    }
    putchar('\n');

    for (i = 0; i < HASHTSZ; ++i) {
	if (st_hash[i] != 0)
	    printf("hash[%3d]	%d\n", i, st_hash[i]);
    }
    putchar('\n');

    for (i = 0; i < SYMTSZ; ++i) {
	if (stab[i].stype != TNULL) {
	    struct symtab * sp = &stab[i];
	    printf("stab[%3d]	level = %d, next = %d, scopelink = %d, own = %d, \
\"%s\"\n",
			i, sp->slevel, sp->st_next, sp->st_scopelink,
			sp->st_own-st_hash, sp->sname);
	}
    }
    printf("\n------\n");
}
#endif
