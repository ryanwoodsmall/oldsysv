/*	@(#)chdir.c	1.1	*/

# include <stand.h>

chdir (dirname)
char *dirname; {
	char cbuf[NAMSIZ];
	extern char *_cdir;
	extern char *malloc ();

	_cond (dirname, cbuf);

	if (_cdir) {
		free (_cdir);
		_cdir = 0;
	}

	_cdir = malloc ((unsigned) (strlen (cbuf) + 1));
	strcpy (_cdir, cbuf);
	return (0);
}
