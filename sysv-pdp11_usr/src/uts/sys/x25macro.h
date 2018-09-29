/*	x25macro.h 1.2 of 12/17/81
	@(#)x25macro.h	1.2
 */

/*
 *	This file contains definitions of macros that are used to
 *	access the items in the level 3 packet header.
 */




/*	get the packet receive sequence number	*/
#define x25getpr(PKTHDR) (((PKTHDR->A_PR) & X25_PR) >> H_PR)


/*	get the packet send sequence number	*/
#define x25getps(PKTHDR) (((PKTHDR->A_PS) & X25_PS) >> H_PS)


/*	get the qualifier bit	*/
#define x25getqbit(PKTHDR) (((PKTHDR->A_QBIT) & X25_QBIT) >> H_QBIT)


/*	get the confirm bit	*/
#define x25getdbit(PKTHDR) (((PKTHDR->A_DBIT) & X25_DBIT) >> H_DBIT)


/*	get the general format indicator	*/
#define x25getgfi(PKTHDR) (((PKTHDR->A_GFI) & X25_GFI) >> H_GFI)


/*	get the logical channel group number	*/
#define x25getcgn(PKTHDR) (((PKTHDR->A_LCGN) & X25_LCGN) >> H_LCGN)


/*	get the logical channel number	*/
#define x25getcn(PKTHDR) (((PKTHDR->A_LCN) & X25_LCN) >> H_LCN)


/*	get the more data bit	*/
#define x25getmbit(PKTHDR) (((PKTHDR->A_MBIT) & X25_MBIT) >> H_MBIT)


/*	get the data packet indicator	*/
#define x25getdflg(PKTHDR) (((PKTHDR->A_DFLG) & X25_DFLG) >> H_DFLG)


/*	get the supervisory packet type	*/
#define x25getstype(PKTHDR) (((PKTHDR->A_STYPE) & X25_STYPE) >> H_STYPE)


/*	get the unnumbered packet type	*/
#define x25getutype(PKTHDR) (((PKTHDR->A_UTYPE) & X25_UTYPE) >> H_UTYPE)


/*	set the packet receive sequence number to the specified value	*/
#define x25setpr(PKTHDR,VALUE) \
		PKTHDR->A_PR &= ~(X25_PR), \
		PKTHDR->A_PR |= ((VALUE << H_PR) & X25_PR)


/*	set the packet send sequence number to the specified value	*/
#define x25setps(PKTHDR,VALUE) \
		PKTHDR->A_PS &= ~(X25_PS), \
		PKTHDR->A_PS |= ((VALUE << H_PS) & X25_PS)


/*	set the qualifier bit to the specified value	*/
#define x25setqbit(PKTHDR,VALUE) \
		PKTHDR->A_QBIT &= ~(X25_QBIT), \
		PKTHDR->A_QBIT |= ((VALUE << H_QBIT) & X25_QBIT)


/*	set the confirm bit to the specified value	*/
#define x25setdbit(PKTHDR,VALUE) \
		PKTHDR->A_DBIT &= ~(X25_DBIT), \
		PKTHDR->A_DBIT |= ((VALUE << H_DBIT) & X25_DBIT)


/*	set the general format indicator to the specified value	*/
#define x25setgfi(PKTHDR,VALUE) \
		PKTHDR->A_GFI &= ~(X25_GFI), \
		PKTHDR->A_GFI |= ((VALUE << H_GFI) & X25_GFI)


/*	set the logical channel group number to the specified value	*/
#define x25setcgn(PKTHDR,VALUE) \
		PKTHDR->A_LCGN &= ~(X25_LCGN), \
		PKTHDR->A_LCGN |= ((VALUE << H_LCGN) & X25_LCGN)


/*	set the logical channel number to the specified value	*/
#define x25setcn(PKTHDR,VALUE) \
		PKTHDR->A_LCN &= ~(X25_LCN), \
		PKTHDR->A_LCN |= ((VALUE << H_LCN) & X25_LCN)


/*	set the more data bit to the specified value	*/
#define x25setmbit(PKTHDR,VALUE) \
		PKTHDR->A_MBIT &= ~(X25_MBIT), \
		PKTHDR->A_MBIT |= ((VALUE << H_MBIT) & X25_MBIT)


/*	set the data packet indicator to the specified value	*/
#define x25setdflg(PKTHDR,VALUE) \
		PKTHDR->A_DFLG &= ~(X25_DFLG), \
		PKTHDR->A_DFLG |= ((VALUE << H_DFLG) & X25_DFLG)


/*	set the supervisory packet type to the specified value	*/
#define x25setstype(PKTHDR,VALUE) \
		PKTHDR->A_STYPE &= ~(X25_STYPE), \
		PKTHDR->A_STYPE |= ((VALUE << H_STYPE) & X25_STYPE)


/*	set the unnumbered packet type to the specified value	*/
#define x25setutype(PKTHDR,VALUE) \
		PKTHDR->A_UTYPE &= ~(X25_UTYPE), \
		PKTHDR->A_UTYPE |= ((VALUE << H_UTYPE) & X25_UTYPE)
