/*	@(#)tabtn300.c	1.4	*/
#define INCH 240
/*
Terminet300
nroff driving tables
width and code tables
*/

struct termtable ttn300 = {
/*bset*/	0,
/*breset*/	0,
/*Hor*/		INCH/10,
/*Vert*/	INCH/6,
/*Newline*/	INCH/6,
/*Char*/	INCH/10,
/*Em*/		INCH/10,
/*Halfline*/	INCH/12,
/*Adj*/		INCH/10,
/*twinit*/	"",
/*twrest*/	"",
/*twnl*/	"\n",
/*hlr*/		"",
/*hlf*/		"",
/*flr*/		"",
/*bdon*/	"",
/*bdoff*/	"",
/*iton*/	"",
/*itoff*/	"",
/*ploton*/	"",
/*plotoff*/	"",
/*up*/		"",
/*down*/	"",
/*right*/	"",
/*left*/	"",
/*codetab*/
"\001 ",	/*space*/
"\001!",	/*!*/
"\001\"",	/*"*/
"\001#",	/*#*/
"\001$",	/*$*/
"\001%",	/*%*/
"\001&",	/*&*/
"\001'",	/*' close*/
"\001(",	/*(*/
"\001)",	/*)*/
"\001*",	/***/
"\001+",	/*+*/
"\001,",	/*,*/
"\001-",	/*- hyphen*/
"\001.",	/*.*/
"\001/",	/*/*/
"\2010",	/*0*/
"\2011",	/*1*/
"\2012",	/*2*/
"\2013",	/*3*/
"\2014",	/*4*/
"\2015",	/*5*/
"\2016",	/*6*/
"\2017",	/*7*/
"\2018",	/*8*/
"\2019",	/*9*/
"\001:",	/*:*/
"\001;",	/*;*/
"\001<",	/*<*/
"\001=",	/*=*/
"\001>",	/*>*/
"\001?",	/*?*/
"\001@",	/*@*/
"\201A",	/*A*/
"\201B",	/*B*/
"\201C",	/*C*/
"\201D",	/*D*/
"\201E",	/*E*/
"\201F",	/*F*/
"\201G",	/*G*/
"\201H",	/*H*/
"\201I",	/*I*/
"\201J",	/*J*/
"\201K",	/*K*/
"\201L",	/*L*/
"\201M",	/*M*/
"\201N",	/*N*/
"\201O",	/*O*/
"\201P",	/*P*/
"\201Q",	/*Q*/
"\201R",	/*R*/
"\201S",	/*S*/
"\201T",	/*T*/
"\201U",	/*U*/
"\201V",	/*V*/
"\201W",	/*W*/
"\201X",	/*X*/
"\201Y",	/*Y*/
"\201Z",	/*Z*/
"\001[",	/*[*/
"\001\\",	/*\*/
"\001]",	/*]*/
"\001^",	/*^*/
"\001_",	/*_ dash*/
"\001`",	/*` open*/
"\201a",	/*a*/
"\201b",	/*b*/
"\201c",	/*c*/
"\201d",	/*d*/
"\201e",	/*e*/
"\201f",	/*f*/
"\201g",	/*g*/
"\201h",	/*h*/
"\201i",	/*i*/
"\201j",	/*j*/
"\201k",	/*k*/
"\201l",	/*l*/
"\201m",	/*m*/
"\201n",	/*n*/
"\201o",	/*o*/
"\201p",	/*p*/
"\201q",	/*q*/
"\201r",	/*r*/
"\201s",	/*s*/
"\201t",	/*t*/
"\201u",	/*u*/
"\201v",	/*v*/
"\201w",	/*w*/
"\201x",	/*x*/
"\201y",	/*y*/
"\201z",	/*z*/
"\001{",	/*{*/
"\001|",	/*|*/
"\001}",	/*}*/
"\001~",	/*~*/
"\000\0",	/*nar sp*/
"\001-",	 /*hyphen*/
"\001+\bo",	 /*bullet*/
"\002\[]",	 /*square*/
"\001-",	 /*3/4 em*/
"\001_",	 /*rule*/
"\0031/4",	 /*1/4*/
"\0031/2",	 /*1/2*/
"\0033/4",	 /*3/4*/
"\001-",	 /*minus*/
"\202fi",	 /*fi*/
"\202fl",	 /*fl*/
"\202ff",	 /*ff*/
"\203ffi",	 /*ffi*/
"\203ffl",	 /*ffl*/
"\001'\b`",	 /*degree*/
"\001|\b-",	 /*dagger*/
"\001j\bf",	 /*section*/
"\001'",	 /*foot mark*/
"\001'",	 /*acute accent*/
"\001`",	 /*grave accent*/
"\001_",	 /*underrule*/
"\001/",	 /*slash (longer)*/
"\000\0",	/*half narrow space*/
"\001 ",	/*unpaddable space*/
"\201<\ba", /*alpha*/
"\201,\bB", /*beta*/
"\201,\by", /*gamma*/
"\201>\bo", /*delta*/
"\201-\bc", /*epsilon*/
"\201,\bL", /*zeta*/
"\201,\bn", /*eta*/
"\201-\b0", /*theta*/
"\201,\bi",	 /*iota*/
"\201k",	 /*kappa*/
"\201\\\b>", /*lambda*/
"\201,\bu", /*mu*/
"\201,\bv", /*nu*/
"\201,\b3", /*xi*/
"\201o",	 /*omicron*/
"\201-\bn", /*pi*/
"\201p", /*rho*/
"\201-\bo", /*sigma*/
"\201~\bt", /*tau*/
"\201u",	 /*upsilon*/
"\201/\bo", /*phi*/
"\201x",	 /*chi*/
"\201|\bu", /*psi*/
"\201u\bw", /*omega*/
"\202~\b|~", /*Gamma*/
"\202_\b/_\b\\", /*Delta*/
"\-\bO", /*Theta*/
"\202/\\", /*Lambda*/
"\201_\b-\b~",	 /*Xi*/
"\202~\b|~\b|", /*Pi*/
"\201~\b_\b>", /*Sigma*/
"\000\0",	 /**/
"\201Y",	 /*Upsilon*/
"\201|\bO", /*Phi*/
"\|\bU", /*Psi*/
"\202_\b(_\b)", /*Omega*/
"002\\\/",	 /*square root*/
"\201s",	 /*terminal sigma*/
"\001~",	 /*root en*/
"\001_\b>",	 /*>=*/
"\001_\b<",	 /*<=*/
"\001_\b=",	 /*identically equal*/
"\001-",	 /*equation minus*/
"\001~\b=",	 /*approx =*/
"\001~",	 /*approximates*/
"\001=\b/",	 /*not equal*/
"\002->",	 /*right arrow*/
"\002<-",	 /*left arrow*/
"\001|\b^",	 /*up arrow*/
"\001|\bv",	 /*down arrow*/
"\001=",	 /*equation equal*/
"\001x",	 /*multiply*/
"\001-\b:",	 /*divide*/
"\001+\b_",	 /*plus-minus*/
"\001U",	 /*cup (union)*/
"\003(^)",	 /*cap (intersection)*/
"\002(_\b~",	 /*subset of*/
"\002_\b~)",	 /*superset of*/
"\002(~\b_\b=",	 /*improper subset*/
"\002~\b_\b=)",	 /* improper superset*/
"\002oo",	 /*infinity*/
"\0016", /*partial derivative*/
"\001~\bV", /*gradient*/
"\001-", /*not*/
"\001'\b,\bI", /*integral sign*/
"\002oc",	 /*proportional to*/
"\001/\bO",	 /*empty set*/
"\001-\bC",	 /*member of*/
"\001+",	 /*equation plus*/
"\001r\bO",	 /*registered*/
"\001c\bO",	 /*copyright*/
"\001|",	 /*box rule */
"\001c\b/",	 /*cent sign*/
"\001|\b=",	 /*dbl dagger*/
"\002=>",	 /*right hand*/
"\002<=",	 /*left hand*/
"\001*",	 /*math * */
"\000\0",	 /*bell system sign*/
"\001|",	 /*or (was star)*/
"\001O",	 /*circle*/
"\001(",	 /*left top (of big curly)*/
"\001(",	 /*left bottom*/
"\001)",	 /*right top*/
"\001)",	 /*right bot*/
"\001|",	 /*left center of big curly bracket*/
"\001|",	 /*right center of big curly bracket*/
"\001|",	/*bold vertical*/
"\001|",	/*left floor (left bot of big sq bract)*/
"\001|",	/*right floor (rb of ")*/
"\001|",	/*left ceiling (lt of ")*/
"\001|"};	/*right ceiling (rt of ")*/
