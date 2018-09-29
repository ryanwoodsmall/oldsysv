/* @(#)du.h	1.1 */
struct du {
	char	*du_buf;
	char	*du_bufp;
	int	du_nxmit;
	char	du_state;
	char	du_dummy;
	struct proc	*du_proc;
	char	*du_bufb;
	char	*du_bufe;
	int	du_nleft;
};
extern struct du du_du[];
