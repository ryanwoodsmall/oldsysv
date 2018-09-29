/* @(#)indir32.h	1.1 */
/* This structure defines an indirect block on a 32 bit VAX */
	struct ind32 {
	short b32_num;
	short fill1;
	short b32_blk[254];
	};
