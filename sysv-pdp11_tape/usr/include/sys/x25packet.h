/*	x25packet.h 1.3 of 12/18/81
	@(#)x25packet.h	1.3
 */

/* this file contains the layout of the BLX.25 level 3 packet header */

/*	layout of the level 3 packet header common for all level 3
 *	packet types - this is contained in the buffer descriptor
 *	structure vpmbd
 */

struct level_3 {
	char x25_oct1;		/* octet 1 of the header */
	char x25_oct2;		/* octet 2 of the header */
	char x25_oct3;		/* octet 3 of the header */
};

/*	layout for x25_octet 1 of the level 3 header
 *
 *	ABCCDDDD
 *
 *		A - qualifier bit
 *		B - delivery conformation bit
 *		C - general format identifier
 *		D - logical channel group number
 */

/*	mask of x25_octet 1 items	*/
#define X25_QBIT 0x80
#define X25_DBIT 0x40
#define X25_GFI  0x30
#define X25_LCGN 0x0f

/*	address of x25_octet 1 items	*/
#define A_QBIT x25_oct1
#define A_DBIT x25_oct1
#define A_GFI x25_oct1
#define A_LCGN x25_oct1

/*	displacement of x25_octet 1 items	*/
#define H_QBIT 7
#define H_DBIT 6
#define H_GFI 4
#define H_LCGN 0

/*	layout for x25_octet 2 of the level 3 header
 *
 *	AAAAAAAA
 *
 *		A - logical channel number
 */

/*	mask of x25_octet 2 items		*/
#define X25_LCN 0xff

/*	address of x25_octet 2 items	*/
#define A_LCN x25_oct2

/*	displacement of x25_octet 2 items	*/
#define H_LCN 0

/*	layout for x25_octet 3 of the level 3 header
 *
 *	the layout of the third word of the header is
 *	dependent on the packet type
 */

/*	layout of x25_octet 3 for an information packet
 *	packet types -
 *		data
 *
 *	AAABCCCD
 *
 *		A - packet receive sequence number
 *		B - more data indicator (M bit)
 *		C - packet send sequence number
 *		D - data packet indicator (if 0)
 */

/*	mask of x25_octet 3 items		*/
#define X25_PR 0xe0
#define X25_MBIT 0x10
#define X25_PS 0x0e
#define X25_DFLG 0x01

/*	address of x25_octet 3 items	*/
#define A_PR x25_oct3
#define A_MBIT x25_oct3
#define A_PS x25_oct3
#define A_DFLG x25_oct3

/*	displacement of x25_octet 3 items	*/
#define H_PR 5
#define H_MBIT 4
#define H_PS 1
#define H_DFLG 0


/*	layout of x25_octet 3 for a supervisory packet
 *	packet types -
 *		RR (receiver ready)
 *		RNR (receiver not ready)
 *		REJ (data reject)
 *
 *	AAABBBBB
 *
 *		A - packet receive sequence number
 *		B - supervisory packet type
 */

/*	mask of x25_octet 3 items		*/
#define X25_STYPE 0x1f

/*	address of x25_octet 3 items	*/
#define A_STYPE x25_oct3

/*	displacement of x25_octet 3 items	*/
#define H_STYPE 0


/*	layout of x25_octet 3 for an unnumbered packet
 *	packet types -
 *		RST (link group restart)
 *		RSTC (link group restart confirm)
 *		RES (channel reset)
 *		RESC (channel reset confirm)
 *		INT (interrupt)
 *		INTC (interrupt confirm)
 *
 *	AAAAAAAA
 *
 *		A - unnumbered packet type
 */

/*	mask of x25_octet 3 items		*/
#define X25_UTYPE 0xff

/*	address of x25_octet 3 items	*/
#define A_UTYPE x25_oct3

/*	displacement of x25_octet 3 items	*/
#define H_UTYPE 0

/*	states for information packet type
 */
#define T_DATA 0

/*	states for supervisory packet type
 */
#define T_RR 0x1
#define T_RNR 0x5
#define T_REJ 0x9

/*	states for unnumbered packet type
 */
#define T_RST 0xfb
#define T_RSTC 0xff
#define T_RES 0x1b
#define T_RESC 0x1f
#define T_INT 0x23
#define T_INTC 0x27

/*	values for valid packet lengths */

#define X25DATAL	3
#define X25RRL		3
#define X25RNRL		3
#define X25REJL		3
#define X25RSTL		4
#define X25RSTCL	3
#define X25RESL		4
#define X25RESCL	3
#define X25INTL		4
#define X25INTCL	3




/*	values for items in the level 3 packet header	*/


/*	states for general format indicator */
#define X25DTEGFI 0x1

/*	states for cause field */
#define X25CAUSE 0

/*	states for diagnostic codes */
#define X25NODGNC	0
#define	X25INVPS	1
#define	X25INVPR	2
/*	the following are for packet types invalid for a given state	*/
#define	X25INVTYP	16
#define	X25INVR1	17
#define	X25INVR2	18
#define	X25INVD1	27
#define	X25INVD2	28
#define	X25INVD3	29
#define	X25BADSTATE	30
/*	the following are for packets that are not allowed	*/
#define	X25NAPKT	32
#define	X25UIDPKT	33
#define	X25INVPVC	35
#define	X25SHORTP	38
#define	X25LONGP	39
#define	X25INVGFI	40
#define	X25INVRST	41
#define	X25UAINC	43
#define	X25UAINT	44
#define	X25XREJ		45
#define	X25INVMBIT	47
/*	the following are for expired timers	*/
#define X25TORES	51
#define X25TORST	52
/*	the following are for additional DTE codes not covered above	*/
#define	X25OUTORD	129
#define	X25INSERV	130
#define X25INSPVC	131
#define	X25TOINTC	145
#define	X25TODATA	146
#define	X25TOREJ	147
