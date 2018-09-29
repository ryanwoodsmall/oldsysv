/*	@(#)cvt.c	1.3	*/
/*LINTLIBRARY*/
/*
 * Floating output conversion
 */

#define	PLUS	01
#define	MINUS	02
#define	SHARP	04
#define	BLANK	010
#define	ISNEG	01000
#define	UCASE	02000

extern char *ecvt(), *fcvt();

char *
_cvt(number, ndigit, buf, code, flag)
double	number;
register int ndigit;
int	code, flag;
char	*buf;
{
	int	sign, decpt, trimzero;
	register char *p1, *p2;

	trimzero = 1;
	if(flag&SHARP || (code != 'g' && ndigit > 0))
		trimzero = 0;
	if(code == 'e')
		ndigit++;
	p1 = (*(code == 'f'? fcvt:ecvt))(number, ndigit, &decpt, &sign);
	if(code == 'e' && number == 0)
		decpt = 1;
	p2 = buf;
	if(sign)
		*p2++ = '-';
	else if(flag&PLUS)
		*p2++ = '+';
	else if(flag&BLANK)
		*p2++ = ' ';
	if(code == 'e' || code == 'g' &&	/* E-style */
			(decpt > ndigit || decpt <= -4)) {	
		decpt--;
		if(ndigit <= 0)
			*p1 = '0';
		*p2++ = *p1++;
		*p2++ = '.';
		while(--ndigit > 0)
			*p2++ = *p1++;
		if(trimzero) {
			while(p2 > buf && *--p2 == '0')
				;
			if(*p2 != '.')
				p2++;
		}
		*p2 = 'e';
		if(flag&UCASE)
			*p2 = 'E';
		p2++;
		if(decpt < 0) {
			decpt = -decpt;
			*p2++ = '-';
		} else
			*p2++ = '+';
		*p2++ = decpt/10 + '0';
		*p2++ = decpt%10 + '0';
	} else {
		if(decpt <= 0) {
			if(code == 'g') {
				ndigit -= decpt;
			}
			*p2++ = '0';
			*p2++ = '.';
			while(decpt < 0 && ndigit > 0) {
				*p2++ = '0';
				decpt++;
				ndigit--;
			}
		}
		if(code == 'f')
			ndigit += decpt;
		while(ndigit > 0) {
			*p2++ = *p1++;
			--decpt;
			--ndigit;
			if(decpt == 0)
				*p2++ = '.';
		}
		if(decpt > 0) {
			do {
				*p2++ = '0';
			} while(--decpt);
			*p2++ = '.';
		}
		if(trimzero) {
			while(p2 > buf && *--p2 == '0')
				;
			if(*p2 != '.')
				p2++;
		}
	}
	return(p2);
}
