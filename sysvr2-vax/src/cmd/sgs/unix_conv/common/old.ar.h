/*	@(#)old.ar.h	2.3	*/

/* archive file header format */


#define	OARMAG	0177545
struct	oar_hdr {
	char	ar_name[14];
	long	ar_date;
	char	ar_uid;
	char	ar_gid;
	int	ar_mode;
	long	ar_size;
};
