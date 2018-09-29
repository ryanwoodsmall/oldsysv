struct filelist {
	char		*file;
	LDFILE		*ldptr;
	struct filelist	*nextitem;
};

#define FILELIST	struct filelist
#define LISTSZ	sizeof(FILELIST)

/*
*     static char	ID_filedefs[ ] = "@(#) filedefs.h: 1.1 1/11/82";
*/
