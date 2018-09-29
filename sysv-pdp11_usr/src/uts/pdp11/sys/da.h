/* @(#)da.h	1.1 */
struct da {
	struct buf	*da_bp;
	int	da_cnt, da_off;
	int	da_cs, da_db;
	struct proc	*da_proc;
	int	da_flags;
};
extern struct da da_da[];
