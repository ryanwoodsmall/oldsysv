/* @(#)nsc.h	1.3 */

#define	NSCSTATS	1	/* 1 to enable read/write delay histograms */
#define	NSCDARD		0	/* 1 to enable driver assisted reads */
				/* WARNING -- allocates DATASZ bytes of memory per active connection */

#define CTRLSZ		10	/* size of msg block ctrl info */
#define MSGDTA		54	/* size of data area in msg block */
#define MSGSZ		64	/* total size of msg block */
#define DATASZ		4096	/* total size of data block */


/* Network Adapter Message */

struct	nmsg
{
	char	nm_adata;	/* associated data flag */
	char	nm_trunk;	/* trunk selection */
	char	nm_acode;	/* access code */
	char	nm_vchan;	/* virual channel of this x-mission */
	short	nm_tonad;	/* "to" network address */
	short	nm_frnad;	/* "from" network address */
	char	nm_fnc;		/* driver protocol function code */
	char	nm_opcod;	/* remote adapter operation code */
	char	nm_data[MSGDTA];/* whatever seems appropriate */
	char	nm_spare[2];	/* not necessary except to solve known A-410 problems */
};

/*  Adapter status returned bytes */

struct adptrst
{
	char	st_afc;		/* last function code */
	char	st_gsw;		/* general status word */
	char	st_trkst;	/* returned trunk status */
	char	st_trkrsp;	/* trunk response */
	char	st_err;		/* general error code */
	char	st_rtnrsp;	/* remote returned response */
	char	st_internal;	/* used by adapter */
	char	st_spare[3];	/* unused */
};

/* general status work masks */

#define G_PORT		0300	/* port id */
#define G_RCVABT	020	/* receiver abort, check receiver status */
#define G_MSGRCV	010	/* message received and in adapter */
#define G_OPABRT	04	/* operation aborted */
#define G_FNRJCT	02	/* function rejected by adapter */
#define G_PARERR	01	/* parity error */

/* trunk status masks */

#define	T_AVAIL		0360	/* trunks specfied that are available */
#define	T_HDRERR	010	/* header fail on response */
#define	T_TIMEO		04	/* trunk timeout - remote did not answer */
#define	T_LRSVD		02	/* local adapter reserved locally */
#define	T_RESRVD	01	/* local trunk reserved by a remote adapter */

/* trunk response masks */

#define	T_TBAD		0200	/* all trunks specified are not available */
#define	T_DMANT		0100	/* dead man's timer expired */
#define	T_RSPR		040	/* response reject, check response byte */
#define	T_RCERR		020	/* response checkword (CRC) error */
#define	T_PARITY	010	/* parity error on DMA onto trunk */
#define	T_INVR		001	/* invalid response by remote */

/* receiver status masks */

#define R_RJCT		0200	/* function rejected - another bit is set */
#define R_BUSY		0100	/* receiving adapter temporarily busy */
#define R_RRESRV	040	/* receiving adapter reserved by another adapter */
#define R_DRESRV	020	/* receiving adapter reserved by attached device */
#define R_FRJCT		010	/* flag reject */
#define	R_IFRF		04	/* illegal frame function detected */
#define R_DCERR		02	/* data checkwork error */
#define R_SEQB		01	/* sequence bit */

/* Adapter error codes, placed in st_err above */

#define	E_CRMOD		1	/* Can't clear rcv mode - a-data in adapter */
#define	E_MSGL		2	/* incorrect message length (<9, >64) */
#define	E_SPAC		3	/* no space available in adapter */
#define	E_DOWN		4	/* "to" port marked down */
#define	E_ABIG		5	/* no space available for associated data */
#define	E_LARG		6	/* associated data too large for adapter */
#define	E_FUNC		7	/* illegal function code */
#define	E_FEWP		10	/* not enough parameters */
#define	E_MANYP		11	/* too many parameters */
#define	E_FEWD		12	/* not enough data sent to adapter */
#define	E_MUCHD		13	/* too much data send to adapter */
#define	E_DATA		14	/* not all data read from adapter */
#define	E_TDAT		15	/* host attempted to read too much data */
#define	E_NOMSG		16	/* no messages waiting to input */
#define	E_MCLR		17	/* host master-cleared during transfer */
#define	E_QUIT		18	/* host quit early, not all data xferred */
#define	E_LPAR		19	/* parity error on adapter to host xfer */
#define	E_TABT		20	/* trunk abort - see bytes 2-4 */
#define	E_LRSV		21	/* device reserved to this adapter */
#define	E_TRY		22	/* trunk x-missions counter expired */
#define	E_NODAT		23	/* remote adapter did not send a-data */

/* NSC A410 Adapter Function Codes */

#define	ATM	0005	/* Transmit Message	*/
#define	ATD	0010	/* Transmit Data	*/
#define	ATLSTD	0014	/* Transmit Last Data	*/
#define	ATLM	0021	/* Transmit Local Message	*/
#define	AIM	0045	/* Input Message	*/
#define	AID	0050	/* Input Data		*/
#define	ASTAT	0101	/* Status		*/
#define	AMDP0	0140	/* Mark Down Port 0	*/
#define	AMDP1	0144	/* Mark Down Port 1	*/
#define	AMDP2	0150	/* Mark Down Port 2	*/
#define	AMDP3	0154	/* Mark Down Port 3	*/
#define	AMDR0	0160	/* Mark Down Port 0 & Reroute Msgs	*/
#define	AMDR1	0164	/* Mark Down Port 1 & Reroute Msgs	*/
#define	AMDR2	0170	/* Mark Down Port 2 & Reroute Msgs	*/
#define	AMDR3	0174	/* Mark Down Port 3 & Reroute Msgs	*/
#define	ARST	0241	/* Read Statistics	*/
#define	ARCST	0245	/* Read & Clear Statistics	*/
#define	ASTST	0300	/* Set Test		*/
#define	ASAL	0305	/* Set Address & Length	*/
#define	AWA	0311	/* Write Adapter	*/
#define	ARA	0315	/* Read Adapter		*/
#define	ACA	0340	/* Clear Adapter	*/
#define	AEOP	0344	/* End Operation	*/
#define ACLWM	0346	/* Clear wait for message state */
#define	AWAITM	0350	/* Wait Message		*/

#define	ADRMSK		0xff03	/* network msg adr mask */
#define	NSCADATA	01	/* associated data bit */
#define	NSCBURST	02	/* burst mode */
#define	NSCEXCPT	04	/* exception message */
#define	NSCSWAB		0	/* assembly mode to swap bytes */


/* Device Block Flags */

#define	DB_OPEN	0001	/* device (half) open for business	*/
#define	DB_IM	0002	/* waiting for an AIM to free db_imsg	*/
#define	DB_IO	0004	/* I/O onto the network is active - IS RESERVED */
#define	DB_OFF	0010	/* driver is off-line, reject all I/O attempts */
#define	DB_W	0020	/* someone wants the A410		*/
#define	DB_OPN2	0040	/* device completely open */

/* Connection Table Slot Flags		*/

#define	CT_MSG		0001	/* message in the adapter is meant for me */
#define	CT_CTRL		0002	/* control mode */
#define	CT_SU		0004	/* this user is the SUPER-USER */
#define	CT_ADATA	0010	/* input operation involves associated data */


/*    NSCIOCTL commands */

#define	NSCIOCTRL	(('n'<<8)|1)
#define	NSCIODATA	(('n'<<8)|2)
#define	NSCIOVCHAN	(('n'<<8)|3)
#define	NSCIOBYE	(('n'<<8)|4)
#define	NSCIOASMB	(('n'<<8)|5)
#define	NSCIOFCODE	(('n'<<8)|6)
#define	NSCIOOFFLINE	(('n'<<8)|7)
#define	NSCIOONLINE	(('n'<<8)|8)
#define	NSCIOCANCEL	(('n'<<8)|9)
#define	NSCIOADDR	(('n'<<8)|10)
#define	NSCIOESTAT	(('n'<<8)|11)
#define	NSCIOSTAT	(('n'<<8)|12)
#define	NSCIOGETP	(('n'<<8)|13)
#define	NSCIOGSTATS	(('n'<<8)|14)
#define	NSCIOCLSTATS	(('n'<<8)|15)

/*
 *	DATAM - Data mode parameter structure
 */

struct datam {
	short i_mbytes;		/* number of bytes into message */
	short o_mbytes;		/* number of bytes out of message */
};

/*
 *	CTRLM - control mode parameter structure
 */

struct ctrlm {
	struct	nmsg	*i_mptr;	/* ptr to input message buf in user space */
	struct	nsctrl *o_mptr;		/* ptr to output message buf in user space */
};

/*
 * 	NSCASMB - input/output assembly mode structure
 */

struct	nscasmb {
	unsigned i_msg : 2;	/* input message assembly mode */
	unsigned i_data : 2;	/* input data assembly mode */
	unsigned o_msg : 2;	/* output message assembly mode */
	unsigned o_data : 2;	/* output data assembly mode */
};


/*
 *	NSCTRL - structure to load destination parameters and data into
 *	the driver during control mode transfers
 */

struct nsctrl {
	short	cn_flags;	/* associated data/exception message/burst mode */
	short	cn_tonad;	/* where is this transmission going */
	char	cn_fnc;		/* adapter function code */
	char	cn_opcode;	/* operation code */
	char	cn_data[MSGDTA];	/* rest of the data */
};


/*
 * NSCFCODE - structure used to issue an arbitrary function code
 *	to the adapter
 */

struct nscfcode {
	char	*f_base;	/* buffer pointer */
	short	f_cnt;		/* number of bytes to transfer */
	short	f_fcode;	/* function code to issue */
};

/*
 *    NSCGETTY - structure to return driver configuration parameters
 */

struct nscgetty{
	short	g_flags;	/* connection flags	*/
	char	g_open;	/* open mode	*/
	char	g_vchan;	/* virtual channel of this connection	*/
	short	g_tonad;	/* to network address	*/
	char	g_acode;	/* access code	*/
	char	g_tmsk;		/* trunk mask	*/
	struct	nscasmb	g_asmb;	/* assembly/disassembly modes	*/
};

#if	NSCSTATS == 1

#define	HISTOSZ		16		/* range is 0 -> 2**HISTOSZ ticks */
#define	TOUTSZ		8		/* number of driver states */

typedef unsigned short	histo_t;

struct histo
{
	time_t		htime;		/* remembers lbolt */
	histo_t		histo[HISTOSZ];	/* delay histogram exp2 */
};

struct stats
{
	struct histo	rdhist;		/* read delay histogram */
	struct histo	wthist;		/* write delay histogram */
	histo_t		touts[TOUTSZ];	/* timeouts per state */
};
#endif
