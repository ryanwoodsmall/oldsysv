
typedef union  {
   long unlong;
   double undble;
   INST *uninst;
   SYM  *unsym;
   EXPR unexpr;
   } YYSTYPE;
extern YYSTYPE yylval;
# define _SPACE 257
# define _TEXT 258
# define _DATA 259
# define _BSS 260
# define _GLOBAL 261
# define _SET 262
# define _COMM 263
# define _ALIGN 264
# define _ORG 265
# define _BYTE 266
# define _WORD 267
# define _INT 268
# define _LONG 269
# define _DOUBLE 270
# define _FLOAT 271
# define _FILE 272
# define _ENTER 273
# define _DEF 274
# define _ENDEF 275
# define _DIM 276
# define _LINE 277
# define _SCL 278
# define _SIZE 279
# define _TAG 280
# define _TYPE 281
# define _VAL 282
# define _LN 283
# define LP 284
# define RP 285
# define LB 286
# define RB 287
# define NL 288
# define SEMI 289
# define COMMA 290
# define ERRCH 291
# define COLON 292
# define DOLLAR 293
# define EXTR 294
# define TOS 295
# define PREG 296
# define MREG 297
# define OPT 298
# define SCALE 299
# define IREG 300
# define FREG 301
# define CFG 302
# define BREG 303
# define INUM 304
# define STRING 305
# define HERE 306
# define FNUM 307
# define IDENT 308
# define NINSTR 309
# define UINSTR 310
# define BINSTR 311
# define TINSTR 312
# define QINSTR 313
# define FINSTR 314
# define XINSTR 315
# define MINSTR 316
# define PINSTR 317
# define SINSTR 318
# define CINSTR 319
# define RINSTR 320
# define EINSTR 321
# define OR 322
# define XOR 323
# define AND 324
# define LSH 325
# define RSH 326
# define PLUS 327
# define MINUS 328
# define STAR 329
# define DIV 330
# define MOD 331
# define NOT 332
