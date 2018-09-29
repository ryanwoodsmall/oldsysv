/* @(#)indir16.h	1.1 */
/* This structure defines an indirect block on a 16 bit PDP 11 */
	struct ind16 {
	short b16_num;
	short b16_blk[255];
	};
