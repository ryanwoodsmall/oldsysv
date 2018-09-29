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
static char	*prtitle[NUMFLAGS] = {
	"\n\n\t\t\t***ARCHIVE HEADER***\n\tMember Name        Date       Uid     Gid     Mode       Size\n\n",
/*
	12345678901234  0x12345678  123  123  0123456  0x12345678
	Member Name        Date     Uid  Gid   Mode       Size
*/

	"\n\n\t\t\t***FILE HEADER***\n\t Magic   Nscns  Time/Date     Symptr      Nsyms     Opthdr  Flags\n\n",
/*
	0123456  12345  0x12345678  0x12345678  1234567890  0x1234  0x1234
	 Magic   Nscns  Time/Date     Symptr      Nsyms     Opthdr  Flags
*/

	"\n\n\t\t\t***OPTIONAL HEADER in HEX***\n\n",

	"\n\n\t\t\t***SECTION HEADER***\n\tName        Paddr       Vaddr       Scnptr      Relptr     Lnnoptr\n\t            Flags                    Size       Nreloc      Nlnno\n\n",
/*
	12345678  0x12345678  0x12345678  0x12345678  0x12345678  0x12345678
		  0x12345678	 	  0x12345678    12345       12345
	Name        Paddr       Vaddr       Scnptr      Relptr     Lnnoptr
		    Flags		     Size       Nreloc      Nlnno
*/


	"\n\n\t\t\t***SECTION DATA in HEX***\n",

	"\n\n\t\t\t***RELOCATION INFORMATION***\n\t  Vaddr       Symndx     Type\n\n",
/*
	0x12345678  1234567890  123456
	  Vaddr       Symndx    Type
*/

	"\n\n\t\t\t***LINE NUMBER INFORMATION***\n\tSymndx/Paddr  Lnno\n\n",
/*
	 0x12345678   12345
	Symndx/Paddr  Lnno
*/

	"\n\n\t\t\t***LINE NUMBERS FOR FUNCTION***\n\tSymndx/Paddr  Lnno\n\n",
/*
	 0x12345678   12345
	Symndx/Paddr  Lnno
*/
#if FLEXNAMES
	"\n\n\t\t\t***SYMBOL TABLE INFORMATION***\n\
[Index]\tm1  Name        Value      Scnum   Type   Sclass  Numaux  Name\n\
[Index]\tm2  Offset      Value      Scnum   Type   Sclass  Numaux  Name\n\
[Index]\ta\t       Tagndx     Misc    Misc     Fcnary       Fcnary    Tvdnx\n\n",
#else
	"\n\n\t\t\t***SYMBOL TABLE INFORMATION***\n\
[Index]\tm1\tName        Value      Scnum   Type   Sclass  Numaux  Name\n\
[Index]\ta\t       Tagndx     Misc    Misc     Fcnary       Fcnary    Tvdnx\n\n",
#endif

#if FLEXNAMES
	"\n\n\t\t\t***STRING TABLE INFORMATION***\n\
\t[Offset]    Name\n\n"
#endif
};

static  char		*prvtitle[NUMFLAGS] = {
	"\n\n\t\t\t***ARCHIVE HEADER***\n\tMember Name        Date               Uid    Gid   Mode       Size\n\n",
/*
	12345678901234  0x12345678  123  123  0123456  0x12345678
	Member Name        Date     Uid  Gid   Mode       Size
*/

	"\n\n\t\t\t   ***FILE HEADER***\nMagic      Nscns     Time/Date          Symptr      Nsyms Opthdr  Flags\n\n",
/*
	0123456  12345  0x12345678  0x12345678  1234567890  0x1234  0x1234
	 Magic   Nscns  Time/Date     Symptr      Nsyms     Opthdr  Flags
*/

	"\n\n\t\t\t***OPTIONAL HEADER in HEX***\n\n",

	"\n\n\t\t\t***SECTION HEADER***\n\tName        Paddr       Vaddr       Scnptr      Relptr     Lnnoptr\n\t            Flags                    Size       Nreloc      Nlnno\n\n",
/*
	12345678  0x12345678  0x12345678  0x12345678  0x12345678  0x12345678
		  0x12345678	 	  0x12345678    12345       12345
	Name        Paddr       Vaddr       Scnptr      Relptr     Lnnoptr
		    Flags		     Size       Nreloc      Nlnno
*/


	"\n\n\t\t\t***SECTION DATA in HEX***\n",

	"\n\n\t\t\t***RELOCATION INFORMATION***\n\t  Vaddr       Symndx     Type     Name\n\n",
/*
	0x12345678  1234567890  123456
	  Vaddr       Symndx    Type     Name
*/

	"\n\n\t\t\t***LINE NUMBER INFORMATION***\n\tSymndx/Paddr  Lnno     Name\n\n",
/*
	 0x12345678   12345
	Symndx/Paddr  Lnno     Name
*/

	"\n\n\t\t\t***LINE NUMBERS FOR FUNCTION***\n\tSymndx/Paddr  Lnno\n\n",
/*
	 0x12345678   12345
	Symndx/Paddr  Lnno
*/
#if FLEXNAMES
	"\n\n\t\t\t***SYMBOL TABLE INFORMATION***\n\
[Index]\tm1  Name        Value      Scnum   Type   Sclass  Numaux  Name\n\
[Index]\tm2  Offset      Value      Scnum   Type   Sclass  Numaux  Name\n\n\
[Index]\ta1             Fname\n\
[Index]\ta2         Tvfill     Tvlen    Tvran[0]    Tvran[1]\n\
[Index]\ta3         Scnlen    Nreloc     Nlinno\n\
[Index]\ta4         Tagndx    Fsize         Lnnoptr    Endndx           Tvndx\n\
[Index]\ta5         Tagndx    Fsize  Dim[0]  Dim[1]  Dim[2]  Dim[3]    Tvndx\n\
[Index]\ta6         Tagndx    Lnno    Size    Lnnoptr   Endndx         Tvndx\n\
[Index]\ta7         Tagndx    Lnno    Size  Dim[0]  Dim[1]  Dim[2]  Dim[3]  Tvndx\n\n\n",
#else
	"\n\n\t\t\t***SYMBOL TABLE INFORMATION***\n\
[Index]\tm1\tName        Value      Scnum   Type   Sclass  Numaux\n\
[Index]\tm2\tOffset      Value      Scnum   Type   Sclass  Numaux\n\n\
[Index]\ta1             Fname\n\
[Index]\ta2         Tvfill     Tvlen    Tvran[0]    Tvran[1]\n\
[Index]\ta3         Scnlen    Nreloc     Nlinno\n\
[Index]\ta4         Tagndx    Fsize         Lnnoptr    Endndx           Tvndx\n\
[Index]\ta5         Tagndx    Fsize  Dim[0]  Dim[1]  Dim[2]  Dim[3]    Tvndx\n\
[Index]\ta6         Tagndx    Lnno    Size    Lnnoptr   Endndx         Tvndx\n\
[Index]\ta7         Tagndx    Lnno    Size  Dim[0]  Dim[1]  Dim[2]  Dim[3]  Tvndx\n\n\n",
#endif

#if FLEXNAMES
	"\n\n\t\t\t***STRING TABLE***\n\
\t[Offset]    Name\n\n"
#endif
};

#if PORTAR
static	char	*gtitle =
	"\n\n\t\t\t***ARCHIVE SYMBOL TABLE***\n\
\tOffset\tName\n\n\n";
#endif

/*
*	@(#) process.h: 1.2 2/24/83
*/
