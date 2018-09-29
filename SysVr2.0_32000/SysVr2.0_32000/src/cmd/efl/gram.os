	.file	"gram.c"
.data
.globl	_p
.comm	_p,4
.globl	_procattrs
.comm	_procattrs,4
.globl	_i
.comm	_i,4
.globl	_n
.comm	_n,4
.align	4
_imptype:
	.space	4
.align	4
_ininit:
	.double	0
.globl	_yylval
.comm	_yylval,4
.globl	_yyval
.comm	_yyval,4
.text
.globl	_setyydeb
_setyydeb:
	.enter
.L182:
	exit	[]
	ret	0
	enter	[], 0
.data
.align	4
.globl	_yyexca
_yyexca:
	.double	-1
	.double	0
	.double	0
	.double	1
	.double	1
	.double	175
	.double	5
	.double	175
	.double	18
	.double	175
	.double	21
	.double	175
	.double	25
	.double	175
	.double	52
	.double	175
	.double	53
	.double	175
	.double	55
	.double	175
	.double	57
	.double	175
	.double	58
	.double	175
	.double	59
	.double	175
	.double	60
	.double	175
	.double	61
	.double	175
	.double	62
	.double	175
	.double	63
	.double	175
	.double	64
	.double	175
	.double	65
	.double	175
	.double	66
	.double	175
	.double	67
	.double	175
	.double	68
	.double	175
	.double	69
	.double	175
	.double	70
	.double	175
	.double	77
	.double	175
	.double	78
	.double	175
	.double	79
	.double	175
	.double	80
	.double	175
	.double	83
	.double	175
	.double	-2
	.double	0
	.double	-1
	.double	1
	.double	0
	.double	-1
	.double	-2
	.double	0
	.double	-1
	.double	80
	.double	9
	.double	152
	.double	19
	.double	152
	.double	20
	.double	152
	.double	-2
	.double	178
	.double	-1
	.double	98
	.double	1
	.double	209
	.double	4
	.double	209
	.double	7
	.double	209
	.double	-2
	.double	0
	.double	-1
	.double	165
	.double	19
	.double	152
	.double	20
	.double	152
	.double	-2
	.double	179
	.double	-1
	.double	205
	.double	7
	.double	265
	.double	-2
	.double	0
	.double	-1
	.double	207
	.double	4
	.double	255
	.double	-2
	.double	0
	.double	-1
	.double	208
	.double	7
	.double	255
	.double	-2
	.double	0
	.double	-1
	.double	232
	.double	1
	.double	175
	.double	5
	.double	175
	.double	18
	.double	175
	.double	21
	.double	175
	.double	25
	.double	175
	.double	52
	.double	175
	.double	53
	.double	175
	.double	55
	.double	175
	.double	57
	.double	175
	.double	58
	.double	175
	.double	59
	.double	175
	.double	60
	.double	175
	.double	61
	.double	175
	.double	62
	.double	175
	.double	63
	.double	175
	.double	64
	.double	175
	.double	65
	.double	175
	.double	66
	.double	175
	.double	67
	.double	175
	.double	68
	.double	175
	.double	69
	.double	175
	.double	70
	.double	175
	.double	77
	.double	175
	.double	78
	.double	175
	.double	79
	.double	175
	.double	80
	.double	175
	.double	83
	.double	175
	.double	-2
	.double	0
	.double	-1
	.double	262
	.double	6
	.double	191
	.double	-2
	.double	14
	.double	-1
	.double	351
	.double	14
	.double	0
	.double	-2
	.double	144
.align	4
.globl	_yyact
_yyact:
	.double	225
	.double	247
	.double	408
	.double	431
	.double	412
	.double	124
	.double	409
	.double	337
	.double	223
	.double	197
	.double	157
	.double	20
	.double	258
	.double	224
	.double	185
	.double	156
	.double	19
	.double	6
	.double	140
	.double	3
	.double	20
	.double	155
	.double	61
	.double	139
	.double	286
	.double	19
	.double	59
	.double	314
	.double	5
	.double	131
	.double	297
	.double	152
	.double	212
	.double	44
	.double	182
	.double	198
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	21
	.double	22
	.double	16
	.double	191
	.double	14
	.double	135
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	10
	.double	12
	.double	11
	.double	72
	.double	227
	.double	121
	.double	77
	.double	64
	.double	17
	.double	20
	.double	446
	.double	66
	.double	60
	.double	413
	.double	19
	.double	179
	.double	189
	.double	425
	.double	58
	.double	195
	.double	196
	.double	338
	.double	42
	.double	149
	.double	43
	.double	141
	.double	65
	.double	80
	.double	73
	.double	109
	.double	129
	.double	78
	.double	87
	.double	71
	.double	199
	.double	400
	.double	399
	.double	170
	.double	200
	.double	172
	.double	178
	.double	167
	.double	175
	.double	176
	.double	177
	.double	154
	.double	211
	.double	233
	.double	305
	.double	120
	.double	315
	.double	203
	.double	325
	.double	218
	.double	243
	.double	326
	.double	137
	.double	62
	.double	220
	.double	218
	.double	68
	.double	447
	.double	148
	.double	147
	.double	356
	.double	74
	.double	209
	.double	222
	.double	242
	.double	199
	.double	123
	.double	204
	.double	256
	.double	200
	.double	201
	.double	202
	.double	213
	.double	229
	.double	216
	.double	215
	.double	214
	.double	217
	.double	213
	.double	68
	.double	216
	.double	215
	.double	214
	.double	217
	.double	134
	.double	62
	.double	73
	.double	169
	.double	245
	.double	165
	.double	20
	.double	164
	.double	80
	.double	166
	.double	109
	.double	19
	.double	78
	.double	87
	.double	278
	.double	76
	.double	132
	.double	260
	.double	133
	.double	273
	.double	161
	.double	201
	.double	202
	.double	144
	.double	148
	.double	147
	.double	163
	.double	382
	.double	267
	.double	250
	.double	263
	.double	160
	.double	265
	.double	271
	.double	272
	.double	273
	.double	148
	.double	147
	.double	259
	.double	433
	.double	280
	.double	394
	.double	282
	.double	148
	.double	147
	.double	228
	.double	268
	.double	269
	.double	270
	.double	239
	.double	279
	.double	69
	.double	287
	.double	272
	.double	273
	.double	289
	.double	235
	.double	250
	.double	127
	.double	295
	.double	296
	.double	298
	.double	298
	.double	58
	.double	303
	.double	240
	.double	291
	.double	292
	.double	238
	.double	262
	.double	146
	.double	128
	.double	18
	.double	65
	.double	252
	.double	251
	.double	344
	.double	343
	.double	449
	.double	307
	.double	148
	.double	147
	.double	55
	.double	310
	.double	460
	.double	429
	.double	417
	.double	253
	.double	254
	.double	434
	.double	264
	.double	283
	.double	300
	.double	293
	.double	20
	.double	329
	.double	20
	.double	304
	.double	173
	.double	19
	.double	20
	.double	19
	.double	319
	.double	322
	.double	321
	.double	19
	.double	171
	.double	372
	.double	428
	.double	316
	.double	330
	.double	429
	.double	317
	.double	327
	.double	323
	.double	281
	.double	141
	.double	392
	.double	377
	.double	7
	.double	393
	.double	378
	.double	367
	.double	342
	.double	248
	.double	348
	.double	349
	.double	350
	.double	351
	.double	352
	.double	353
	.double	354
	.double	355
	.double	335
	.double	364
	.double	143
	.double	154
	.double	365
	.double	357
	.double	360
	.double	234
	.double	339
	.double	363
	.double	462
	.double	340
	.double	333
	.double	463
	.double	287
	.double	334
	.double	277
	.double	275
	.double	276
	.double	159
	.double	274
	.double	271
	.double	272
	.double	273
	.double	274
	.double	271
	.double	272
	.double	273
	.double	373
	.double	376
	.double	255
	.double	158
	.double	375
	.double	159
	.double	380
	.double	145
	.double	302
	.double	371
	.double	366
	.double	358
	.double	361
	.double	276
	.double	301
	.double	274
	.double	271
	.double	272
	.double	273
	.double	379
	.double	418
	.double	389
	.double	423
	.double	236
	.double	396
	.double	390
	.double	420
	.double	391
	.double	237
	.double	277
	.double	275
	.double	276
	.double	414
	.double	274
	.double	271
	.double	272
	.double	273
	.double	398
	.double	134
	.double	416
	.double	331
	.double	397
	.double	415
	.double	332
	.double	451
	.double	395
	.double	277
	.double	275
	.double	276
	.double	381
	.double	274
	.double	271
	.double	272
	.double	273
	.double	369
	.double	259
	.double	312
	.double	277
	.double	275
	.double	276
	.double	410
	.double	274
	.double	271
	.double	272
	.double	273
	.double	279
	.double	445
	.double	407
	.double	279
	.double	436
	.double	465
	.double	427
	.double	250
	.double	150
	.double	419
	.double	250
	.double	277
	.double	275
	.double	276
	.double	421
	.double	274
	.double	271
	.double	272
	.double	273
	.double	125
	.double	406
	.double	20
	.double	125
	.double	403
	.double	306
	.double	285
	.double	19
	.double	284
	.double	424
	.double	422
	.double	308
	.double	74
	.double	122
	.double	277
	.double	275
	.double	276
	.double	249
	.double	274
	.double	271
	.double	272
	.double	273
	.double	219
	.double	210
	.double	207
	.double	206
	.double	435
	.double	48
	.double	205
	.double	439
	.double	437
	.double	50
	.double	438
	.double	52
	.double	53
	.double	162
	.double	442
	.double	126
	.double	410
	.double	125
	.double	441
	.double	440
	.double	444
	.double	388
	.double	443
	.double	410
	.double	448
	.double	450
	.double	387
	.double	386
	.double	385
	.double	383
	.double	347
	.double	453
	.double	456
	.double	345
	.double	457
	.double	458
	.double	150
	.double	49
	.double	414
	.double	459
	.double	51
	.double	411
	.double	142
	.double	9
	.double	461
	.double	454
	.double	188
	.double	370
	.double	186
	.double	208
	.double	464
	.double	187
	.double	107
	.double	192
	.double	68
	.double	99
	.double	44
	.double	97
	.double	180
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	96
	.double	426
	.double	95
	.double	266
	.double	174
	.double	88
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	455
	.double	86
	.double	84
	.double	341
	.double	168
	.double	452
	.double	277
	.double	275
	.double	276
	.double	116
	.double	274
	.double	271
	.double	272
	.double	273
	.double	277
	.double	275
	.double	276
	.double	261
	.double	274
	.double	271
	.double	272
	.double	273
	.double	42
	.double	82
	.double	43
	.double	193
	.double	194
	.double	150
	.double	113
	.double	114
	.double	111
	.double	112
	.double	195
	.double	196
	.double	384
	.double	230
	.double	328
	.double	188
	.double	221
	.double	186
	.double	226
	.double	336
	.double	187
	.double	153
	.double	192
	.double	68
	.double	241
	.double	44
	.double	70
	.double	180
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	151
	.double	63
	.double	136
	.double	54
	.double	8
	.double	75
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	430
	.double	13
	.double	277
	.double	275
	.double	276
	.double	404
	.double	274
	.double	271
	.double	272
	.double	273
	.double	232
	.double	277
	.double	275
	.double	276
	.double	231
	.double	274
	.double	271
	.double	272
	.double	273
	.double	4
	.double	2
	.double	102
	.double	42
	.double	432
	.double	43
	.double	193
	.double	194
	.double	110
	.double	113
	.double	114
	.double	111
	.double	112
	.double	195
	.double	196
	.double	150
	.double	181
	.double	29
	.double	23
	.double	288
	.double	25
	.double	294
	.double	299
	.double	290
	.double	346
	.double	188
	.double	85
	.double	186
	.double	15
	.double	246
	.double	187
	.double	67
	.double	192
	.double	68
	.double	184
	.double	44
	.double	183
	.double	180
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	28
	.double	56
	.double	309
	.double	24
	.double	318
	.double	138
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	401
	.double	130
	.double	257
	.double	320
	.double	1
	.double	0
	.double	277
	.double	275
	.double	276
	.double	0
	.double	274
	.double	271
	.double	272
	.double	273
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	42
	.double	0
	.double	43
	.double	193
	.double	194
	.double	0
	.double	113
	.double	114
	.double	111
	.double	112
	.double	195
	.double	196
	.double	0
	.double	150
	.double	244
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	188
	.double	0
	.double	186
	.double	0
	.double	0
	.double	187
	.double	0
	.double	192
	.double	68
	.double	190
	.double	44
	.double	0
	.double	180
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	374
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	277
	.double	275
	.double	276
	.double	0
	.double	274
	.double	271
	.double	272
	.double	273
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	42
	.double	0
	.double	43
	.double	193
	.double	194
	.double	150
	.double	113
	.double	114
	.double	111
	.double	112
	.double	195
	.double	196
	.double	0
	.double	0
	.double	0
	.double	188
	.double	0
	.double	186
	.double	0
	.double	0
	.double	187
	.double	0
	.double	192
	.double	68
	.double	190
	.double	44
	.double	0
	.double	180
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	368
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	277
	.double	275
	.double	276
	.double	0
	.double	274
	.double	271
	.double	272
	.double	273
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	42
	.double	0
	.double	43
	.double	193
	.double	194
	.double	0
	.double	113
	.double	114
	.double	111
	.double	112
	.double	195
	.double	196
	.double	150
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	188
	.double	0
	.double	186
	.double	0
	.double	0
	.double	187
	.double	0
	.double	192
	.double	68
	.double	190
	.double	44
	.double	0
	.double	180
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	38
	.double	39
	.double	40
	.double	41
	.double	362
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	313
	.double	0
	.double	277
	.double	275
	.double	276
	.double	311
	.double	274
	.double	271
	.double	272
	.double	273
	.double	0
	.double	277
	.double	275
	.double	276
	.double	0
	.double	274
	.double	271
	.double	272
	.double	273
	.double	0
	.double	0
	.double	0
	.double	42
	.double	0
	.double	43
	.double	193
	.double	194
	.double	0
	.double	113
	.double	114
	.double	111
	.double	112
	.double	195
	.double	196
	.double	0
	.double	150
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	188
	.double	0
	.double	186
	.double	0
	.double	0
	.double	187
	.double	0
	.double	192
	.double	68
	.double	190
	.double	44
	.double	0
	.double	180
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	38
	.double	39
	.double	40
	.double	41
	.double	359
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	42
	.double	0
	.double	43
	.double	193
	.double	194
	.double	0
	.double	113
	.double	114
	.double	111
	.double	112
	.double	195
	.double	196
	.double	44
	.double	0
	.double	0
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	0
	.double	0
	.double	0
	.double	190
	.double	14
	.double	0
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	10
	.double	12
	.double	11
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	83
	.double	57
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	42
	.double	0
	.double	43
	.double	0
	.double	0
	.double	0
	.double	79
	.double	0
	.double	0
	.double	91
	.double	62
	.double	0
	.double	44
	.double	92
	.double	0
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	21
	.double	22
	.double	0
	.double	0
	.double	0
	.double	0
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	0
	.double	0
	.double	90
	.double	104
	.double	0
	.double	106
	.double	190
	.double	105
	.double	116
	.double	115
	.double	108
	.double	94
	.double	93
	.double	101
	.double	100
	.double	118
	.double	119
	.double	117
	.double	98
	.double	103
	.double	81
	.double	0
	.double	42
	.double	0
	.double	43
	.double	0
	.double	0
	.double	113
	.double	114
	.double	111
	.double	112
	.double	0
	.double	62
	.double	89
	.double	44
	.double	0
	.double	0
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	10
	.double	12
	.double	11
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	44
	.double	0
	.double	0
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	42
	.double	0
	.double	43
	.double	0
	.double	0
	.double	190
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	10
	.double	12
	.double	11
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	150
	.double	405
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	42
	.double	0
	.double	43
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	44
	.double	0
	.double	0
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	324
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	0
	.double	0
	.double	0
	.double	150
	.double	402
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	42
	.double	44
	.double	43
	.double	0
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	57
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	62
	.double	42
	.double	44
	.double	43
	.double	0
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	0
	.double	0
	.double	0
	.double	150
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	42
	.double	44
	.double	43
	.double	0
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	44
	.double	0
	.double	0
	.double	37
	.double	34
	.double	36
	.double	32
	.double	33
	.double	35
	.double	0
	.double	0
	.double	0
	.double	42
	.double	0
	.double	43
	.double	38
	.double	39
	.double	40
	.double	41
	.double	26
	.double	45
	.double	27
	.double	30
	.double	31
	.double	46
	.double	47
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	42
	.double	0
	.double	43
.align	4
.globl	_yypact
_yypact:
	.double	10
	.double	-1000
	.double	450
	.double	450
	.double	453
	.double	450
	.double	450
	.double	-1000
	.double	-1000
	.double	1035
	.double	98
	.double	-1000
	.double	121
	.double	141
	.double	-1000
	.double	1034
	.double	-1000
	.double	-1000
	.double	-1000
	.double	402
	.double	402
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	428
	.double	426
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	202
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	17
	.double	-1000
	.double	-1000
	.double	142
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	93
	.double	-1000
	.double	-1000
	.double	1095
	.double	-1000
	.double	275
	.double	-1000
	.double	402
	.double	-1000
	.double	308
	.double	-1000
	.double	208
	.double	-1000
	.double	378
	.double	-1000
	.double	1377
	.double	304
	.double	292
	.double	-1000
	.double	171
	.double	98
	.double	424
	.double	-1000
	.double	-1000
	.double	164
	.double	98
	.double	-1000
	.double	98
	.double	-1000
	.double	-1000
	.double	1034
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	245
	.double	-1000
	.double	237
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	754
	.double	-1000
	.double	107
	.double	109
	.double	417
	.double	-1000
	.double	414
	.double	-1000
	.double	413
	.double	-1000
	.double	412
	.double	-1000
	.double	80
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	411
	.double	754
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	754
	.double	520
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	69
	.double	280
	.double	194
	.double	330
	.double	-1000
	.double	-1000
	.double	275
	.double	1292
	.double	95
	.double	187
	.double	98
	.double	-1000
	.double	106
	.double	90
	.double	-1000
	.double	681
	.double	264
	.double	-1000
	.double	406
	.double	1377
	.double	-1000
	.double	-1000
	.double	402
	.double	401
	.double	98
	.double	98
	.double	306
	.double	127
	.double	754
	.double	102
	.double	-1000
	.double	102
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	229
	.double	-1000
	.double	754
	.double	-1000
	.double	-1000
	.double	-1000
	.double	355
	.double	152
	.double	-1000
	.double	-1000
	.double	1341
	.double	-1000
	.double	-1000
	.double	-1000
	.double	754
	.double	98
	.double	754
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-3
	.double	397
	.double	395
	.double	-1000
	.double	-1000
	.double	-1000
	.double	600
	.double	-1000
	.double	-1000
	.double	754
	.double	-1000
	.double	72
	.double	107
	.double	754
	.double	754
	.double	754
	.double	754
	.double	315
	.double	754
	.double	86
	.double	82
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	396
	.double	520
	.double	884
	.double	360
	.double	-1000
	.double	875
	.double	-1000
	.double	85
	.double	85
	.double	341
	.double	1134
	.double	-1000
	.double	975
	.double	-1000
	.double	-1000
	.double	89
	.double	1095
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	754
	.double	378
	.double	-1000
	.double	-1000
	.double	344
	.double	288
	.double	-1000
	.double	1377
	.double	4
	.double	-1000
	.double	-1000
	.double	-1000
	.double	102
	.double	102
	.double	-1000
	.double	-1000
	.double	284
	.double	-1000
	.double	-1000
	.double	355
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	216
	.double	355
	.double	446
	.double	443
	.double	-1000
	.double	754
	.double	754
	.double	754
	.double	754
	.double	754
	.double	754
	.double	754
	.double	754
	.double	-1000
	.double	149
	.double	102
	.double	290
	.double	103
	.double	915
	.double	834
	.double	-1000
	.double	355
	.double	754
	.double	355
	.double	277
	.double	-1000
	.double	-1000
	.double	600
	.double	262
	.double	355
	.double	799
	.double	358
	.double	355
	.double	-1000
	.double	247
	.double	-1000
	.double	-1000
	.double	726
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	754
	.double	261
	.double	-1000
	.double	-1000
	.double	-1000
	.double	520
	.double	353
	.double	-1000
	.double	158
	.double	-1000
	.double	442
	.double	-1000
	.double	-1000
	.double	441
	.double	440
	.double	439
	.double	434
	.double	-1000
	.double	-1000
	.double	-1000
	.double	754
	.double	355
	.double	-1000
	.double	-1000
	.double	754
	.double	-1000
	.double	754
	.double	-1000
	.double	260
	.double	-1000
	.double	173
	.double	-1000
	.double	95
	.double	326
	.double	-1000
	.double	-1000
	.double	754
	.double	41
	.double	-1000
	.double	38
	.double	184
	.double	149
	.double	149
	.double	165
	.double	309
	.double	290
	.double	355
	.double	355
	.double	-1000
	.double	645
	.double	1243
	.double	392
	.double	570
	.double	1194
	.double	389
	.double	355
	.double	-1000
	.double	72
	.double	-1000
	.double	447
	.double	-1000
	.double	-1000
	.double	-1000
	.double	346
	.double	-1000
	.double	226
	.double	-1000
	.double	-1000
	.double	327
	.double	-1000
	.double	520
	.double	-1000
	.double	355
	.double	-1000
	.double	-1000
	.double	1134
	.double	324
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	355
	.double	355
	.double	355
	.double	-1000
	.double	4
	.double	0
	.double	-1000
	.double	-1000
	.double	-1000
	.double	355
	.double	-1000
	.double	376
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	251
	.double	-1000
	.double	561
	.double	-1000
	.double	228
	.double	335
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	754
	.double	-1000
	.double	-1000
	.double	432
	.double	-1000
	.double	-1000
	.double	-1000
	.double	754
	.double	-1000
	.double	447
	.double	-1000
	.double	-1000
	.double	371
	.double	44
	.double	-1000
	.double	217
	.double	447
	.double	348
	.double	-1000
	.double	500
	.double	-1000
	.double	-1000
	.double	492
	.double	-1000
	.double	-1000
	.double	754
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	225
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	286
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	-1000
	.double	754
	.double	374
	.double	-1000
.align	4
.globl	_yypgo
_yypgo:
	.double	0
	.double	653
	.double	18
	.double	652
	.double	17
	.double	47
	.double	219
	.double	651
	.double	12
	.double	22
	.double	27
	.double	650
	.double	23
	.double	637
	.double	636
	.double	34
	.double	92
	.double	456
	.double	67
	.double	635
	.double	108
	.double	64
	.double	634
	.double	8
	.double	13
	.double	633
	.double	26
	.double	71
	.double	21
	.double	15
	.double	632
	.double	29
	.double	0
	.double	74
	.double	14
	.double	75
	.double	624
	.double	622
	.double	70
	.double	619
	.double	617
	.double	616
	.double	614
	.double	612
	.double	611
	.double	35
	.double	24
	.double	72
	.double	30
	.double	610
	.double	609
	.double	2
	.double	6
	.double	4
	.double	3
	.double	608
	.double	606
	.double	605
	.double	604
	.double	45
	.double	596
	.double	32
	.double	592
	.double	590
	.double	9
	.double	589
	.double	419
	.double	588
	.double	583
	.double	28
	.double	579
	.double	570
	.double	557
	.double	556
	.double	555
	.double	554
	.double	5
	.double	553
	.double	186
	.double	552
	.double	1
	.double	544
	.double	66
	.double	542
	.double	31
	.double	539
	.double	537
	.double	7
	.double	62
	.double	10
	.double	534
	.double	532
	.double	63
	.double	531
	.double	530
	.double	65
	.double	519
	.double	513
	.double	500
	.double	499
	.double	498
	.double	497
	.double	484
	.double	483
	.double	482
	.double	481
	.double	480
	.double	479
	.double	471
	.double	469
	.double	466
	.double	463
	.double	461
	.double	377
.align	4
.globl	_yyr1
_yyr1:
	.double	0
	.double	1
	.double	1
	.double	1
	.double	1
	.double	1
	.double	1
	.double	1
	.double	66
	.double	3
	.double	3
	.double	3
	.double	3
	.double	5
	.double	70
	.double	5
	.double	67
	.double	67
	.double	67
	.double	67
	.double	72
	.double	71
	.double	71
	.double	6
	.double	6
	.double	7
	.double	7
	.double	8
	.double	65
	.double	73
	.double	74
	.double	74
	.double	74
	.double	75
	.double	75
	.double	75
	.double	69
	.double	68
	.double	76
	.double	13
	.double	13
	.double	13
	.double	12
	.double	12
	.double	2
	.double	2
	.double	2
	.double	2
	.double	2
	.double	2
	.double	14
	.double	14
	.double	14
	.double	77
	.double	77
	.double	83
	.double	82
	.double	79
	.double	79
	.double	84
	.double	84
	.double	85
	.double	86
	.double	86
	.double	87
	.double	87
	.double	81
	.double	81
	.double	88
	.double	16
	.double	16
	.double	17
	.double	17
	.double	18
	.double	18
	.double	20
	.double	20
	.double	90
	.double	21
	.double	22
	.double	22
	.double	23
	.double	23
	.double	24
	.double	24
	.double	25
	.double	25
	.double	26
	.double	26
	.double	27
	.double	91
	.double	27
	.double	9
	.double	15
	.double	15
	.double	28
	.double	28
	.double	29
	.double	29
	.double	29
	.double	29
	.double	29
	.double	29
	.double	29
	.double	56
	.double	56
	.double	56
	.double	56
	.double	56
	.double	19
	.double	19
	.double	92
	.double	10
	.double	10
	.double	55
	.double	55
	.double	55
	.double	55
	.double	55
	.double	55
	.double	30
	.double	30
	.double	30
	.double	11
	.double	11
	.double	93
	.double	94
	.double	31
	.double	89
	.double	89
	.double	57
	.double	57
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	32
	.double	33
	.double	38
	.double	38
	.double	38
	.double	38
	.double	38
	.double	39
	.double	34
	.double	34
	.double	34
	.double	40
	.double	40
	.double	36
	.double	36
	.double	36
	.double	37
	.double	37
	.double	37
	.double	58
	.double	58
	.double	59
	.double	59
	.double	4
	.double	41
	.double	95
	.double	95
	.double	95
	.double	95
	.double	95
	.double	95
	.double	95
	.double	95
	.double	95
	.double	95
	.double	95
	.double	95
	.double	102
	.double	102
	.double	98
	.double	99
	.double	100
	.double	100
	.double	100
	.double	100
	.double	103
	.double	100
	.double	104
	.double	104
	.double	42
	.double	42
	.double	42
	.double	42
	.double	105
	.double	106
	.double	107
	.double	43
	.double	43
	.double	101
	.double	101
	.double	101
	.double	101
	.double	101
	.double	101
	.double	101
	.double	45
	.double	44
	.double	44
	.double	46
	.double	46
	.double	64
	.double	64
	.double	64
	.double	64
	.double	109
	.double	109
	.double	109
	.double	109
	.double	109
	.double	60
	.double	60
	.double	60
	.double	61
	.double	61
	.double	61
	.double	61
	.double	61
	.double	61
	.double	108
	.double	108
	.double	108
	.double	108
	.double	47
	.double	47
	.double	47
	.double	47
	.double	47
	.double	47
	.double	110
	.double	111
	.double	111
	.double	49
	.double	112
	.double	112
	.double	48
	.double	48
	.double	96
	.double	97
	.double	35
	.double	63
	.double	63
	.double	63
	.double	63
	.double	50
	.double	50
	.double	51
	.double	51
	.double	52
	.double	52
	.double	52
	.double	52
	.double	52
	.double	52
	.double	52
	.double	113
	.double	53
	.double	54
	.double	54
	.double	54
	.double	54
	.double	62
	.double	78
	.double	80
.align	4
.globl	_yyr2
_yyr2:
	.double	0
	.double	1
	.double	5
	.double	5
	.double	9
	.double	5
	.double	5
	.double	3
	.double	3
	.double	5
	.double	5
	.double	5
	.double	5
	.double	1
	.double	1
	.double	7
	.double	3
	.double	5
	.double	9
	.double	11
	.double	3
	.double	3
	.double	5
	.double	3
	.double	3
	.double	3
	.double	7
	.double	3
	.double	5
	.double	3
	.double	0
	.double	4
	.double	6
	.double	3
	.double	7
	.double	7
	.double	3
	.double	3
	.double	1
	.double	2
	.double	4
	.double	7
	.double	2
	.double	2
	.double	5
	.double	9
	.double	5
	.double	9
	.double	5
	.double	5
	.double	2
	.double	4
	.double	7
	.double	2
	.double	6
	.double	1
	.double	9
	.double	2
	.double	6
	.double	3
	.double	8
	.double	3
	.double	2
	.double	6
	.double	3
	.double	7
	.double	2
	.double	6
	.double	7
	.double	7
	.double	7
	.double	2
	.double	5
	.double	5
	.double	5
	.double	1
	.double	2
	.double	1
	.double	9
	.double	2
	.double	7
	.double	3
	.double	7
	.double	2
	.double	3
	.double	1
	.double	2
	.double	2
	.double	7
	.double	5
	.double	1
	.double	11
	.double	3
	.double	2
	.double	5
	.double	2
	.double	5
	.double	3
	.double	5
	.double	3
	.double	9
	.double	9
	.double	3
	.double	3
	.double	3
	.double	3
	.double	3
	.double	3
	.double	3
	.double	11
	.double	11
	.double	1
	.double	1
	.double	2
	.double	3
	.double	3
	.double	3
	.double	3
	.double	5
	.double	3
	.double	3
	.double	9
	.double	5
	.double	3
	.double	3
	.double	1
	.double	1
	.double	13
	.double	2
	.double	2
	.double	3
	.double	3
	.double	3
	.double	2
	.double	3
	.double	5
	.double	2
	.double	2
	.double	3
	.double	7
	.double	7
	.double	7
	.double	5
	.double	5
	.double	7
	.double	7
	.double	7
	.double	5
	.double	7
	.double	7
	.double	2
	.double	3
	.double	3
	.double	2
	.double	5
	.double	7
	.double	9
	.double	7
	.double	3
	.double	5
	.double	7
	.double	7
	.double	7
	.double	7
	.double	9
	.double	9
	.double	9
	.double	9
	.double	9
	.double	9
	.double	2
	.double	7
	.double	2
	.double	2
	.double	5
	.double	1
	.double	7
	.double	5
	.double	3
	.double	5
	.double	6
	.double	11
	.double	5
	.double	3
	.double	3
	.double	3
	.double	3
	.double	3
	.double	1
	.double	3
	.double	1
	.double	1
	.double	7
	.double	5
	.double	9
	.double	7
	.double	1
	.double	10
	.double	3
	.double	7
	.double	17
	.double	9
	.double	9
	.double	7
	.double	9
	.double	1
	.double	3
	.double	3
	.double	11
	.double	3
	.double	5
	.double	2
	.double	5
	.double	7
	.double	7
	.double	9
	.double	7
	.double	3
	.double	7
	.double	2
	.double	5
	.double	3
	.double	3
	.double	5
	.double	3
	.double	3
	.double	5
	.double	5
	.double	7
	.double	7
	.double	3
	.double	3
	.double	3
	.double	3
	.double	3
	.double	3
	.double	3
	.double	3
	.double	3
	.double	9
	.double	9
	.double	9
	.double	3
	.double	9
	.double	7
	.double	13
	.double	11
	.double	17
	.double	15
	.double	5
	.double	5
	.double	9
	.double	4
	.double	5
	.double	9
	.double	1
	.double	2
	.double	5
	.double	1
	.double	13
	.double	3
	.double	3
	.double	3
	.double	3
	.double	2
	.double	1
	.double	3
	.double	7
	.double	3
	.double	7
	.double	5
	.double	3
	.double	5
	.double	9
	.double	7
	.double	1
	.double	9
	.double	3
	.double	9
	.double	13
	.double	7
	.double	7
	.double	1
	.double	1
.align	4
.globl	_yychk
_yychk:
	.double	-1000
	.double	-1
	.double	-65
	.double	-2
	.double	-67
	.double	-69
	.double	-4
	.double	256
	.double	-73
	.double	-17
	.double	49
	.double	51
	.double	50
	.double	-71
	.double	36
	.double	-41
	.double	34
	.double	-18
	.double	-6
	.double	-29
	.double	-89
	.double	32
	.double	33
	.double	-56
	.double	-19
	.double	-55
	.double	42
	.double	44
	.double	-30
	.double	-57
	.double	45
	.double	46
	.double	29
	.double	30
	.double	27
	.double	31
	.double	28
	.double	26
	.double	38
	.double	39
	.double	40
	.double	41
	.double	71
	.double	73
	.double	23
	.double	43
	.double	47
	.double	48
	.double	-66
	.double	1
	.double	-66
	.double	1
	.double	-66
	.double	-66
	.double	-74
	.double	-6
	.double	-25
	.double	5
	.double	-18
	.double	-26
	.double	-27
	.double	-9
	.double	21
	.double	-77
	.double	-82
	.double	-33
	.double	-38
	.double	-39
	.double	21
	.double	-78
	.double	-81
	.double	-16
	.double	-88
	.double	-33
	.double	3
	.double	-72
	.double	21
	.double	-95
	.double	-33
	.double	18
	.double	-38
	.double	70
	.double	-96
	.double	5
	.double	-100
	.double	-42
	.double	-101
	.double	-35
	.double	-102
	.double	83
	.double	52
	.double	21
	.double	25
	.double	62
	.double	61
	.double	-105
	.double	-107
	.double	-108
	.double	68
	.double	-109
	.double	64
	.double	63
	.double	-63
	.double	69
	.double	53
	.double	57
	.double	55
	.double	-110
	.double	60
	.double	-47
	.double	-60
	.double	79
	.double	80
	.double	77
	.double	78
	.double	59
	.double	58
	.double	67
	.double	65
	.double	66
	.double	-20
	.double	-21
	.double	3
	.double	-20
	.double	-76
	.double	3
	.double	3
	.double	3
	.double	16
	.double	72
	.double	-11
	.double	-31
	.double	21
	.double	23
	.double	5
	.double	-5
	.double	-75
	.double	22
	.double	-13
	.double	-12
	.double	-2
	.double	-26
	.double	-17
	.double	7
	.double	-20
	.double	7
	.double	9
	.double	20
	.double	19
	.double	-34
	.double	3
	.double	-79
	.double	-84
	.double	-85
	.double	-15
	.double	-28
	.double	-29
	.double	-89
	.double	7
	.double	7
	.double	7
	.double	-16
	.double	3
	.double	9
	.double	-33
	.double	-38
	.double	-33
	.double	-4
	.double	-98
	.double	-95
	.double	-76
	.double	8
	.double	-76
	.double	8
	.double	-103
	.double	-76
	.double	-76
	.double	-76
	.double	-32
	.double	-33
	.double	25
	.double	-58
	.double	-15
	.double	-36
	.double	-37
	.double	-34
	.double	15
	.double	18
	.double	13
	.double	-35
	.double	256
	.double	-59
	.double	20
	.double	74
	.double	75
	.double	81
	.double	82
	.double	-64
	.double	-45
	.double	21
	.double	25
	.double	61
	.double	62
	.double	3
	.double	21
	.double	3
	.double	3
	.double	3
	.double	-111
	.double	-4
	.double	3
	.double	25
	.double	-61
	.double	55
	.double	59
	.double	58
	.double	57
	.double	60
	.double	32
	.double	3
	.double	-32
	.double	-90
	.double	-32
	.double	-23
	.double	-24
	.double	-32
	.double	16
	.double	-92
	.double	-92
	.double	-76
	.double	-93
	.double	-68
	.double	-70
	.double	37
	.double	7
	.double	9
	.double	1
	.double	6
	.double	-27
	.double	9
	.double	-82
	.double	-83
	.double	21
	.double	23
	.double	4
	.double	-32
	.double	-40
	.double	-80
	.double	7
	.double	3
	.double	-28
	.double	-21
	.double	-88
	.double	-33
	.double	-33
	.double	4
	.double	4
	.double	-7
	.double	-8
	.double	-9
	.double	-32
	.double	-97
	.double	-5
	.double	-76
	.double	8
	.double	-76
	.double	-104
	.double	-32
	.double	-4
	.double	-4
	.double	-4
	.double	15
	.double	16
	.double	17
	.double	14
	.double	11
	.double	12
	.double	10
	.double	9
	.double	-34
	.double	-32
	.double	-33
	.double	-32
	.double	-59
	.double	3
	.double	3
	.double	-46
	.double	-32
	.double	7
	.double	-32
	.double	-44
	.double	-64
	.double	-64
	.double	-45
	.double	-50
	.double	-32
	.double	-32
	.double	-48
	.double	-32
	.double	-49
	.double	-48
	.double	7
	.double	1
	.double	-32
	.double	-61
	.double	25
	.double	1
	.double	-76
	.double	7
	.double	-22
	.double	-23
	.double	4
	.double	4
	.double	8
	.double	-10
	.double	24
	.double	-10
	.double	-31
	.double	-14
	.double	-2
	.double	-3
	.double	-2
	.double	-4
	.double	-69
	.double	256
	.double	22
	.double	25
	.double	-12
	.double	-91
	.double	-32
	.double	-34
	.double	4
	.double	7
	.double	4
	.double	7
	.double	-84
	.double	-86
	.double	-87
	.double	76
	.double	4
	.double	7
	.double	-99
	.double	-76
	.double	8
	.double	7
	.double	1
	.double	-43
	.double	1
	.double	-32
	.double	-32
	.double	-32
	.double	-32
	.double	-32
	.double	-32
	.double	-32
	.double	-32
	.double	20
	.double	-32
	.double	-15
	.double	42
	.double	-32
	.double	-15
	.double	42
	.double	-32
	.double	4
	.double	7
	.double	-46
	.double	7
	.double	4
	.double	4
	.double	-112
	.double	-4
	.double	7
	.double	-76
	.double	4
	.double	-76
	.double	-32
	.double	4
	.double	7
	.double	-24
	.double	-32
	.double	4
	.double	16
	.double	1
	.double	-94
	.double	1
	.double	1
	.double	1
	.double	1
	.double	-32
	.double	-32
	.double	-32
	.double	4
	.double	7
	.double	15
	.double	-8
	.double	6
	.double	-76
	.double	-32
	.double	54
	.double	56
	.double	4
	.double	4
	.double	4
	.double	4
	.double	4
	.double	4
	.double	-64
	.double	-51
	.double	-52
	.double	-32
	.double	8
	.double	-53
	.double	-47
	.double	5
	.double	4
	.double	1
	.double	7
	.double	1
	.double	-76
	.double	7
	.double	-23
	.double	-2
	.double	6
	.double	-87
	.double	76
	.double	-106
	.double	3
	.double	4
	.double	7
	.double	8
	.double	-54
	.double	-62
	.double	-78
	.double	8
	.double	-53
	.double	-113
	.double	-76
	.double	-76
	.double	-32
	.double	1
	.double	-76
	.double	-32
	.double	-52
	.double	-54
	.double	3
	.double	25
	.double	76
	.double	-54
	.double	8
	.double	-51
	.double	4
	.double	1
	.double	-76
	.double	-4
	.double	4
	.double	-32
	.double	-80
	.double	-80
	.double	-54
	.double	6
	.double	-76
	.double	4
	.double	7
	.double	-32
	.double	4
.align	4
.globl	_yydef
_yydef:
	.double	-2
	.double	-2
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	7
	.double	30
	.double	85
	.double	0
	.double	282
	.double	0
	.double	16
	.double	36
	.double	188
	.double	29
	.double	71
	.double	21
	.double	75
	.double	75
	.double	23
	.double	24
	.double	97
	.double	38
	.double	99
	.double	0
	.double	0
	.double	102
	.double	103
	.double	128
	.double	129
	.double	104
	.double	105
	.double	106
	.double	107
	.double	108
	.double	0
	.double	114
	.double	115
	.double	116
	.double	117
	.double	0
	.double	119
	.double	120
	.double	0
	.double	130
	.double	131
	.double	2
	.double	8
	.double	3
	.double	13
	.double	5
	.double	6
	.double	28
	.double	22
	.double	44
	.double	0
	.double	72
	.double	86
	.double	87
	.double	75
	.double	92
	.double	46
	.double	53
	.double	0
	.double	152
	.double	153
	.double	158
	.double	0
	.double	48
	.double	49
	.double	66
	.double	0
	.double	0
	.double	17
	.double	20
	.double	174
	.double	0
	.double	0
	.double	-2
	.double	0
	.double	175
	.double	190
	.double	188
	.double	183
	.double	184
	.double	185
	.double	186
	.double	187
	.double	38
	.double	158
	.double	38
	.double	0
	.double	196
	.double	38
	.double	38
	.double	38
	.double	-2
	.double	211
	.double	0
	.double	0
	.double	0
	.double	189
	.double	0
	.double	206
	.double	0
	.double	175
	.double	0
	.double	242
	.double	225
	.double	260
	.double	261
	.double	262
	.double	263
	.double	0
	.double	0
	.double	230
	.double	231
	.double	232
	.double	73
	.double	76
	.double	77
	.double	74
	.double	98
	.double	0
	.double	0
	.double	111
	.double	111
	.double	118
	.double	38
	.double	122
	.double	123
	.double	124
	.double	125
	.double	14
	.double	31
	.double	33
	.double	0
	.double	39
	.double	42
	.double	43
	.double	85
	.double	0
	.double	89
	.double	0
	.double	55
	.double	0
	.double	0
	.double	154
	.double	0
	.double	283
	.double	57
	.double	59
	.double	61
	.double	93
	.double	95
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	177
	.double	-2
	.double	0
	.double	258
	.double	13
	.double	182
	.double	257
	.double	38
	.double	193
	.double	38
	.double	0
	.double	175
	.double	175
	.double	175
	.double	210
	.double	132
	.double	133
	.double	134
	.double	0
	.double	136
	.double	137
	.double	138
	.double	0
	.double	0
	.double	0
	.double	150
	.double	151
	.double	170
	.double	0
	.double	0
	.double	0
	.double	172
	.double	173
	.double	212
	.double	0
	.double	221
	.double	222
	.double	0
	.double	224
	.double	0
	.double	0
	.double	-2
	.double	0
	.double	-2
	.double	-2
	.double	0
	.double	0
	.double	226
	.double	227
	.double	233
	.double	234
	.double	235
	.double	236
	.double	237
	.double	238
	.double	249
	.double	38
	.double	0
	.double	0
	.double	0
	.double	81
	.double	83
	.double	84
	.double	112
	.double	112
	.double	0
	.double	0
	.double	4
	.double	-2
	.double	37
	.double	32
	.double	0
	.double	40
	.double	45
	.double	88
	.double	90
	.double	54
	.double	0
	.double	155
	.double	157
	.double	159
	.double	0
	.double	0
	.double	47
	.double	0
	.double	0
	.double	94
	.double	96
	.double	67
	.double	70
	.double	69
	.double	68
	.double	18
	.double	0
	.double	25
	.double	27
	.double	176
	.double	180
	.double	-2
	.double	192
	.double	38
	.double	195
	.double	0
	.double	198
	.double	0
	.double	0
	.double	203
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	135
	.double	142
	.double	143
	.double	147
	.double	0
	.double	0
	.double	0
	.double	214
	.double	219
	.double	0
	.double	223
	.double	0
	.double	217
	.double	213
	.double	0
	.double	0
	.double	264
	.double	0
	.double	0
	.double	256
	.double	175
	.double	0
	.double	250
	.double	38
	.double	0
	.double	228
	.double	229
	.double	38
	.double	244
	.double	0
	.double	0
	.double	79
	.double	100
	.double	101
	.double	0
	.double	0
	.double	113
	.double	0
	.double	121
	.double	126
	.double	50
	.double	15
	.double	0
	.double	0
	.double	0
	.double	0
	.double	34
	.double	35
	.double	41
	.double	0
	.double	56
	.double	156
	.double	160
	.double	0
	.double	161
	.double	0
	.double	58
	.double	0
	.double	62
	.double	64
	.double	19
	.double	0
	.double	0
	.double	194
	.double	38
	.double	0
	.double	201
	.double	202
	.double	207
	.double	139
	.double	140
	.double	141
	.double	-2
	.double	145
	.double	146
	.double	149
	.double	148
	.double	171
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	0
	.double	220
	.double	216
	.double	0
	.double	215
	.double	0
	.double	204
	.double	239
	.double	240
	.double	0
	.double	252
	.double	0
	.double	241
	.double	243
	.double	38
	.double	78
	.double	0
	.double	82
	.double	83
	.double	109
	.double	110
	.double	51
	.double	0
	.double	9
	.double	10
	.double	11
	.double	12
	.double	91
	.double	162
	.double	163
	.double	60
	.double	0
	.double	0
	.double	26
	.double	181
	.double	197
	.double	199
	.double	205
	.double	0
	.double	164
	.double	165
	.double	166
	.double	167
	.double	168
	.double	169
	.double	218
	.double	0
	.double	266
	.double	268
	.double	282
	.double	271
	.double	0
	.double	275
	.double	253
	.double	38
	.double	251
	.double	38
	.double	246
	.double	0
	.double	80
	.double	52
	.double	0
	.double	63
	.double	65
	.double	38
	.double	0
	.double	259
	.double	0
	.double	282
	.double	270
	.double	277
	.double	0
	.double	282
	.double	272
	.double	0
	.double	0
	.double	245
	.double	38
	.double	127
	.double	175
	.double	0
	.double	267
	.double	269
	.double	0
	.double	283
	.double	283
	.double	274
	.double	282
	.double	0
	.double	254
	.double	38
	.double	248
	.double	200
	.double	208
	.double	0
	.double	280
	.double	281
	.double	273
	.double	276
	.double	247
	.double	278
	.double	0
	.double	0
	.double	279
.globl	_yydebug
.comm	_yydebug,4
.globl	_yyv
.comm	_yyv,1000
.globl	_yys
.comm	_yys,1000
.globl	_yypv
.comm	_yypv,4
.globl	_yyps
.comm	_yyps,4
.globl	_yystate
.comm	_yystate,4
.globl	_yytmp
.comm	_yytmp,4
.globl	_yynerrs
.comm	_yynerrs,4
.globl	_yyerrflag
.comm	_yyerrflag,4
.globl	_yychar
.comm	_yychar,4
.text
.globl	_yyparse
_yyparse:
	.enter
	movd	$_yyv+-4, _yypv
	movd	$_yys+-4, _yyps
	movqd	0, _yystate
	movqd	0, _yytmp
	movqd	0, _yynerrs
	movqd	0, _yyerrflag
	movqd	-1, _yychar
	br	.L205
.L206:
	movd	_yypv, r6
	movd	_yyps, r5
	movd	_yystate, r4
	br	.L207
.L205:
	movd	_yypv, r6
	movd	_yyps, r5
	movd	_yystate, r4
.L208:
	addqd	4, r5
	movd	r5, r0
	cmpd	r0,$_yys+1000
	blt	.L209
	movd	$.L211, tos
	jsr	_yyerror
	adjspb	$-4
	movqd	1, r0
	br	.L202
.L209:
	movd	r4, 0(r5)
	addqd	4, r6
	movd	r6, r0
	movd	_yyval, 0(r0)
.L207:
	movd	r4,r0
	ashd	$2,r0
	movd	_yypact(r0), -4(fp)
	movd	-4(fp), r1
	cmpd	r1,$-1000
	bgt	.L212
	br	.L213
.L212:
	cmpqd	0, _yychar
	ble	.L215
	jsr	_yylex
	movd	r0, _yychar
	movd	_yychar, r1
	cmpqd	0, r1
	ble	.L215
	movqd	0, _yychar
.L215:
	addd	_yychar,-4(fp)
	cmpqd	0, -4(fp)
	bgt	.L9999
	cmpd	-4(fp),$1451
	blt	.L216
.L9999:
	br	.L213
.L216:
	movd	-4(fp), r0
	ashd	$2,r0
	movd	_yyact(r0), -4(fp)
	movd	-4(fp), r1
	ashd	$2,r1
	cmpd	_yychk(r1),_yychar
	bne	.L217
	movqd	-1, _yychar
	movd	_yylval, _yyval
	movd	-4(fp), r4
	cmpqd	0, _yyerrflag
	bge	.L218
	addqd	-1, _yyerrflag
.L218:
	br	.L208
.L217:
.L213:
	movd	r4,r0
	ashd	$2,r0
	movd	_yydef(r0), -4(fp)
	movd	-4(fp), r1
	cmpqd	-2, r1
	bne	.L219
	cmpqd	0, _yychar
	ble	.L220
	jsr	_yylex
	movd	r0, _yychar
	movd	_yychar, r1
	cmpqd	0, r1
	ble	.L220
	movqd	0, _yychar
.L220:
	movd	$_yyexca, -8(fp)
	br	.L222
.L223:
	addr	8(-8(fp)), -8(fp)
.L222:
	cmpqd	-1, 0(-8(fp))
	bne	.L223
	cmpd	4(-8(fp)),r4
	bne	.L223
.L221:
	br	.L225
.L226:
.L225:
	addr	8(-8(fp)), -8(fp)
	movd	-8(fp), r0
	cmpqd	0, 0(r0)
	bgt	.L9998
	cmpd	0(-8(fp)),_yychar
	bne	.L226
.L9998:
.L224:
	movd	4(-8(fp)), -4(fp)
	movd	-4(fp), r0
	cmpqd	0, r0
	ble	.L227
	movqd	0, r0
	br	.L202
.L227:
.L219:
	cmpqd	0, -4(fp)
	bne	.L228
	movd	_yyerrflag, r0
	br	.L230
.L231:
	movd	$.L232, tos
	jsr	_yyerror
	adjspb	$-4
	br	.L233
.L234:
	movd	_yypv, r6
	movd	_yyps, r5
	movd	_yystate, r4
	addqd	1, _yynerrs
.L233:
.L235:
.L236:
	movqd	3, _yyerrflag
	br	.L238
.L239:
	movd	0(r5), r0
	ashd	$2,r0
	movd	_yypact(r0),r1
	addr	256(r1), r1
	movd	r1, -4(fp)
	cmpqd	0, -4(fp)
	bgt	.L240
	cmpd	-4(fp),$1451
	bge	.L240
	movd	-4(fp), r0
	ashd	$2,r0
	movd	_yyact(r0), r0
	ashd	$2,r0
	cmpd	_yychk(r0),$256
	bne	.L240
	movd	-4(fp), r0
	ashd	$2,r0
	movd	_yyact(r0), r4
	br	.L208
.L240:
	addqd	-4, r5
	addqd	-4, r6
.L238:
	cmpd	r5,$_yys
	bge	.L239
.L237:
	movqd	1, r0
	br	.L202
.L241:
	cmpqd	0, _yychar
	bne	.L242
	movqd	1, r0
	br	.L202
.L242:
	movqd	-1, _yychar
	br	.L207
.L230:
.data
.L245:	.double	3, 0
.text
	checkd	r0, .L245, r0
	bfs	.L244
.L243:	cased .L246[r0:d]
.L246:
.double	.L231-.L243
.double	.L235-.L243
.double	.L236-.L243
.double	.L241-.L243
.L244:
.L229:
.L228:
	movd	-4(fp), _yytmp
	movd	r6, r7
	movd	-4(fp), r0
	ashd	$2,r0
	movd	_yyr2(r0), -8(fp)
	movd	-8(fp),r0
	andd	$1,r0
	cmpqd	0, r0
	bne	.L247
	ashd	$-1,-8(fp)
	movd	-8(fp),r0
	ashd	$2,r0
	subd	r0,r6
	movd	r6,r0
	addqd	4, r0
	movd	0(r0), _yyval
	movd	-8(fp),r0
	ashd	$2,r0
	subd	r0,r5
	movd	-4(fp), r0
	ashd	$2,r0
	movd	_yyr1(r0), -4(fp)
	movd	-4(fp), r1
	ashd	$2,r1
	movd	_yypgo(r1),r0
	addd	0(r5),r0
	addqd	1, r0
	movd	r0, r4
	cmpd	r4,$1451
	bge	.L9997
	movd	r4,r0
	ashd	$2,r0
	movd	_yyact(r0), r4
	movd	r4, r1
	ashd	$2,r1
	negd	-4(fp),r0
	cmpd	_yychk(r1),r0
	beq	.L248
.L9997:
	movd	-4(fp), r0
	ashd	$2,r0
	movd	_yypgo(r0), r0
	ashd	$2,r0
	movd	_yyact(r0), r4
.L248:
	br	.L208
.L247:
	ashd	$-1,-8(fp)
	movd	-8(fp),r0
	ashd	$2,r0
	subd	r0,r6
	movd	r6,r0
	addqd	4, r0
	movd	0(r0), _yyval
	movd	-8(fp),r0
	ashd	$2,r0
	subd	r0,r5
	movd	-4(fp), r0
	ashd	$2,r0
	movd	_yyr1(r0), -4(fp)
	movd	-4(fp), r1
	ashd	$2,r1
	movd	_yypgo(r1),r0
	addd	0(r5),r0
	addqd	1, r0
	movd	r0, r4
	cmpd	r4,$1451
	bge	.L9996
	movd	r4,r0
	ashd	$2,r0
	movd	_yyact(r0), r4
	movd	r4, r1
	ashd	$2,r1
	negd	-4(fp),r0
	cmpd	_yychk(r1),r0
	beq	.L249
.L9996:
	movd	-4(fp), r0
	ashd	$2,r0
	movd	_yypgo(r0), r0
	ashd	$2,r0
	movd	_yyact(r0), r4
.L249:
	movd	r4, _yystate
	movd	r5, _yyps
	movd	r6, _yypv
	movd	_yytmp, r0
	br	.L251
.L252:
	movqd	2, _graal
	br	.L250
.L253:
	movqd	3, _graal
	br	.L250
.L254:
	movqd	4, _graal
	movd	-4(r7), tos
	jsr	_doinits
	adjspb	$-4
	movd	r7,r0
	addqd	-4, r0
	movd	r0, tos
	jsr	_frchain
	adjspb	$-4
	br	.L250
.L257:
	jsr	_endproc
	movqd	6, _graal
	br	.L250
.L259:
	movqd	5, _graal
	br	.L250
.L260:
	movqd	1, _graal
	br	.L250
.L261:
	movqd	1, _graal
	movd	$.L265, tos
	movd	$.L264, tos
	movd	$.L263, tos
	jsr	_errmess
	adjspb	$-12
	br	.L250
.L266:
	movqd	1, _eofneed
	br	.L250
.L267:
	cmpqd	0, _dclsect
	bne	.L268
	movd	$.L270, tos
	jsr	_warn
	adjspb	$-4
.L268:
	jsr	_bgnexec
	movd	r0, _yyval
	cmpqd	0, _dbgflag
	beq	.L271
	movd	$.L273, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L271:
	movd	-4(r7), tos
	jsr	_doinits
	adjspb	$-4
	movd	r7,r0
	addqd	-4, r0
	movd	r0, tos
	jsr	_frchain
	adjspb	$-4
	br	.L250
.L274:
	cmpqd	0, _dclsect
	beq	.L275
	movd	-4(r7), r0
	movd	$0, r1
	extd	r1, 0(r0), r1, 8
	cmpd	r1,$13
	beq	.L275
	movqd	0, _dclsect
.L275:
	cmpqd	0, _dbgflag
	beq	.L276
	movd	$.L277, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L276:
	br	.L250
.L278:
	jsr	_bgnexec
	movd	r0, _yyval
	br	.L250
.L279:
	movqd	0, _yyerrflag
	movd	$.L282, tos
	movd	$.L281, tos
	movd	$.L280, tos
	jsr	_errmess
	adjspb	$-12
	jsr	_bgnexec
	movd	r0, _yyval
	br	.L250
.L283:
	jsr	_bgnexec
	movd	r0, _yyval
	br	.L250
.L284:
	movd	_thisexec, r0
	movd	$3, r1
	insd	r1, $1, 12(r0), 1
	br	.L250
.L285:
	jsr	_addexec
	movd	r0, _yyval
	movd	_thisexec, r0
	movd	$3, r1
	insd	r1, $0, 12(r0), 1
	br	.L250
.L286:
	movqd	0, _procname
	movqd	0, _thisargs
	cmpqd	0, _procclass
	bne	.L287
	movqd	2, _procclass
.L287:
	br	.L288
.L289:
	movqd	0, _thisargs
	br	.L288
.L290:
	movqd	0, _thisargs
	br	.L288
.L291:
	movd	-4(r7), _thisargs
.L288:
	cmpqd	0, _procattrs
	beq	.L292
	cmpqd	0, _procname
	bne	.L293
	movd	$.L296, tos
	movd	$.L295, tos
	jsr	_dclerr
	adjspb	$-8
	br	.L297
.L293:
	movqd	0, tos
	movd	_procname, tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, tos
	movd	_procattrs, tos
	jsr	_attvars
	adjspb	$-8
	movqd	4, _procclass
.L297:
.L292:
	jsr	_procnm
	movd	r0, tos
	movd	$.L299, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-12
	cmpqd	0, _verbose
	beq	.L300
	movd	$.L301, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L300:
	br	.L250
.L302:
	movd	0(r7), tos
	jsr	_mkvar
	adjspb	$-4
	movd	r0, _procname
	movd	_procname, tos
	jsr	_extname
	adjspb	$-4
	br	.L250
.L304:
	movqd	0, _procattrs
	br	.L250
.L305:
	movd	-4(r7), _procattrs
	cmpqd	0, _procclass
	bne	.L306
	movqd	4, _procclass
.L306:
	br	.L250
.L307:
	jsr	_bgnproc
	movd	r0, _yyval
	movqd	0, _procclass
	br	.L250
.L308:
	jsr	_bgnproc
	movd	r0, _yyval
	movqd	1, _procclass
	br	.L250
.L309:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L310:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, tos
	movd	-8(r7), tos
	jsr	_hookup
	adjspb	$-8
	br	.L250
.L311:
	movd	0(r7), r0
	movd	$13, r1
	extd	r1, 12(r0), r1, 3
	cmpqd	0, r1
	bne	.L312
	movd	0(r7), r0
	movd	$13, r1
	insd	r1, $1, 12(r0), 3
	br	.L313
.L312:
	movd	0(r7), r0
	movd	4(r0), r0
	movd	4(r0), tos
	movd	$.L314, tos
	jsr	_dclerr
	adjspb	$-8
.L313:
	br	.L250
.L315:
	movqd	0, _optneed
	br	.L250
.L316:
	cmpqd	0, _blklevel
	bge	.L317
	movd	$.L320, tos
	movd	$.L319, tos
	jsr	_execerr
	adjspb	$-8
	jsr	_procnm
	movd	r0, tos
	movd	$.L321, tos
	jsr	_execerr
	adjspb	$-8
	jsr	_endproc
.L317:
	movqd	1, _optneed
	br	.L250
.L322:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_setopt
	adjspb	$-8
	movd	0(r7), tos
	jsr	_cfree
	adjspb	$-4
	br	.L250
.L325:
	movd	0(r7), tos
	movd	-8(r7), tos
	jsr	_setopt
	adjspb	$-8
	movd	-8(r7), tos
	jsr	_cfree
	adjspb	$-4
	movd	0(r7), tos
	jsr	_cfree
	adjspb	$-4
	br	.L250
.L326:
	movd	0(r7), r0
	movd	4(r0), tos
	movd	-8(r7), tos
	jsr	_setopt
	adjspb	$-8
	movd	-8(r7), tos
	jsr	_cfree
	adjspb	$-4
	movd	0(r7), tos
	jsr	_cfree
	adjspb	$-4
	br	.L250
.L327:
	movqd	1, _defneed
	br	.L250
.L328:
	movd	_thisctl, r0
	movd	$8, r1
	extd	r1, 0(r0), r1, 8
	cmpd	r1,$12
	beq	.L329
	movd	$.L331, tos
	movd	$.L330, tos
	jsr	_execerr
	adjspb	$-8
.L329:
	jsr	_exnull
	jsr	_popctl
	br	.L250
.L334:
	movqd	1, _igeol
	br	.L250
.L335:
	movd	0(r7), tos
	movd	-8(r7), tos
	jsr	_hookup
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L336:
	movd	0(r7), tos
	movd	-4(r7), tos
	jsr	_attvars
	adjspb	$-8
	movd	0(r7), _yyval
	br	.L250
.L337:
	movd	-4(r7), tos
	movd	-12(r7), tos
	jsr	_attvars
	adjspb	$-8
	movd	-4(r7), _yyval
	br	.L250
.L338:
	movqd	0, _yyval
	br	.L250
.L339:
	movqd	0, _yyval
	br	.L250
.L340:
	movqd	0, _yyval
	br	.L250
.L341:
	movd	0(r7), tos
	jsr	_mkequiv
	adjspb	$-4
	movqd	0, _yyval
	br	.L250
.L343:
	movd	0(r7), tos
	movd	-8(r7), tos
	jsr	_hookup
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L344:
	movqd	1, _ininit
	br	.L250
.L345:
	movqd	0, _ininit
	movd	0(r7), tos
	movd	-12(r7), tos
	jsr	_mkinit
	adjspb	$-8
	movd	-12(r7), tos
	jsr	_frexpr
	adjspb	$-4
	br	.L250
.L348:
	addr	122, tos
	addr	97, tos
	movd	_imptype, tos
	jsr	_setimpl
	adjspb	$-12
	br	.L250
.L350:
	movd	0(r7), r0
	movd	4(r0), _imptype
	movd	0(r7), tos
	jsr	_cfree
	adjspb	$-4
	br	.L250
.L351:
	movd	0(r7), tos
	movd	0(r7), tos
	movd	_imptype, tos
	jsr	_setimpl
	adjspb	$-12
	br	.L250
.L352:
	movd	0(r7), tos
	movd	-8(r7), tos
	movd	_imptype, tos
	jsr	_setimpl
	adjspb	$-12
	br	.L250
.L353:
	movd	-4(r7), tos
	jsr	_mkequiv
	adjspb	$-4
	br	.L250
.L354:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, tos
	movd	-8(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L355:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, tos
	movd	-8(r7), tos
	jsr	_hookup
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L356:
	movd	0(r7), tos
	movd	-4(r7), tos
	jsr	_attatt
	adjspb	$-8
	br	.L250
.L358:
	movd	-4(r7), r0
	movd	0(r7), 24(r0)
	br	.L250
.L359:
	addr	28, tos
	jsr	_intalloc
	adjspb	$-4
	movd	r0, _yyval
	movd	_yyval, r0
	movd	0(r7), 24(r0)
	br	.L250
.L360:
	movqd	0, _yyval
	br	.L250
.L361:
	movqd	1, _inbound
	br	.L250
.L362:
	movqd	0, _inbound
	movd	_arrays, tos
	movd	-4(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, _arrays
	movd	_arrays, _yyval
	br	.L250
.L363:
	movd	0(r7), tos
	movd	-8(r7), tos
	jsr	_hookup
	adjspb	$-8
	br	.L250
.L364:
	addr	12, tos
	jsr	_intalloc
	adjspb	$-4
	movd	r0, _yyval
	movd	_yyval, r0
	movqd	0, 4(r0)
	movd	_yyval, r0
	movd	0(r7), 8(r0)
	br	.L250
.L365:
	addr	12, tos
	jsr	_intalloc
	adjspb	$-4
	movd	r0, _yyval
	movd	_yyval, r0
	movd	-8(r7), 4(r0)
	movd	_yyval, r0
	movd	0(r7), 8(r0)
	br	.L250
.L366:
	movqd	0, _yyval
	br	.L250
.L367:
	movqd	0, _yyval
	br	.L250
.L368:
	movd	0(r7), tos
	movd	-8(r7), tos
	jsr	_hookup
	adjspb	$-8
	br	.L250
.L369:
	cmpqd	0, 0(r7)
	beq	.L370
	movd	-4(r7), r0
	cmpqd	0, 20(r0)
	bne	.L371
	movd	-4(r7), r0
	movd	0(r7), 20(r0)
	br	.L372
.L371:
	movd	-4(r7), r0
	movd	20(r0), tos
	movd	0(r7), tos
	jsr	_eqdim
	adjspb	$-8
	cmpqd	0, r0
	bne	.L374
	movd	-4(r7), r0
	movd	4(r0), tos
	movd	$.L375, tos
	jsr	_dclerr
	adjspb	$-8
.L374:
.L372:
.L370:
	movqd	0, tos
	movd	-4(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L376:
	movqd	1, _ininit
	br	.L250
.L377:
	movqd	0, _ininit
	cmpqd	0, -8(r7)
	beq	.L378
	movd	-16(r7), r0
	movd	4(r0), r0
	movd	4(r0), tos
	movd	$.L379, tos
	jsr	_dclerr
	adjspb	$-8
.L378:
	cmpqd	0, -12(r7)
	beq	.L380
	movd	-16(r7), r0
	cmpqd	0, 20(r0)
	bne	.L381
	movd	-16(r7), r0
	movd	-12(r7), 20(r0)
	br	.L382
.L381:
	movd	-16(r7), r0
	movd	20(r0), tos
	movd	-12(r7), tos
	jsr	_eqdim
	adjspb	$-8
	cmpqd	0, r0
	bne	.L383
	movd	-16(r7), r0
	movd	4(r0), r0
	movd	4(r0), tos
	movd	$.L384, tos
	jsr	_dclerr
	adjspb	$-8
.L383:
.L382:
.L380:
	cmpqd	0, 0(r7)
	beq	.L385
	movd	-16(r7), r0
	cmpqd	0, 8(r0)
	beq	.L385
	movd	-16(r7), r0
	movd	4(r0), r0
	movd	4(r0), tos
	movd	$.L386, tos
	jsr	_dclerr
	adjspb	$-8
.L385:
	movd	-16(r7), r0
	movd	0(r7), 8(r0)
	movqd	0, tos
	movd	-16(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L387:
	movd	0(r7), tos
	jsr	_mkvar
	adjspb	$-4
	movd	r0, _yyval
	br	.L250
.L388:
	movd	0(r7), tos
	movd	-4(r7), tos
	jsr	_attatt
	adjspb	$-8
	br	.L250
.L389:
	addr	28, tos
	jsr	_intalloc
	adjspb	$-4
	movd	r0, _yyval
	movd	_yyval, r0
	movd	0(r7), 24(r0)
	br	.L250
.L390:
	addr	28, tos
	jsr	_intalloc
	adjspb	$-4
	movd	r0, _yyval
	cmpqd	7, 0(r7)
	bne	.L391
	movd	_yyval, r0
	movqd	1, 8(r0)
.L391:
	movd	_yyval, r0
	movd	0(r7), 12(r0)
	br	.L250
.L392:
	addr	28, tos
	jsr	_intalloc
	adjspb	$-4
	movd	r0, _yyval
	movd	_yyval, r0
	movqd	5, 12(r0)
	movd	_yyval, r0
	movd	-4(r7), 20(r0)
	br	.L250
.L393:
	addr	28, tos
	jsr	_intalloc
	adjspb	$-4
	movd	r0, _yyval
	movd	_yyval, r0
	movd	0(r7), 4(r0)
	br	.L250
.L394:
	addr	28, tos
	jsr	_intalloc
	adjspb	$-4
	movd	r0, _yyval
	movd	_yyval, r0
	movqd	7, 4(r0)
	movd	_yyval, r0
	movd	-4(r7), 16(r0)
	br	.L250
.L395:
	addr	28, tos
	jsr	_intalloc
	adjspb	$-4
	movd	r0, _yyval
	movd	_yyval, r0
	addr	11, 4(r0)
	movd	-4(r7), tos
	jsr	_mkfield
	adjspb	$-4
	movd	_yyval, r1
	movd	r0, 16(r1)
	br	.L250
.L396:
	addr	28, tos
	jsr	_intalloc
	adjspb	$-4
	movd	r0, _yyval
	movd	_yyval, r0
	addr	8, 4(r0)
	movd	_yyval, r0
	movd	0(r7), 16(r0)
	br	.L250
.L397:
	addr	28, tos
	jsr	_intalloc
	adjspb	$-4
	movd	r0, _yyval
	movd	_yyval, r0
	movd	0(r7), 0(r0)
	br	.L250
.L398:
	movqd	4, _yyval
	movd	$.L399, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
	br	.L250
.L400:
	movqd	3, _yyval
	br	.L250
.L401:
	movqd	3, _yyval
	br	.L250
.L402:
	movqd	2, _yyval
	movd	$.L403, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
	br	.L250
.L404:
	movqd	7, _yyval
	br	.L250
.L405:
	movd	-4(r7), _yyval
	br	.L250
.L406:
	movd	-4(r7), _yyval
	br	.L250
.L407:
	movqd	1, _comneed
	br	.L250
.L408:
	movd	$.L409, tos
	jsr	_mkcomm
	adjspb	$-4
	movd	r0, _yyval
	br	.L250
.L410:
	movqd	1, _yyval
	br	.L250
.L411:
	movqd	2, _yyval
	br	.L250
.L412:
	movqd	5, _yyval
	br	.L250
.L413:
	movqd	4, _yyval
	br	.L250
.L414:
	movqd	3, _yyval
	br	.L250
.L415:
	movqd	3, _yyval
	br	.L250
.L416:
	movd	0(r7), r0
	movd	8(r0), _yyval
	br	.L250
.L417:
	movd	0(r7), tos
	movd	-8(r7), tos
	jsr	_mkstruct
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L418:
	movd	0(r7), tos
	movqd	0, tos
	jsr	_mkstruct
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L419:
	movd	0(r7), r0
	cmpqd	0, 8(r0)
	beq	.L420
	movd	0(r7), r0
	movd	8(r0), r0
	movd	$16, r1
	extd	r1, 0(r0), r1, 8
	cmpd	r1,_blklevel
	bhs	.L420
	movd	0(r7), tos
	jsr	_hide
	adjspb	$-4
.L420:
	movd	0(r7), r0
	movd	$0, r1
	insd	r1, $12, 0(r0), 8
	br	.L250
.L422:
	movd	0(r7), r0
	cmpqd	0, 8(r0)
	beq	.L423
	movd	0(r7), r0
	movd	8(r0), r0
	movd	$16, r1
	extd	r1, 0(r0), r1, 8
	cmpd	r1,_blklevel
	bhs	.L424
	movd	0(r7), tos
	jsr	_hide
	adjspb	$-4
	br	.L425
.L424:
	movd	0(r7), r0
	movd	4(r0), tos
	movd	$.L426, tos
	jsr	_dclerr
	adjspb	$-8
.L425:
.L423:
	br	.L250
.L427:
	addqd	1, _instruct
	br	.L250
.L428:
	addqd	-1, _instruct
	br	.L250
.L429:
	movd	-12(r7), _yyval
	movqd	-1, _prevv
	br	.L250
.L430:
	movqd	1, _yyval
	br	.L250
.L431:
	movqd	0, _yyval
	br	.L250
.L432:
	movd	0(r7), r0
	movd	$0, r1
	extd	r1, 0(r0), r1, 8
	cmpqd	5, r1
	bne	.L433
	movd	0(r7), tos
	jsr	_funcinv
	adjspb	$-4
	movd	r0, 0(r7)
.L433:
	movd	0(r7), r0
	movd	$9, r1
	extd	r1, 12(r0), r1, 4
	cmpqd	0, r1
	bne	.L434
	movd	0(r7), r0
	movd	$5, r1
	extd	r1, 12(r0), r1, 1
	cmpqd	0, r1
	bne	.L434
	movd	0(r7), tos
	jsr	_impldecl
	adjspb	$-4
	br	.L436
.L434:
	movd	0(r7), r0
	movd	$0, r1
	extd	r1, 0(r0), r1, 8
	cmpd	r1,$9
	bne	.L437
	movd	0(r7), r0
	movd	$1, r1
	extd	r1, 12(r0), r1, 1
	cmpqd	0, r1
	bne	.L437
	movd	0(r7), r0
	movd	$5, r1
	extd	r1, 12(r0), r1, 1
	cmpqd	0, r1
	bne	.L437
	cmpqd	0, _inbound
	bne	.L437
	movd	0(r7), tos
	jsr	_dclit
	adjspb	$-4
.L437:
.L436:
	movd	0(r7), r0
	movd	$9, r1
	extd	r1, 12(r0), r1, 4
	cmpd	r1,$11
	bne	.L439
	movd	0(r7), tos
	jsr	_extrfield
	adjspb	$-4
	movd	r0, _yyval
.L439:
	br	.L250
.L440:
	cmpd	0(r7),$81
	bne	.L9995
	movd	$.L441, r0
	br	.L9994
.L9995:
	movd	$.L442, r0
.L9994:
	movd	r0, tos
	movqd	4, tos
	jsr	_mkconst
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L443:
	movd	0(r7), tos
	movd	-4(r7), tos
	jsr	_typexpr
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L444:
	cmpqd	0, _ininit
	bne	.L445
	movd	0(r7), r0
	movd	$0, r1
	extd	r1, 0(r0), r1, 8
	cmpqd	7, r1
	bne	.L445
	movd	0(r7), tos
	jsr	_compconst
	adjspb	$-4
	movd	r0, _yyval
	br	.L446
.L445:
	movd	0(r7), r0
	movd	$8, r1
	insd	r1, $1, 12(r0), 1
.L446:
	br	.L250
.L447:
	movd	0(r7), tos
	movd	-8(r7), tos
	movd	-4(r7), tos
	movqd	1, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, _yyval
	br	.L250
.L448:
	movd	0(r7), tos
	movd	-8(r7), tos
	movd	-4(r7), tos
	movqd	1, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, _yyval
	br	.L250
.L449:
	movd	0(r7), tos
	movd	-8(r7), tos
	movd	-4(r7), tos
	movqd	1, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, _yyval
	br	.L250
.L450:
	cmpqd	2, -4(r7)
	bne	.L451
	movqd	0, tos
	movd	0(r7), tos
	movqd	2, tos
	addr	21, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, _yyval
	br	.L452
.L451:
	movd	0(r7), _yyval
.L452:
	br	.L250
.L453:
	movqd	1, tos
	jsr	_mkint
	adjspb	$-4
	movd	r0, tos
	movd	0(r7), tos
	movd	-4(r7), tos
	movqd	2, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, _yyval
	br	.L250
.L454:
	movd	0(r7), tos
	movd	-8(r7), tos
	movd	-4(r7), tos
	movqd	4, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, _yyval
	br	.L250
.L455:
	movd	0(r7), tos
	movd	-8(r7), tos
	movd	-4(r7), tos
	movqd	3, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, _yyval
	br	.L250
.L456:
	movd	0(r7), tos
	movd	-8(r7), tos
	movd	-4(r7), tos
	movqd	3, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, _yyval
	br	.L250
.L457:
	movqd	0, tos
	movd	0(r7), tos
	movd	-4(r7), tos
	addr	20, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, _yyval
	br	.L250
.L458:
	movd	-8(r7), r0
	movd	$0, r1
	extd	r1, 0(r0), r1, 8
	cmpqd	5, r1
	bne	.L459
	movqd	0, tos
	movd	$.L461, tos
	jsr	_exprerr
	adjspb	$-8
	jsr	_errnode
	movd	r0, _yyval
	br	.L462
.L459:
	movd	0(r7), tos
	movd	-8(r7), tos
	movd	-4(r7), tos
	movqd	2, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, _yyval
.L462:
	br	.L250
.L463:
	movd	0(r7), tos
	movd	-8(r7), tos
	movqd	0, tos
	movqd	6, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, _yyval
	br	.L250
.L464:
	jsr	_errnode
	movd	r0, _yyval
	br	.L250
.L465:
	movd	0(r7), r0
	movd	$0, r1
	extd	r1, 0(r0), r1, 8
	cmpd	r1,$9
	bne	.L466
	movd	0(r7), r0
	movd	$1, r1
	extd	r1, 12(r0), r1, 1
	cmpqd	0, r1
	bne	.L466
	movd	0(r7), r0
	cmpqd	0, 24(r0)
	bne	.L466
	movd	0(r7), r0
	movd	$5, r1
	extd	r1, 12(r0), r1, 1
	cmpqd	0, r1
	bne	.L466
	cmpqd	0, _inbound
	bne	.L466
	movd	0(r7), tos
	jsr	_dclit
	adjspb	$-4
.L466:
	br	.L250
.L467:
	movd	0(r7), r0
	movd	$0, r1
	extd	r1, 0(r0), r1, 8
	cmpqd	7, r1
	beq	.L468
	movqd	0, tos
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, tos
	movqd	0, tos
	movqd	7, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, 0(r7)
.L468:
	movd	-4(r7), r0
	cmpqd	0, 20(r0)
	beq	.L469
	movd	-4(r7), r0
	movd	$1, r1
	extd	r1, 12(r0), r1, 1
	cmpqd	0, r1
	bne	.L470
	movd	-4(r7), r0
	movd	$5, r1
	extd	r1, 12(r0), r1, 1
	cmpqd	0, r1
	bne	.L470
	movd	-4(r7), tos
	jsr	_dclit
	adjspb	$-4
.L470:
	movd	0(r7), tos
	movd	-4(r7), tos
	jsr	_subscript
	adjspb	$-8
	movd	r0, _yyval
	br	.L471
.L469:
	movd	0(r7), tos
	movd	-4(r7), tos
	jsr	_mkcall
	adjspb	$-8
	movd	r0, _yyval
.L471:
	br	.L250
.L472:
	movd	0(r7), tos
	movd	-8(r7), tos
	jsr	_strucelt
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L473:
	movd	0(r7), r0
	movd	$0, r1
	extd	r1, 0(r0), r1, 8
	cmpqd	7, r1
	beq	.L474
	movqd	0, tos
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, tos
	movqd	0, tos
	movqd	7, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, 0(r7)
.L474:
	movd	0(r7), tos
	movd	-4(r7), tos
	movd	-12(r7), tos
	jsr	_strucelt
	adjspb	$-8
	movd	r0, tos
	jsr	_subscript
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L475:
	movd	0(r7), tos
	movd	-8(r7), tos
	jsr	_mkarrow
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L476:
	movd	0(r7), r0
	cmpqd	0, 8(r0)
	bne	.L477
	movd	0(r7), tos
	jsr	_mkvar
	adjspb	$-4
.L477:
	cmpqd	0, _inbound
	beq	.L478
	movd	0(r7), r0
	movd	8(r0), r0
	movd	$0, r1
	insd	r1, $1, 12(r0), 1
.L478:
	movd	0(r7), r0
	movd	8(r0), tos
	jsr	_cpexpr
	adjspb	$-4
	movd	r0, _yyval
	br	.L250
.L479:
	movqd	0, tos
	movqd	0, tos
	movqd	0, tos
	movqd	7, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, _yyval
	br	.L250
.L480:
	movd	-4(r7), _yyval
	br	.L250
.L481:
	movqd	0, tos
	movd	-4(r7), tos
	movqd	0, tos
	movqd	7, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, _yyval
	br	.L250
.L482:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, tos
	movd	-8(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L483:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, tos
	movd	-8(r7), tos
	jsr	_hookup
	adjspb	$-8
	br	.L250
.L484:
	movd	-4(r7), r0
	movd	20(r0), tos
	movd	-4(r7), r0
	movd	16(r0), tos
	movd	-4(r7), r0
	movd	$9, r1
	extd	r1, 12(r0), r1, 4
	movd	r1, tos
	jsr	_esizeof
	adjspb	$-12
	movd	r0, _yyval
	movd	-4(r7), tos
	jsr	_frexpr
	adjspb	$-4
	br	.L250
.L485:
	movd	-4(r7), r0
	cmpqd	2, 4(r0)
	bne	.L486
	movd	-4(r7), r0
	cmpqd	0, 0(r0)
	beq	.L486
	movd	-4(r7), r0
	movqd	3, 4(r0)
.L486:
	movd	-4(r7), r0
	movd	24(r0), tos
	movd	-4(r7), r0
	movd	16(r0), tos
	movd	-4(r7), r0
	movd	4(r0), tos
	jsr	_esizeof
	adjspb	$-12
	movd	r0, _yyval
	movd	-4(r7), tos
	jsr	_cfree
	adjspb	$-4
	br	.L250
.L487:
	movd	_tailor+40,r0
	quod	_tailor+36,r0
	movd	r0, tos
	jsr	_mkint
	adjspb	$-4
	movd	r0, _yyval
	br	.L250
.L488:
	movd	-4(r7), r0
	movd	20(r0), tos
	movd	-4(r7), r0
	movd	16(r0), tos
	movd	-4(r7), r0
	movd	$9, r1
	extd	r1, 12(r0), r1, 4
	movd	r1, tos
	jsr	_elenof
	adjspb	$-12
	movd	r0, _yyval
	movd	-4(r7), tos
	jsr	_frexpr
	adjspb	$-4
	br	.L250
.L489:
	movd	-4(r7), r0
	movd	24(r0), tos
	movd	-4(r7), r0
	movd	16(r0), tos
	movd	-4(r7), r0
	movd	4(r0), tos
	jsr	_elenof
	adjspb	$-12
	movd	r0, _yyval
	movd	-4(r7), tos
	jsr	_cfree
	adjspb	$-4
	br	.L250
.L490:
	movqd	1, tos
	jsr	_mkint
	adjspb	$-4
	movd	r0, _yyval
	br	.L250
.L491:
	movd	-4(r7), _yyval
	br	.L250
.L492:
	cmpqd	0, _dbgflag
	beq	.L493
	movd	$.L494, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L493:
	br	.L250
.L495:
	jsr	_bgnexec
	movd	r0, _yyval
	cmpqd	0, _ncases
	bge	.L496
	movqd	0, _ncases
.L496:
	br	.L250
.L497:
	movd	-8(r7), r0
	movd	$0, r1
	extd	r1, 0(r0), r1, 8
	cmpqd	5, r1
	bne	.L498
	movd	-8(r7), r0
	movd	4(r0), r0
	movd	4(r0), tos
	movd	$.L499, tos
	jsr	_dclerr
	adjspb	$-8
	movd	-8(r7), tos
	jsr	_frexpr
	adjspb	$-4
	movd	0(r7), tos
	jsr	_frexpr
	adjspb	$-4
	br	.L500
.L498:
	movd	0(r7), tos
	movd	-4(r7), tos
	movd	-8(r7), tos
	jsr	_exasgn
	adjspb	$-12
.L500:
	br	.L250
.L502:
	movqd	1, tos
	jsr	_mkint
	adjspb	$-4
	movd	r0, tos
	movd	-4(r7), tos
	movd	0(r7), tos
	jsr	_exasgn
	adjspb	$-12
	br	.L250
.L503:
	movd	0(r7), tos
	jsr	_excall
	adjspb	$-4
	br	.L250
.L505:
	movd	0(r7), tos
	jsr	_excall
	adjspb	$-4
	br	.L250
.L506:
	cmpqd	0, _dbgflag
	beq	.L507
	movd	$.L508, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L507:
	jsr	_addexec
	br	.L250
.L509:
	movd	_thisexec, r0
	movd	$2, r1
	insd	r1, $1, 12(r0), 1
	br	.L250
.L510:
	movd	_thisexec, r0
	movd	$0, r1
	insd	r1, $1, 12(r0), 1
	jsr	_popctl
	br	.L250
.L511:
	movd	_thisexec, r0
	movd	$1, r1
	insd	r1, $1, 12(r0), 1
	br	.L250
.L512:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_exio
	adjspb	$-8
	br	.L250
.L514:
	jsr	_exnull
	br	.L250
.L515:
	jsr	_exnull
	movd	0(r7), tos
	addr	9, tos
	jsr	_putsii
	adjspb	$-8
	movd	0(r7), tos
	jsr	_cfree
	adjspb	$-4
	jsr	_exnull
	br	.L250
.L517:
	cmpqd	0, _dbgflag
	beq	.L518
	movd	$.L519, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L518:
	br	.L250
.L520:
	cmpqd	0, _dbgflag
	beq	.L521
	movd	$.L522, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L521:
	br	.L250
.L523:
	movd	_thisexec, r0
	movd	$3, r1
	insd	r1, $1, 12(r0), 1
	addqd	1, _blklevel
	movqd	1, _dclsect
	movd	_blklevel, r0
	ashd	$2,r0
	movqd	0, _ndecl(r0)
	movd	_blklevel, r0
	ashd	$2,r0
	movqd	0, _nhid(r0)
	br	.L250
.L524:
	movd	_blklevel, r0
	ashd	$2,r0
	cmpqd	0, _ndecl(r0)
	beq	.L525
	jsr	_unhide
.L525:
	addqd	-1, _blklevel
	movqd	0, _dclsect
	br	.L250
.L527:
	movqd	1, tos
	movd	-8(r7), tos
	jsr	_mklabel
	adjspb	$-8
	br	.L250
.L529:
	movqd	1, tos
	movd	-4(r7), tos
	jsr	_mkilab
	adjspb	$-4
	movd	r0, tos
	jsr	_mklabel
	adjspb	$-8
	br	.L250
.L530:
	movqd	1, tos
	movd	-12(r7), tos
	jsr	_mkilab
	adjspb	$-4
	movd	r0, tos
	jsr	_mklabel
	adjspb	$-8
	br	.L250
.L531:
	jsr	_brkcase
	movqd	1, tos
	movqd	0, tos
	jsr	_mkcase
	adjspb	$-8
	br	.L250
.L534:
	jsr	_brkcase
	br	.L250
.L535:
	movqd	1, tos
	movd	0(r7), tos
	jsr	_mkcase
	adjspb	$-8
	br	.L250
.L536:
	movqd	1, tos
	movd	0(r7), tos
	jsr	_mkcase
	adjspb	$-8
	br	.L250
.L537:
	cmpqd	0, _dbgflag
	beq	.L538
	movd	$.L539, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L538:
	movd	-20(r7), r0
	movd	$1, r1
	extd	r1, 12(r0), r1, 1
	movd	0(r7), r0
	movd	$1, r2
	extd	r2, 12(r0), r2, 1
	andd	r2,r1
	movd	r1, _i
	jsr	_addexec
	jsr	_addexec
	movd	r0, _yyval
	movd	_thisexec, r0
	movd	$1, r1
	insd	r1, _i, 12(r0), 1
	cmpqd	0, _dbgflag
	beq	.L540
	movd	$.L541, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L540:
	br	.L250
.L542:
	cmpqd	0, _dbgflag
	beq	.L543
	movd	$.L544, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L543:
	movqd	1, _pushlex
	movqd	-1, _yychar
	jsr	_ifthen
	movd	r0, _yyval
	cmpqd	0, _dbgflag
	beq	.L545
	movd	$.L546, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L545:
	br	.L250
.L547:
	cmpqd	0, _dbgflag
	beq	.L548
	movd	$.L549, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L548:
	br	.L250
.L550:
	cmpqd	0, _dbgflag
	beq	.L551
	movd	$.L552, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L551:
	jsr	_addexec
	movd	r0, _yyval
	br	.L250
.L553:
	movd	-4(r7), tos
	movqd	2, tos
	jsr	_pushctl
	adjspb	$-8
	br	.L250
.L555:
	movd	_thisctl, r0
	cmpqd	0, 20(r0)
	bne	.L556
	jsr	_nextlab
	movd	_thisctl, r1
	movd	r0, 20(r1)
.L556:
	movd	_thisctl, r0
	movd	20(r0), tos
	jsr	_exgoto
	adjspb	$-4
	jsr	_nextlab
	movd	_thisctl, r1
	movd	28(r1), r1
	ashd	$2,r1
	movd	r0, _indifs(r1)
	movd	_indifs(r1), tos
	jsr	_exlab
	adjspb	$-4
	br	.L250
.L560:
	movqd	0, tos
	movqd	4, tos
	jsr	_pushctl
	adjspb	$-8
	br	.L250
.L561:
	movqd	1, _pushlex
	movqd	-1, _yychar
	jsr	_addexec
	movd	r0, _yyval
	movd	_thisctl, r0
	movd	24(r0), tos
	jsr	_exgoto
	adjspb	$-4
	cmpqd	0, _dbgflag
	beq	.L562
	movd	$.L563, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L562:
	br	.L250
.L564:
	jsr	_addexec
	movd	r0, _yyval
	jsr	_exnull
	jsr	_nextlab
	movd	_thisctl, r1
	movd	28(r1), r1
	ashd	$2,r1
	movd	r0, _indifs(r1)
	movd	_indifs(r1), tos
	jsr	_exlab
	adjspb	$-4
	movd	_thisctl, r0
	movd	24(r0), tos
	movqd	0, tos
	movd	-4(r7), tos
	movqd	6, tos
	addr	20, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, tos
	jsr	_exifgo
	adjspb	$-8
	cmpqd	0, _dbgflag
	beq	.L566
	movd	$.L567, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L566:
	br	.L250
.L568:
	movqd	0, tos
	jsr	_exretn
	adjspb	$-4
	br	.L250
.L570:
	movd	0(r7), tos
	jsr	_exretn
	adjspb	$-4
	br	.L250
.L571:
	movd	0(r7), tos
	jsr	_exgoto
	adjspb	$-4
	br	.L250
.L572:
	movd	$.L574, tos
	movd	-4(r7), r0
	movd	4(r0), tos
	jsr	_equals
	adjspb	$-8
	cmpqd	0, r0
	bne	.L575
	movd	-4(r7), r0
	movd	4(r0), tos
	movd	$.L576, tos
	jsr	_execerr
	adjspb	$-8
	br	.L577
.L575:
	movd	0(r7), tos
	jsr	_exgoto
	adjspb	$-4
.L577:
	br	.L250
.L578:
	movd	0(r7), tos
	movd	-4(r7), tos
	jsr	_excompgoto
	adjspb	$-8
	br	.L250
.L580:
	movd	$.L581, tos
	movd	-8(r7), r0
	movd	4(r0), tos
	jsr	_equals
	adjspb	$-8
	cmpqd	0, r0
	bne	.L582
	movd	-8(r7), r0
	movd	4(r0), tos
	movd	$.L583, tos
	jsr	_execerr
	adjspb	$-8
	br	.L584
.L582:
	movd	0(r7), tos
	movd	-4(r7), tos
	jsr	_excompgoto
	adjspb	$-8
.L584:
	br	.L250
.L585:
	movd	-4(r7), _yyval
	br	.L250
.L586:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L587:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, tos
	movd	-8(r7), tos
	jsr	_hookup
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L588:
	movd	0(r7), _yyval
	br	.L250
.L589:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mklabel
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L590:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkilab
	adjspb	$-4
	movd	r0, tos
	jsr	_mklabel
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L591:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkcase
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L592:
	movqd	0, tos
	movqd	0, tos
	jsr	_mkcase
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L593:
	movqd	0, tos
	movqd	0, tos
	movd	0(r7), tos
	jsr	_exbrk
	adjspb	$-12
	br	.L250
.L595:
	movqd	0, tos
	movd	0(r7), tos
	movd	-4(r7), tos
	jsr	_exbrk
	adjspb	$-12
	br	.L250
.L596:
	movd	0(r7), tos
	movqd	0, tos
	movd	-4(r7), tos
	jsr	_exbrk
	adjspb	$-12
	br	.L250
.L597:
	movd	0(r7), tos
	movd	-4(r7), tos
	movd	-8(r7), tos
	jsr	_exbrk
	adjspb	$-12
	br	.L250
.L598:
	movd	-4(r7), tos
	movd	0(r7), tos
	movd	-8(r7), tos
	jsr	_exbrk
	adjspb	$-12
	br	.L250
.L599:
	movqd	1, _yyval
	br	.L250
.L600:
	movqd	0, _yyval
	br	.L250
.L601:
	movqd	0, _yyval
	br	.L250
.L602:
	movqd	5, _yyval
	br	.L250
.L603:
	movqd	6, _yyval
	br	.L250
.L604:
	movqd	7, _yyval
	br	.L250
.L605:
	movqd	4, _yyval
	br	.L250
.L606:
	addr	8, _yyval
	br	.L250
.L607:
	addr	12, _yyval
	br	.L250
.L608:
	movd	-4(r7), tos
	movqd	5, tos
	jsr	_pushctl
	adjspb	$-8
	cmpqd	0, _dbgflag
	beq	.L609
	movd	$.L610, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L609:
	br	.L250
.L611:
	movd	_thisctl, r0
	movd	24(r0), tos
	jsr	_exlab
	adjspb	$-4
	cmpqd	0, -4(r7)
	beq	.L612
	jsr	_nextlab
	movd	_thisctl, r1
	movd	r0, 20(r1)
	movd	20(r1), tos
	movqd	0, tos
	movd	-4(r7), tos
	movqd	6, tos
	addr	20, tos
	jsr	_mknode
	adjspb	$-16
	movd	r0, tos
	jsr	_exifgo
	adjspb	$-8
	br	.L613
.L612:
	jsr	_exnull
.L613:
	cmpqd	0, _dbgflag
	beq	.L614
	movd	$.L615, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L614:
	br	.L250
.L616:
	movd	-4(r7), tos
	movqd	1, tos
	jsr	_simple
	adjspb	$-8
	movd	r0, tos
	addr	8, tos
	jsr	_pushctl
	adjspb	$-8
	cmpqd	0, _dbgflag
	beq	.L617
	movd	$.L618, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L617:
	br	.L250
.L619:
	movd	0(r7), tos
	movqd	7, tos
	jsr	_pushctl
	adjspb	$-8
	cmpqd	0, _dbgflag
	beq	.L620
	movd	$.L621, tos
	movd	_diagfile, tos
	jsr	_fprintf
	adjspb	$-8
.L620:
	br	.L250
.L622:
	movqd	0, tos
	movqd	0, tos
	movd	-8(r7), tos
	jsr	_doloop
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L623:
	movqd	0, tos
	movqd	0, tos
	movd	-4(r7), tos
	jsr	_doloop
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L624:
	movqd	0, tos
	movd	-8(r7), tos
	movd	-16(r7), tos
	jsr	_doloop
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L625:
	movqd	0, tos
	movd	-4(r7), tos
	movd	-12(r7), tos
	jsr	_doloop
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L626:
	movd	-8(r7), tos
	movd	-16(r7), tos
	movd	-24(r7), tos
	jsr	_doloop
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L627:
	movd	-4(r7), tos
	movd	-12(r7), tos
	movd	-20(r7), tos
	jsr	_doloop
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L628:
	movqd	0, tos
	movqd	6, tos
	jsr	_pushctl
	adjspb	$-8
	br	.L250
.L629:
	movd	_thisctl, r0
	movd	24(r0), tos
	jsr	_exgoto
	adjspb	$-4
	movd	_thisctl, r0
	movd	16(r0), tos
	jsr	_exlab
	adjspb	$-4
	jsr	_addexec
	br	.L250
.L630:
	movd	_thisctl, r0
	movd	24(r0), tos
	jsr	_exgoto
	adjspb	$-4
	movd	_thisctl, r0
	movd	16(r0), tos
	jsr	_exlab
	adjspb	$-4
	jsr	_addexec
	br	.L250
.L631:
	jsr	_addexec
	br	.L250
.L632:
	jsr	_addexec
	br	.L250
.L633:
	movqd	0, _yyval
	br	.L250
.L634:
	cmpqd	0, _dbgopt
	beq	.L635
	addqd	1, _dbglevel
.L635:
	br	.L250
.L636:
	cmpqd	0, _dbgopt
	beq	.L637
	addqd	-1, _dbglevel
.L637:
	br	.L250
.L638:
	movd	-4(r7), tos
	movd	-12(r7), tos
	movd	-20(r7), tos
	jsr	_mkiost
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L639:
	movqd	0, _yyval
	br	.L250
.L640:
	movqd	1, _yyval
	br	.L250
.L641:
	movqd	2, _yyval
	br	.L250
.L642:
	movqd	3, _yyval
	br	.L250
.L643:
	movqd	0, _yyval
	br	.L250
.L644:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L645:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkchain
	adjspb	$-8
	movd	r0, tos
	movd	-8(r7), tos
	jsr	_hookup
	adjspb	$-8
	br	.L250
.L646:
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkioitem
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L647:
	movd	0(r7), tos
	movd	-8(r7), tos
	jsr	_mkioitem
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L648:
	movd	0(r7), tos
	movqd	0, tos
	jsr	_mkioitem
	adjspb	$-8
	movd	r0, _yyval
	br	.L250
.L649:
	movqd	0, tos
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkiogroup
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L650:
	movd	-4(r7), tos
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkiogroup
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L651:
	movd	-12(r7), tos
	movd	0(r7), tos
	movd	-8(r7), tos
	jsr	_mkiogroup
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L652:
	movqd	0, tos
	movd	0(r7), tos
	movd	-8(r7), tos
	jsr	_mkiogroup
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L653:
	addqd	1, _iobrlevel
	br	.L250
.L654:
	addqd	-1, _iobrlevel
	movd	-4(r7), _yyval
	br	.L250
.L655:
	movqd	0, tos
	movqd	0, tos
	movd	0(r7), tos
	jsr	_mkformat
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L656:
	movqd	0, tos
	movd	-4(r7), tos
	movd	-12(r7), tos
	jsr	_mkformat
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L657:
	movd	-4(r7), tos
	movd	-12(r7), tos
	movd	-20(r7), tos
	jsr	_mkformat
	adjspb	$-12
	movd	r0, _yyval
	br	.L250
.L658:
	movd	-4(r7), r0
	movd	4(r0), _yyval
	movd	-4(r7), tos
	jsr	_frexpblock
	adjspb	$-4
	br	.L250
.L660:
	movd	-4(r7), _yyval
	br	.L250
.L661:
	movqd	1, _lettneed
	br	.L250
.L662:
	movqd	0, _lettneed
	br	.L250
.L251:
.data
.L665:	.double	283, 1
.text
	checkd	r0, .L665, r0
	bfs	.L664
.L663:	cased .L666[r0:d]
.L666:
.double	.L252-.L663
.double	.L253-.L663
.double	.L254-.L663
.double	.L257-.L663
.double	.L259-.L663
.double	.L260-.L663
.double	.L261-.L663
.double	.L266-.L663
.double	.L267-.L663
.double	.L274-.L663
.double	.L278-.L663
.double	.L279-.L663
.double	.L283-.L663
.double	.L284-.L663
.double	.L285-.L663
.double	.L286-.L663
.double	.L289-.L663
.double	.L290-.L663
.double	.L291-.L663
.double	.L302-.L663
.double	.L304-.L663
.double	.L305-.L663
.double	.L307-.L663
.double	.L308-.L663
.double	.L309-.L663
.double	.L310-.L663
.double	.L311-.L663
.double	.L315-.L663
.double	.L316-.L663
.double	.L664-.L663
.double	.L664-.L663
.double	.L664-.L663
.double	.L322-.L663
.double	.L325-.L663
.double	.L326-.L663
.double	.L327-.L663
.double	.L328-.L663
.double	.L334-.L663
.double	.L664-.L663
.double	.L664-.L663
.double	.L335-.L663
.double	.L664-.L663
.double	.L664-.L663
.double	.L336-.L663
.double	.L337-.L663
.double	.L338-.L663
.double	.L339-.L663
.double	.L340-.L663
.double	.L341-.L663
.double	.L664-.L663
.double	.L664-.L663
.double	.L343-.L663
.double	.L664-.L663
.double	.L664-.L663
.double	.L344-.L663
.double	.L345-.L663
.double	.L664-.L663
.double	.L664-.L663
.double	.L348-.L663
.double	.L664-.L663
.double	.L350-.L663
.double	.L664-.L663
.double	.L664-.L663
.double	.L351-.L663
.double	.L352-.L663
.double	.L664-.L663
.double	.L664-.L663
.double	.L353-.L663
.double	.L354-.L663
.double	.L355-.L663
.double	.L664-.L663
.double	.L356-.L663
.double	.L358-.L663
.double	.L359-.L663
.double	.L360-.L663
.double	.L664-.L663
.double	.L361-.L663
.double	.L362-.L663
.double	.L664-.L663
.double	.L363-.L663
.double	.L364-.L663
.double	.L365-.L663
.double	.L664-.L663
.double	.L366-.L663
.double	.L367-.L663
.double	.L664-.L663
.double	.L664-.L663
.double	.L368-.L663
.double	.L369-.L663
.double	.L376-.L663
.double	.L377-.L663
.double	.L387-.L663
.double	.L664-.L663
.double	.L388-.L663
.double	.L664-.L663
.double	.L389-.L663
.double	.L390-.L663
.double	.L392-.L663
.double	.L393-.L663
.double	.L394-.L663
.double	.L395-.L663
.double	.L396-.L663
.double	.L397-.L663
.double	.L398-.L663
.double	.L400-.L663
.double	.L401-.L663
.double	.L402-.L663
.double	.L404-.L663
.double	.L405-.L663
.double	.L406-.L663
.double	.L407-.L663
.double	.L408-.L663
.double	.L664-.L663
.double	.L410-.L663
.double	.L411-.L663
.double	.L412-.L663
.double	.L413-.L663
.double	.L414-.L663
.double	.L415-.L663
.double	.L416-.L663
.double	.L417-.L663
.double	.L418-.L663
.double	.L419-.L663
.double	.L422-.L663
.double	.L427-.L663
.double	.L428-.L663
.double	.L429-.L663
.double	.L664-.L663
.double	.L664-.L663
.double	.L430-.L663
.double	.L431-.L663
.double	.L432-.L663
.double	.L664-.L663
.double	.L440-.L663
.double	.L443-.L663
.double	.L664-.L663
.double	.L664-.L663
.double	.L444-.L663
.double	.L447-.L663
.double	.L448-.L663
.double	.L449-.L663
.double	.L450-.L663
.double	.L453-.L663
.double	.L454-.L663
.double	.L455-.L663
.double	.L456-.L663
.double	.L457-.L663
.double	.L458-.L663
.double	.L463-.L663
.double	.L664-.L663
.double	.L464-.L663
.double	.L465-.L663
.double	.L664-.L663
.double	.L467-.L663
.double	.L472-.L663
.double	.L473-.L663
.double	.L475-.L663
.double	.L476-.L663
.double	.L479-.L663
.double	.L480-.L663
.double	.L481-.L663
.double	.L482-.L663
.double	.L483-.L663
.double	.L484-.L663
.double	.L485-.L663
.double	.L487-.L663
.double	.L488-.L663
.double	.L489-.L663
.double	.L490-.L663
.double	.L664-.L663
.double	.L491-.L663
.double	.L664-.L663
.double	.L664-.L663
.double	.L492-.L663
.double	.L495-.L663
.double	.L497-.L663
.double	.L502-.L663
.double	.L503-.L663
.double	.L505-.L663
.double	.L664-.L663
.double	.L506-.L663
.double	.L509-.L663
.double	.L510-.L663
.double	.L511-.L663
.double	.L512-.L663
.double	.L514-.L663
.double	.L515-.L663
.double	.L517-.L663
.double	.L520-.L663
.double	.L523-.L663
.double	.L524-.L663
.double	.L527-.L663
.double	.L529-.L663
.double	.L530-.L663
.double	.L531-.L663
.double	.L534-.L663
.double	.L664-.L663
.double	.L535-.L663
.double	.L536-.L663
.double	.L537-.L663
.double	.L542-.L663
.double	.L547-.L663
.double	.L550-.L663
.double	.L553-.L663
.double	.L555-.L663
.double	.L560-.L663
.double	.L561-.L663
.double	.L564-.L663
.double	.L568-.L663
.double	.L570-.L663
.double	.L664-.L663
.double	.L571-.L663
.double	.L572-.L663
.double	.L578-.L663
.double	.L580-.L663
.double	.L585-.L663
.double	.L586-.L663
.double	.L587-.L663
.double	.L664-.L663
.double	.L588-.L663
.double	.L589-.L663
.double	.L590-.L663
.double	.L591-.L663
.double	.L592-.L663
.double	.L593-.L663
.double	.L595-.L663
.double	.L596-.L663
.double	.L597-.L663
.double	.L598-.L663
.double	.L599-.L663
.double	.L600-.L663
.double	.L601-.L663
.double	.L602-.L663
.double	.L603-.L663
.double	.L604-.L663
.double	.L605-.L663
.double	.L606-.L663
.double	.L607-.L663
.double	.L608-.L663
.double	.L611-.L663
.double	.L616-.L663
.double	.L619-.L663
.double	.L622-.L663
.double	.L623-.L663
.double	.L624-.L663
.double	.L625-.L663
.double	.L626-.L663
.double	.L627-.L663
.double	.L628-.L663
.double	.L629-.L663
.double	.L630-.L663
.double	.L664-.L663
.double	.L631-.L663
.double	.L632-.L663
.double	.L633-.L663
.double	.L664-.L663
.double	.L634-.L663
.double	.L636-.L663
.double	.L638-.L663
.double	.L639-.L663
.double	.L640-.L663
.double	.L641-.L663
.double	.L642-.L663
.double	.L664-.L663
.double	.L643-.L663
.double	.L644-.L663
.double	.L645-.L663
.double	.L646-.L663
.double	.L647-.L663
.double	.L648-.L663
.double	.L649-.L663
.double	.L650-.L663
.double	.L651-.L663
.double	.L652-.L663
.double	.L653-.L663
.double	.L654-.L663
.double	.L655-.L663
.double	.L656-.L663
.double	.L657-.L663
.double	.L658-.L663
.double	.L660-.L663
.double	.L661-.L663
.double	.L662-.L663
.L664:
.L250:
	br	.L205
.L202:
	exit	[r4,r5,r6,r7]
	ret	0
	enter	[r4,r5,r6,r7], 8
.data
.L211:
	.byte	0x79,0x61,0x63,0x63,0x20,0x73,0x74,0x61
	.byte	0x63,0x6b,0x20,0x6f,0x76,0x65,0x72,0x66
	.byte	0x6c,0x6f,0x77,0x0
.L232:
	.byte	0x73,0x79,0x6e,0x74,0x61,0x78,0x20,0x65
	.byte	0x72,0x72,0x6f,0x72,0x0
.L263:
	.byte	0x53,0x79,0x6e,0x74,0x61,0x78,0x20,0x65
	.byte	0x72,0x72,0x6f,0x72,0x0
.L264:
	.byte	0x0
.L265:
	.byte	0x0
.L270:
	.byte	0x64,0x65,0x63,0x6c,0x61,0x72,0x61,0x74
	.byte	0x69,0x6f,0x6e,0x20,0x61,0x6d,0x69,0x64
	.byte	0x20,0x65,0x78,0x65,0x63,0x75,0x74,0x61
	.byte	0x62,0x6c,0x65,0x73,0x0
.L273:
	.byte	0x73,0x74,0x61,0x74,0x3a,0x20,0x64,0x63
	.byte	0x6c,0xa,0x0
.L277:
	.byte	0x73,0x74,0x61,0x74,0x3a,0x20,0x65,0x78
	.byte	0x65,0x63,0xa,0x0
.L280:
	.byte	0x53,0x79,0x6e,0x74,0x61,0x78,0x20,0x65
	.byte	0x72,0x72,0x6f,0x72,0x0
.L281:
	.byte	0x0
.L282:
	.byte	0x0
.L295:
	.byte	0x61,0x74,0x74,0x72,0x69,0x62,0x75,0x74
	.byte	0x65,0x73,0x20,0x6f,0x6e,0x20,0x75,0x6e
	.byte	0x6e,0x61,0x6d,0x65,0x64,0x20,0x70,0x72
	.byte	0x6f,0x63,0x65,0x64,0x75,0x72,0x65,0x0
.L296:
	.byte	0x0
.L299:
	.byte	0x50,0x72,0x6f,0x63,0x65,0x64,0x75,0x72
	.byte	0x65,0x20,0x25,0x73,0x3a,0xa,0x0
.L301:
	.byte	0x20,0x20,0x20,0x20,0x50,0x61,0x73,0x73
	.byte	0x20,0x31,0xa,0x0
.L314:
	.byte	0x61,0x72,0x67,0x75,0x6d,0x65,0x6e,0x74
	.byte	0x20,0x61,0x6c,0x72,0x65,0x61,0x64,0x79
	.byte	0x20,0x75,0x73,0x65,0x64,0x0
.L319:
	.byte	0x4f,0x70,0x74,0x69,0x6f,0x6e,0x20,0x73
	.byte	0x74,0x61,0x74,0x65,0x6d,0x65,0x6e,0x74
	.byte	0x20,0x69,0x6e,0x73,0x69,0x64,0x65,0x20
	.byte	0x70,0x72,0x6f,0x63,0x65,0x64,0x75,0x72
	.byte	0x65,0x0
.L320:
	.byte	0x0
.L321:
	.byte	0x70,0x72,0x6f,0x63,0x65,0x64,0x75,0x72
	.byte	0x65,0x20,0x25,0x73,0x20,0x74,0x65,0x72
	.byte	0x6d,0x69,0x6e,0x61,0x74,0x65,0x64,0x20
	.byte	0x70,0x72,0x65,0x6d,0x61,0x74,0x75,0x72
	.byte	0x65,0x6c,0x79,0x0
.L330:
	.byte	0x63,0x6f,0x6e,0x74,0x72,0x6f,0x6c,0x20
	.byte	0x73,0x74,0x61,0x63,0x6b,0x20,0x6e,0x6f
	.byte	0x74,0x20,0x65,0x6d,0x70,0x74,0x79,0x20
	.byte	0x75,0x70,0x6f,0x6e,0x20,0x45,0x4e,0x44
	.byte	0x0
.L331:
	.byte	0x0
.L375:
	.byte	0x6d,0x75,0x6c,0x74,0x69,0x70,0x6c,0x65
	.byte	0x20,0x64,0x69,0x6d,0x65,0x6e,0x73,0x69
	.byte	0x6f,0x6e,0x0
.L379:
	.byte	0x69,0x6c,0x6c,0x65,0x67,0x61,0x6c,0x20
	.byte	0x69,0x6e,0x69,0x74,0x69,0x61,0x6c,0x69
	.byte	0x7a,0x61,0x74,0x69,0x6f,0x6e,0x20,0x6f
	.byte	0x70,0x65,0x72,0x61,0x74,0x6f,0x72,0x0
.L384:
	.byte	0x6d,0x75,0x6c,0x74,0x69,0x70,0x6c,0x65
	.byte	0x20,0x64,0x69,0x6d,0x65,0x6e,0x73,0x69
	.byte	0x6f,0x6e,0x0
.L386:
	.byte	0x6d,0x75,0x6c,0x74,0x69,0x70,0x6c,0x65
	.byte	0x20,0x69,0x6e,0x69,0x74,0x69,0x61,0x6c
	.byte	0x69,0x7a,0x61,0x74,0x69,0x6f,0x6e,0x0
.L399:
	.byte	0x41,0x55,0x54,0x4f,0x4d,0x41,0x54,0x49
	.byte	0x43,0x20,0x6e,0x6f,0x74,0x20,0x79,0x65
	.byte	0x74,0x20,0x69,0x6d,0x70,0x6c,0x65,0x6d
	.byte	0x65,0x6e,0x74,0x65,0x64,0xa,0x0
.L403:
	.byte	0x56,0x41,0x4c,0x55,0x45,0x20,0x6e,0x6f
	.byte	0x74,0x20,0x79,0x65,0x74,0x20,0x69,0x6d
	.byte	0x70,0x6c,0x65,0x6d,0x65,0x6e,0x74,0x65
	.byte	0x64,0xa,0x0
.L409:
	.byte	0x0
.L426:
	.byte	0x6d,0x75,0x6c,0x74,0x69,0x70,0x6c,0x65
	.byte	0x20,0x64,0x65,0x63,0x6c,0x61,0x72,0x61
	.byte	0x74,0x69,0x6f,0x6e,0x20,0x66,0x6f,0x72
	.byte	0x20,0x74,0x79,0x70,0x65,0x20,0x25,0x73
	.byte	0x0
.L441:
	.byte	0x2e,0x74,0x72,0x75,0x65,0x2e,0x0
.L442:
	.byte	0x2e,0x66,0x61,0x6c,0x73,0x65,0x2e,0x0
.L461:
	.byte	0x6d,0x61,0x79,0x20,0x6e,0x6f,0x74,0x20
	.byte	0x61,0x73,0x73,0x69,0x67,0x6e,0x20,0x74
	.byte	0x6f,0x20,0x61,0x20,0x66,0x75,0x6e,0x63
	.byte	0x74,0x69,0x6f,0x6e,0x0
.L494:
	.byte	0x65,0x78,0x65,0x63,0x20,0x64,0x6f,0x6e
	.byte	0x65,0xa,0x0
.L499:
	.byte	0x6e,0x6f,0x20,0x73,0x74,0x61,0x74,0x65
	.byte	0x6d,0x65,0x6e,0x74,0x20,0x66,0x75,0x6e
	.byte	0x63,0x74,0x69,0x6f,0x6e,0x73,0x20,0x69
	.byte	0x6e,0x20,0x45,0x46,0x4c,0x0
.L508:
	.byte	0x65,0x78,0x65,0x63,0x3a,0x20,0x7b,0x20
	.byte	0x73,0x74,0x61,0x74,0x73,0x20,0x7d,0xa
	.byte	0x0
.L519:
	.byte	0x65,0x78,0x65,0x63,0x3a,0x65,0x6d,0x70
	.byte	0x74,0x79,0xa,0x0
.L522:
	.byte	0x65,0x78,0x65,0x63,0x3a,0x20,0x63,0x6f
	.byte	0x6e,0x74,0x69,0x6e,0x75,0x65,0xa,0x0
.L539:
	.byte	0x69,0x66,0x2d,0x74,0x68,0x65,0x6e,0x2d
	.byte	0x65,0x6c,0x73,0x65,0xa,0x0
.L541:
	.byte	0x65,0x78,0x65,0x63,0x3a,0x20,0x69,0x66
	.byte	0x28,0x65,0x78,0x70,0x72,0x29,0x20,0x65
	.byte	0x78,0x65,0x63,0x20,0x65,0x6c,0x73,0x65
	.byte	0x20,0x65,0x78,0x65,0x63,0xa,0x0
.L544:
	.byte	0x69,0x66,0x2d,0x74,0x68,0x65,0x6e,0xa
	.byte	0x0
.L546:
	.byte	0x65,0x78,0x65,0x63,0x3a,0x20,0x69,0x66
	.byte	0x28,0x65,0x78,0x70,0x72,0x29,0x20,0x65
	.byte	0x78,0x65,0x63,0xa,0x0
.L549:
	.byte	0x72,0x65,0x70,0x65,0x61,0x74,0x20,0x64
	.byte	0x6f,0x6e,0x65,0xa,0x0
.L552:
	.byte	0x65,0x78,0x65,0x63,0x3a,0x20,0x63,0x6f
	.byte	0x6e,0x74,0x72,0x6f,0x6c,0x20,0x65,0x78
	.byte	0x65,0x63,0xa,0x0
.L563:
	.byte	0x20,0x6e,0x6f,0x20,0x75,0x6e,0x74,0x69
	.byte	0x6c,0xa,0x0
.L567:
	.byte	0x75,0x6e,0x74,0x69,0x6c,0xa,0x0
.L574:
	.byte	0x74,0x6f,0x0
.L576:
	.byte	0x67,0x6f,0x20,0x25,0x73,0x20,0x3f,0x3f
	.byte	0xa,0x0
.L581:
	.byte	0x74,0x6f,0x0
.L583:
	.byte	0x67,0x6f,0x20,0x25,0x73,0x20,0x3f,0x3f
	.byte	0xa,0x0
.L610:
	.byte	0x77,0x68,0x69,0x6c,0x65,0x28,0x65,0x78
	.byte	0x70,0x72,0x29,0xa,0x0
.L615:
	.byte	0x66,0x6f,0x72,0x20,0x28,0x66,0x6f,0x72
	.byte	0x6c,0x69,0x73,0x74,0x29,0xa,0x0
.L618:
	.byte	0x73,0x77,0x69,0x74,0x63,0x68,0x20,0x28
	.byte	0x65,0x78,0x70,0x72,0x29,0xa,0x0
.L621:
	.byte	0x64,0x6f,0x20,0x6c,0x6f,0x6f,0x70,0xa
	.byte	0x0
