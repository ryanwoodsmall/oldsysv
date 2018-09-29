/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libcrypt_x:crypt.c	1.1"
void setkey (key)
char *key;
{
	extern void	des_setkey();
	des_setkey(key);
}

void encrypt(block, edflag)
char *block;
int edflag;
{
	extern void	des_encrypt();
	des_encrypt(block, edflag);
}

char *crypt(pw, salt)
char *pw;
char *salt;
{
	extern char	*des_crypt();
	
	return(des_crypt(pw, salt));
}
