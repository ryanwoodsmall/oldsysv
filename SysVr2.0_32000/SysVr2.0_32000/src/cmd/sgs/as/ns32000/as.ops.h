/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*
 *		  special control symbol syntax:
 *					 '%' [ 0-4 ] {sym}
 *
 *					 ex: 		%2a - operand 2 specifies gen:addr field
 *								%c  - operand 0 (default) specifies cfglist field
 *
 *		  format control symbols:
 *					 0			litteral bit 0
 *					 1			litteral bit 1
 *					 %a		gen:addr field, default operand 0
 *					 %c		cfglist field, default operand 0
 *					 %d		disp: pc relative, default operand $
 *					 %f		float size field
 *					 %h		condition code field
 *					 %i		integer size field
 *					 %j		disp: constant for block instructions
 *					 %k		disp: constant, default operand $
 *					 %l		reglist field, default operand 0
 *					 %m		mmureg field, default operand 0
 *					 %n		offset/length field, default operand $
 *					 %o		options, default operand 0
 *					 %p		procreg field, default operand 0
 *					 %q		quick field, default operand 0
 *					 %r		gen:read field, default operand 0
 *					 %R		gen:read field, byte only, default operand 0
 *					 %s		reg field, default operand 0
 *					 %t		regaddr field, default operand 1
 *					 %w		gen:write field, default operand 1
 *					 %x		gen:rmw field, default operand 1
 *					 ,			begin extension specifications
 */

{"absF",			 "%r%w11010%f10111110", FINSTR, TFLT},
{"absI",			 "%r%w1100%i01001110", BINSTR, TINT},
{"acbI",			 "%x%q10011%i,%d", TINSTR, TINT},
{"addF",			 "%r%x00000%f10111110", FINSTR, TFLT},
{"addI",			 "%r%x0000%i", BINSTR, TINT},
{"addcI",		 "%r%x0100%i", BINSTR, TINT},
{"addpI",		 "%r%x1111%i01001110", BINSTR, TINT},
{"addqI",		 "%x%q00011%i", BINSTR, TINT},
{"addr",			 "%a%w100111", BINSTR, TINT},
{"adjspI",		 "%r101011111%i", UINSTR, TINT},
{"andI",			 "%r%x1010%i", BINSTR, TINT},
{"ashI",			 "%R%x0001%i01001110", BINSTR, TINT},
{"bC",			 "%h1010,%d", UINSTR, TINT},
{"bicI",			 "%r%x0010%i", BINSTR, TINT},
{"bicpsrbB",	 "%r00101111100", UINSTR, TINT},
{"bicpsrwW",	 "%r00101111101", UINSTR, TINT},
{"bispsrbB",	 "%r01101111100", UINSTR, TINT},
{"bispsrwW",	 "%r01101111101", UINSTR, TINT},
{"bpt",			 "11110010", NINSTR, TINT},
{"br",			 "11101010,%d", UINSTR, TINT},
{"bsr",			 "00000010,%d", UINSTR, TINT},
{"caseI",		 "%r111011111%i", UINSTR, TINT},
{"cbitI",		 "%r%t0010%i01001110", BINSTR, TINT},
{"cbitiI",		 "%r%t0011%i01001110", BINSTR, TINT},
{"checkI",		 "%1a%2r%s0%i11101110", TINSTR, TINT},
{"cmpF",			 "%r%1r00100%f10111110", FINSTR, TFLT},
{"cmpI",			 "%r%1r0001%i", BINSTR, TINT},
{"cmpmI",		 "%a%1a0001%i11001110,%j", TINSTR, TINT},
{"cmpqI",		 "%1r%q00111%i", BINSTR, TINT},
{"cmpsI",		 "00000%o000001%i00001110", SINSTR, TINT},
{"cmpst",		 "00000%o1000010000001110", SINSTR, TINT},
{"comI",			 "%r%w1101%i01001110", BINSTR, TINT},
{"cvtp",			 "%1a%2w%s01101101110", TINSTR, TINT},
{"cxp",			 "00100010,%k", UINSTR, TINT},
{"cxpd",			 "%a00001111111", UINSTR, TINT},
{"deiI",			 "%r%x1011%i11001110", BINSTR, TINT},
{"dia",			 "11000010", NINSTR, TINT},
{"divF",			 "%r%x10000%f10111110", FINSTR, TFLT},
{"divI",			 "%r%x1111%i11001110", BINSTR, TINT},
{"enter",		 "10000010,%l%k", EINSTR, TINT},
{"exit",			 "10010010,%l", RINSTR, TINT},
{"extI",			 "%t%2w%s0%i00101110,%k", QINSTR, TINT},
{"extsI",		 "%0t%w0011%i11001110,%n", QINSTR, TINT},
{"ffsI",			 "%r%x0001%i01101110", BINSTR, TINT},
{"flag",			 "11010010", NINSTR, TINT},
{"floorFI",		 "%r%w111%f%i00111110", XINSTR, TFLT},
{"ibitI",		 "%r%t1110%i01001110", BINSTR, TINT},
{"indexI",		 "%1r%2r%s1%i00101110", TINSTR, TINT},
{"insI",			 "%1r%2t%s0%i10101110,%k", QINSTR, TINT},
{"inssI",		 "%r%t0010%i11001110,%n", QINSTR, TINT},
{"jsr",			 "%a11001111111", UINSTR, TINT},
{"jump",			 "%a01001111111", UINSTR, TINT},
{"lfsrD",		 "%r0000000111100111110", UINSTR, TINT},
{"lmrD",			 "%r%m000101100011110", MINSTR, TINT},
{"lprI",			 "%r%p11011%i", PINSTR, TINT},
{"lshI",			 "%R%x0101%i01001110", BINSTR, TINT},
{"lxpd",			 "%a%w100111", BINSTR, TINT},
{"meiI",			 "%r%x1001%i11001110", BINSTR, TINT},
{"modI",			 "%r%x1110%i11001110", BINSTR, TINT},
{"movF",			 "%r%w00010%f10111110", FINSTR, TFLT},
{"movI",			 "%r%w0101%i", BINSTR, TINT},
{"movIF",		 "%r%w000%f%i00111110", XINSTR, TINT},
{"movfl",		 "%r%w01101100111110", FINSTR, TINT},
{"movlf",		 "%r%w01011000111110", FINSTR, TINT},
{"movmI",		 "%a%1a0000%i11001110,%j", TINSTR, TINT},
{"movqI",		 "%w%q10111%i", BINSTR, TINT},
{"movsI",		 "00000%o000000%i00001110", SINSTR, TINT},
{"movst",		 "00000%o1000000000001110", SINSTR, TINT},
{"movsuI",		 "%a%1a0011%i10101110", BINSTR, TINT},
{"movusI",		 "%a%1a0111%i10101110", BINSTR, TINT},
{"movxbdB",		 "%r%w01110011001110", BINSTR, TINT},
{"movxwdW",		 "%r%w01110111001110", BINSTR, TINT},
{"movxbwB",		 "%r%w01000011001110", BINSTR, TINT},
{"movzbdB",		 "%r%w01100011001110", BINSTR, TINT},
{"movzwdW",		 "%r%w01100111001110", BINSTR, TINT},
{"movzbwB",		 "%r%w01010011001110", BINSTR, TINT},
{"mulF",			 "%r%x11000%f10111110", FINSTR, TFLT},
{"mulI",			 "%r%x1000%i11001110", BINSTR, TINT},
{"negF",			 "%r%w01010%f10111110", FINSTR, TFLT},
{"negI",			 "%r%w1000%i01001110", BINSTR, TINT},
{"nop",			 "10100010", NINSTR, TINT},
{"notI",			 "%r%w1001%i01001110", BINSTR, TINT},
{"orI",			 "%r%x0110%i", BINSTR, TINT},
{"quoI",			 "%r%x1100%i11001110", BINSTR, TINT},
{"rdval",		 "%a0000000001100011110", UINSTR, TINT},
{"remI",			 "%r%x1101%i11001110", BINSTR, TINT},
{"restore",		 "01110010,%l", RINSTR, TINT},
{"ret",			 "00010010,%k", UINSTR, TINT},
{"reti",			 "01010010", NINSTR, TINT},
{"rett",			 "01000010,%k", UINSTR, TINT},
{"rotI",			 "%R%x0000%i01001110", BINSTR, TINT},
{"roundFI",		 "%r%w100%f%i00111110", XINSTR, TFLT},
{"rxp",			 "00110010,%k", UINSTR, TINT},
{"sCI",			 "%0w%h01111%i", UINSTR, TINT},
{"save",			 "01100010,%l", RINSTR, TINT},
{"sbitI",		 "%r%t0110%i01001110", BINSTR, TINT},
{"sbitiI",		 "%r%t0111%i01001110", BINSTR, TINT},
{"setcfg",		 "00000%c000101100001110", CINSTR, TINT},
{"sfsr",			 "00000%0w11011100111110", UINSTR, TINT},
{"skpsI",		 "00000%o000011%i00001110", SINSTR, TINT},
{"skpst",		 "00000%o1000110000001110", SINSTR, TINT},
{"smr",			 "%0w%m000111100011110", MINSTR, TINT},
{"sprI",			 "%0w%p01011%i", PINSTR, TINT},
{"subF",			 "%r%x01000%f10111110", FINSTR, TFLT},
{"subI",			 "%r%x1000%i", BINSTR, TINT},
{"subcI",		 "%r%x1100%i", BINSTR, TINT},
{"subpI",		 "%r%x1011%i01001110", BINSTR, TINT},
{"svc",			 "11100010", NINSTR, TINT},
{"tbitI",		 "%r%t1101%i", BINSTR, TINT},
{"truncFI",		 "%r%w101%f%i00111110", XINSTR, TFLT},
{"wait",			 "10110010", NINSTR, TINT},
{"wrval",		 "%a0000000011100011110", UINSTR, TINT},
{"xorI",			 "%r%x1110%i", BINSTR, TINT},
{" ", 0}
