/* @(#)mem.h	6.2 */
/*
 * "incore" memory free list for the VAX
 */

#define	NICMEM	100
struct {
	int	m_free;
	int	m_lo;
	int	m_hi;
	int	m_ptr;
	int	m_avail;
	short	m_pnum[NICMEM];
} mem;
