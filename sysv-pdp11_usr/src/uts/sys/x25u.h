/*	x25u.h 1.3 of 2/25/82
	@(#)x25u.h	1.3
 */

/*	session accept/disconnect/connect/reject messages */

/*
 *	session control messages have the following general format:
 *
 *	slcmh	-- session layer control kessage header
 *	     	   first char b00100001 followed by char message type
 *
 *	esi	-- one or more extended session items; may be intermingled
 *		   with session items (si--below) 
 *		   first char with high 2 bits b11, low 6 are session item code
 *		   next char is length of parameter info, 0-255
 *		   next is 0-255 bytes of parameter info
 *
 *	si	-- one or more session items; may be intermingled with
 *		   extended session items (esi--above)
 *		   first char has high 4 bits b0011 followed by 4 bit parameter
 *		   length, 0-14 (not 15!)
 *		   next char is session item code
 *		   next is 0-14 bytes of parameter info
 *
 *	eoh	-- end of headings item is last
 *		   char has high bit set only b010000000 or 0200
 */
struct sesscon {	/* session connect message */
	char	c_slcmh;		/* message header */
	char	c_messtype;	/* message type--connect code 0 */
	char	c_esicd1;	/* called port address */
	char	c_esiln1;	/* esi with code 01, len 0 */
	char	c_esicd2;	/* calling port address */
	char	c_esiln2;	/* esi with code 02, len 0 */
	char	c_siln1;		/* origin type */
	char	c_sicd1;	/* si with len 1 code 1 */
	char	c_otype;		/* origin type 05 BX.25 application */
	char	c_siln2;		/* session type */
	char	c_sicd2;	/* si with len 01 code 02 */
	char	c_stype;		/* session type 0 two-way simultaneous */
	char	c_siln3;		/* presentation type */
	char	c_sicd3;	/* si with len 01 code 06 */
	char	c_ptype;		/* presentation type 0 binary data */
	char	c_eoh;		/* end of heading item */
};

struct sessacc {	/* session accept message */
	char	a_slcmh;		/* message header */
	char	a_messtype;	/* session accept type 01 */
	char	a_siln1;		/* presentation type */
	char	a_sicd1;	/* si with len 01 code 06 */
	char	a_ptype;		/* presentation type 0 binary data */
	char	a_eoh;		/* end of heading item */
};

struct sessrej {	/* session reject message */
	char	r_slcmh;		/* message header */
	char	r_messtype;	/* session reject type 02 */
	char	r_siln1;		/* reject reason */
	char	r_sicd1;	/* si with len 01 code 04 */
	char	r_reason;		/* reject reason--port unavail 0 */
	char	r_eoh;		/* end of heading item */
};

struct sessdis {	/* session disconnect message */
	char	d_slcmh;		/* message header */
	char	d_messtype;	/* session disconnect type 03 */
	char	d_siln1;		/* disconnect reason */
	char	d_sicd1;	/* si with len 01 code 05 */
	char	d_reason;		/* disconnect reason--normal 0 */
	char	d_eoh;		/* end of heading item */
};

/*	Session protocol options for Perm Virt Circuits */
#define PVC_SESS	02
#define PVC_RST		01
#define PVC_NONE	0

/*	ioctl commands */
#define X25GET	(('x'<<8) | 0)
#define X25SET	(('x'<<8) | 1)
#ifdef CBUNIX
#define X25FCNTL	(('x'<<8) | 2)
#endif

struct x25sctl {
	unsigned short
		c_delconf : 1,	/* delivery confirmation requested */
		c_isreset : 1,	/* channel has been reset */
		c_ndelay:1,	/* no delay requested */
		:13;
};

struct x25gctl {
	struct t_state	c_t_state;	/* tab state structure */
	struct l_state	c_l_state;	/* link state structure */
	struct s_flags	c_s_flags;	/* slot flag structure */
	unsigned char	c_xmtq;		/* length of lev 3 xmtq */
	unsigned char	c_recvq;	/* length of lev 3 recvq */
	unsigned char	c_rststate;	/* restart state of link */
	struct l_line	c_primline;	/* primary line info */
	struct l_line	c_bckline;	/* backup line info */
};
