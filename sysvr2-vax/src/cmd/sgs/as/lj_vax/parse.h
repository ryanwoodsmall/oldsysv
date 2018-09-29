
typedef union 	{
		long	unlong;
		double	unfloat;
		rexpr	unrexpr;
		symbol	*unsymptr;
		instr	*uninsptr;
		addrmode unaddr;
		char	*unstrptr;
	} YYSTYPE;
extern YYSTYPE yylval;
# define ZEROP 257
# define ONEOP 258
# define TWOOP 259
# define THROP 260
# define FOUOP 261
# define FIVOP 262
# define SIXOP 263
# define ISPACE 264
# define IBYTE 265
# define IWORD 266
# define ILONG 267
# define IINT 268
# define IDATA 269
# define IBSS 270
# define IGLOBL 271
# define ISET 272
# define ITEXT 273
# define ICOMM 274
# define IALIGN 275
# define IFLOAT 276
# define IDOUBLE 277
# define IORG 278
# define IFILE 279
# define ILN 280
# define IDEF 281
# define IENDEF 282
# define IVAL 283
# define ISCL 284
# define ITYPE 285
# define ILINE 286
# define ITAG 287
# define ISIZE 288
# define IDIM 289
# define SP 290
# define NL 291
# define SH 292
# define LITOP 293
# define REG 294
# define REGOP 295
# define AND 296
# define SQ 297
# define LP 298
# define RP 299
# define MUL 300
# define PLUS 301
# define COMMA 302
# define MINUS 303
# define ALPH 304
# define DIV 305
# define DIG 306
# define COLON 307
# define SEMI 308
# define LSH 309
# define RSH 310
# define LB 311
# define RB 312
# define XOR 313
# define IOR 314
# define TILDE 315
# define DQ 316
# define INTNUM 317
# define FLTNUM 318
# define IDENT 319
# define ERROR 320
# define STRING 321
