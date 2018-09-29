/*	x25user.h 1.3 of 3/31/82
	@(#)x25user.h	1.3
 */

#include "sys/types.h"
#include "sys/csi.h"
#include "sys/x25.h"
#include "sys/x25u.h"

/* Defines for application code */

#define X25LRDY(X)	(X->c_rststate == 1)
#define X25L2FULL(X)	(X->c_l_state.l2full == 1)
#define X25L2RDYP(X)	(X->c_primline.l2sync == 1)
#define X25L2ERRP(X)	(X->c_primline.errcode)
#define X25ABNHLTP(X)	((X->c_rststate == 0) && (X->c_primline.errcode != 0))
#define X25NRMHLTP(X)	((X->c_rststate == 0) && (X->c_primline.errcode == 0))
#define X25L2FCP(X)	(X->c_primline.nl2fails)
#define X25LATCHP(X)	(X->c_l_state.devset == 1)
#define X25CRDY(X)	(X->c_t_state.state == 4)
#define	X25CHALT(X)	(X->c_t_state.state == 0)
#define X25FAROPN(X)	(X->c_t_state.faropen == 1)
#define X25ISRESET(X)	(X->c_t_state.isreset == 1)
#define X25MSGCON(X)	(X->c_s_flags.messcont == 1)
#define X25L3XQFUL	(X->c_xmtq == HIXMTQ)
#define X25L3RQFUL	(X->c_recvq == HIRECVQ)
