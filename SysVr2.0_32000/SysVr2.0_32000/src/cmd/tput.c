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
/*	@(#)tput.c	1.2	*/

/* tput - print terminal attribute
/*
/* return codes:
/*	-1	usage error
/*	-2	bad terminal type given
/*	-3	unknown capname
/*	 0	ok (if boolean capname -> TRUE)
/*	 1	(for boolean capname  -> FALSE)
/*
/* tput printfs (a value) if an INT capname was given (e.g. cols),
/*	putp's (a string) if a STRING capname was given (e.g. clear), and
/*	for BOOLEAN capnames (e.g. hard-copy) just returns the boolean value. */

#include <curses.h>
#include <term.h>

main (argc,argv)
char **argv;
{
	int err;
	int xcode;
	char cap[40];
	
	if (!((argc == 2 && argv[1][0] != '-') ||
	 (argc == 3 && strncmp(argv[1],"-T",2) == 0 && strlen(argv[1]) > 2))) {
		fprintf(stderr,"tput: usage: \"tput [-Tterm] capname\"\n");
		exit(-1);
	}
	if (argc == 3) {
		strcpy(cap,argv[2]);
		setupterm(&argv[1][2],1,&err);
	}
	else {
		strcpy(cap,argv[1]);
		setupterm(0,1,&err);
	}
	if (err <= 0)
	{
	    if (argc == 3) {
		fprintf(stderr,"tput: unknown terminal \"%s\"\n",&argv[1][2]);
		exit(-2);
	    }
	    else {
		fprintf(stderr,"tput: unknown terminal \"%s\"\n",getenv("TERM"));
		exit(-2);
	    }
	}
	xcode = capindex(cap);
	resetterm();
	exit(xcode);
}



capindex (cap)
char *cap;
	{
	if (strcmp(cap,	"bw") == 0)	return(1-auto_left_margin);
	if (strcmp(cap,	"am") == 0)	return(1-auto_right_margin);
	if (strcmp(cap,	"xsb") == 0)	return(1-beehive_glitch);
	if (strcmp(cap,	"xhp") == 0)	return(1-ceol_standout_glitch);
	if (strcmp(cap,	"xenl") == 0)	return(1-eat_newline_glitch);
	if (strcmp(cap,	"eo") == 0)	return(1-erase_overstrike);
	if (strcmp(cap,	"gn") == 0)	return(1-generic_type);
	if (strcmp(cap,	"hc") == 0)	return(1-hard_copy);
	if (strcmp(cap,	"km") == 0)	return(1-has_meta_key);
	if (strcmp(cap,	"hs") == 0)	return(1-has_status_line);
	if (strcmp(cap,	"in") == 0)	return(1-insert_null_glitch);
	if (strcmp(cap,	"da") == 0)	return(1-memory_above);
	if (strcmp(cap,	"db") == 0)	return(1-memory_below);
	if (strcmp(cap,	"mir") == 0)	return(1-move_insert_mode);
	if (strcmp(cap,	"msgr") == 0)	return(1-move_standout_mode);
	if (strcmp(cap,	"os") == 0)	return(1-over_strike);
	if (strcmp(cap,	"eslok") == 0)	return(1-status_line_esc_ok);
	if (strcmp(cap,	"xt") == 0)	return(1-teleray_glitch);
	if (strcmp(cap,	"hz") == 0)	return(1-tilde_glitch);
	if (strcmp(cap,	"ul") == 0)	return(1-transparent_underline);
	if (strcmp(cap,	"xon") == 0)	return(1-xon_xoff);

	if (strcmp(cap,	"cols") == 0) 	{ printf("%d\n",columns); return(0); }
	if (strcmp(cap,	"it") == 0) 	{ printf("%d\n",init_tabs); return(0); }
	if (strcmp(cap,	"lines") == 0) 	{ printf("%d\n",lines); return(0); }
	if (strcmp(cap,	"lm") == 0) 	{ printf("%d\n",lines_of_memory); return(0); }
	if (strcmp(cap,	"xmc") == 0) { printf("%d\n",magic_cookie_glitch); return(0); }
	if (strcmp(cap,	"pb") == 0)  { printf("%d\n",padding_baud_rate); return(0); }
	if (strcmp(cap,	"vt") == 0) 	{ printf("%d\n",virtual_terminal); return(0); }
	if (strcmp(cap,	"wsl") == 0) { printf("%d\n",width_status_line); return(0); }

	if (strcmp(cap,	"cbt") == 0)	{ putp(back_tab);	return(0); }
	if (strcmp(cap,	"bel") == 0)	{ putp(bell);	return(0); }
	if (strcmp(cap,	"cr") == 0)	{ putp(carriage_return);	return(0); }
	if (strcmp(cap,	"csr") == 0)	{ putp(change_scroll_region);	return(0); }
	if (strcmp(cap,	"tbc") == 0)	{ putp(clear_all_tabs);	return(0); }
	if (strcmp(cap,	"clear") == 0)	{ putp(clear_screen);	return(0); }
	if (strcmp(cap,	"el") == 0)	{ putp(clr_eol);	return(0); }
	if (strcmp(cap,	"ed") == 0)	{ putp(clr_eos);	return(0); }
	if (strcmp(cap,	"hpa") == 0)	{ putp(column_address);	return(0); }
	if (strcmp(cap,	"cmdch") == 0)	{ putp(command_character);	return(0); }
	if (strcmp(cap,	"cup") == 0)	{ putp(cursor_address);	return(0); }
	if (strcmp(cap,	"cud1") == 0)	{ putp(cursor_down);	return(0); }
	if (strcmp(cap,	"home") == 0)	{ putp(cursor_home);	return(0); }
	if (strcmp(cap,	"civis") == 0)	{ putp(cursor_invisible);	return(0); }
	if (strcmp(cap,	"cub1") == 0)	{ putp(cursor_left);	return(0); }
	if (strcmp(cap,	"mrcup") == 0)	{ putp(cursor_mem_address);	return(0); }
	if (strcmp(cap,	"cnorm") == 0)	{ putp(cursor_normal);	return(0); }
	if (strcmp(cap,	"cuf1") == 0)	{ putp(cursor_right);	return(0); }
	if (strcmp(cap,	"ll") == 0)	{ putp(cursor_to_ll);	return(0); }
	if (strcmp(cap,	"cuu1") == 0)	{ putp(cursor_up);	return(0); }
	if (strcmp(cap,	"cvvis") == 0)	{ putp(cursor_visible);	return(0); }
	if (strcmp(cap,	"dch1") == 0)	{ putp(delete_character);	return(0); }
	if (strcmp(cap,	"dl1") == 0)	{ putp(delete_line);	return(0); }
	if (strcmp(cap,	"dsl") == 0)	{ putp(dis_status_line);	return(0); }
	if (strcmp(cap,	"hd") == 0)	{ putp(down_half_line);	return(0); }
	if (strcmp(cap,	"smacs") == 0)	{ putp(enter_alt_charset_mode);	return(0); }
	if (strcmp(cap,	"blink") == 0)	{ putp(enter_blink_mode);	return(0); }
	if (strcmp(cap,	"bold") == 0)	{ putp(enter_bold_mode);	return(0); }
	if (strcmp(cap,	"smcup") == 0)	{ putp(enter_ca_mode);	return(0); }
	if (strcmp(cap,	"smdc") == 0)	{ putp(enter_delete_mode);	return(0); }
	if (strcmp(cap,	"dim") == 0)	{ putp(enter_dim_mode);	return(0); }
	if (strcmp(cap,	"smir") == 0)	{ putp(enter_insert_mode);	return(0); }
	if (strcmp(cap,	"prot") == 0)	{ putp(enter_protected_mode);	return(0); }
	if (strcmp(cap,	"rev") == 0)	{ putp(enter_reverse_mode);	return(0); }
	if (strcmp(cap,	"invis") == 0)	{ putp(enter_secure_mode);	return(0); }
	if (strcmp(cap,	"smso") == 0)	{ putp(enter_standout_mode);	return(0); }
	if (strcmp(cap,	"smul") == 0)	{ putp(enter_underline_mode);	return(0); }
	if (strcmp(cap,	"ech") == 0)	{ putp(erase_chars);	return(0); }
	if (strcmp(cap,	"rmacs") == 0)	{ putp(exit_alt_charset_mode);	return(0); }
	if (strcmp(cap,	"sgr0") == 0)	{ putp(exit_attribute_mode);	return(0); }
	if (strcmp(cap,	"rmcup") == 0)	{ putp(exit_ca_mode);	return(0); }
	if (strcmp(cap,	"rmdc") == 0)	{ putp(exit_delete_mode);	return(0); }
	if (strcmp(cap,	"rmir") == 0)	{ putp(exit_insert_mode);	return(0); }
	if (strcmp(cap,	"rmso") == 0)	{ putp(exit_standout_mode);	return(0); }
	if (strcmp(cap,	"rmul") == 0)	{ putp(exit_underline_mode);	return(0); }
	if (strcmp(cap,	"flash") == 0)	{ putp(flash_screen);	return(0); }
	if (strcmp(cap,	"ff") == 0)	{ putp(form_feed);	return(0); }
	if (strcmp(cap,	"fsl") == 0)	{ putp(from_status_line);	return(0); }
	if (strcmp(cap,	"is1") == 0)	{ putp(init_1string);	return(0); }
	if (strcmp(cap,	"is2") == 0)	{ putp(init_2string);	return(0); }
	if (strcmp(cap,	"is3") == 0)	{ putp(init_3string);	return(0); }
	if (strcmp(cap,	"if") == 0)	{ putp(init_file);	return(0); }
	if (strcmp(cap,	"ich1") == 0)	{ putp(insert_character);	return(0); }
	if (strcmp(cap,	"il1") == 0)	{ putp(insert_line);	return(0); }
	if (strcmp(cap,	"ip") == 0)	{ putp(insert_padding);	return(0); }
	if (strcmp(cap,	"kbs") == 0)	{ putp(key_backspace);	return(0); }
	if (strcmp(cap,	"ktbc") == 0)	{ putp(key_catab);	return(0); }
	if (strcmp(cap,	"kclr") == 0)	{ putp(key_clear);	return(0); }
	if (strcmp(cap,	"kctab") == 0)	{ putp(key_ctab);	return(0); }
	if (strcmp(cap,	"kdch1") == 0)	{ putp(key_dc);	return(0); }
	if (strcmp(cap,	"kdl1") == 0)	{ putp(key_dl);	return(0); }
	if (strcmp(cap,	"kcud1") == 0)	{ putp(key_down);	return(0); }
	if (strcmp(cap,	"krmir") == 0)	{ putp(key_eic);	return(0); }
	if (strcmp(cap,	"kel") == 0)	{ putp(key_eol);	return(0); }
	if (strcmp(cap,	"ked") == 0)	{ putp(key_eos);	return(0); }
	if (strcmp(cap,	"kf0") == 0)	{ putp(key_f0);	return(0); }
	if (strcmp(cap,	"kf1") == 0)	{ putp(key_f1);	return(0); }
	if (strcmp(cap,	"kf10") == 0)	{ putp(key_f10);	return(0); }
	if (strcmp(cap,	"kf2") == 0)	{ putp(key_f2);	return(0); }
	if (strcmp(cap,	"kf3") == 0)	{ putp(key_f3);	return(0); }
	if (strcmp(cap,	"kf4") == 0)	{ putp(key_f4);	return(0); }
	if (strcmp(cap,	"kf5") == 0)	{ putp(key_f5);	return(0); }
	if (strcmp(cap,	"kf6") == 0)	{ putp(key_f6);	return(0); }
	if (strcmp(cap,	"kf7") == 0)	{ putp(key_f7);	return(0); }
	if (strcmp(cap,	"kf8") == 0)	{ putp(key_f8);	return(0); }
	if (strcmp(cap,	"kf9") == 0)	{ putp(key_f9);	return(0); }
	if (strcmp(cap,	"khome") == 0)	{ putp(key_home);	return(0); }
	if (strcmp(cap,	"kich1") == 0)	{ putp(key_ic);	return(0); }
	if (strcmp(cap,	"kil1") == 0)	{ putp(key_il);	return(0); }
	if (strcmp(cap,	"kcub1") == 0)	{ putp(key_left);	return(0); }
	if (strcmp(cap,	"kll") == 0)	{ putp(key_ll);	return(0); }
	if (strcmp(cap,	"knp") == 0)	{ putp(key_npage);	return(0); }
	if (strcmp(cap,	"kpp") == 0)	{ putp(key_ppage);	return(0); }
	if (strcmp(cap,	"kcuf1") == 0)	{ putp(key_right);	return(0); }
	if (strcmp(cap,	"kind") == 0)	{ putp(key_sf);	return(0); }
	if (strcmp(cap,	"kri") == 0)	{ putp(key_sr);	return(0); }
	if (strcmp(cap,	"khts") == 0)	{ putp(key_stab);	return(0); }
	if (strcmp(cap,	"kcuu1") == 0)	{ putp(key_up);	return(0); }
	if (strcmp(cap,	"rmkx") == 0)	{ putp(keypad_local);	return(0); }
	if (strcmp(cap,	"smkx") == 0)	{ putp(keypad_xmit);	return(0); }
	if (strcmp(cap,	"lf0") == 0)	{ putp(lab_f0);	return(0); }
	if (strcmp(cap,	"lf1") == 0)	{ putp(lab_f1);	return(0); }
	if (strcmp(cap,	"lf10") == 0)	{ putp(lab_f10);	return(0); }
	if (strcmp(cap,	"lf2") == 0)	{ putp(lab_f2);	return(0); }
	if (strcmp(cap,	"lf3") == 0)	{ putp(lab_f3);	return(0); }
	if (strcmp(cap,	"lf4") == 0)	{ putp(lab_f4);	return(0); }
	if (strcmp(cap,	"lf5") == 0)	{ putp(lab_f5);	return(0); }
	if (strcmp(cap,	"lf6") == 0)	{ putp(lab_f6);	return(0); }
	if (strcmp(cap,	"lf7") == 0)	{ putp(lab_f7);	return(0); }
	if (strcmp(cap,	"lf8") == 0)	{ putp(lab_f8);	return(0); }
	if (strcmp(cap,	"lf9") == 0)	{ putp(lab_f9);	return(0); }
	if (strcmp(cap,	"smm") == 0)	{ putp(meta_on);	return(0); }
	if (strcmp(cap,	"rmm") == 0)	{ putp(meta_off);	return(0); }
	if (strcmp(cap,	"nel") == 0)	{ putp(newline);	return(0); }
	if (strcmp(cap,	"pad") == 0)	{ putp(pad_char);	return(0); }
	if (strcmp(cap,	"dch") == 0)	{ putp(parm_dch);	return(0); }
	if (strcmp(cap,	"dl") == 0)	{ putp(parm_delete_line);	return(0); }
	if (strcmp(cap,	"cud") == 0)	{ putp(parm_down_cursor);	return(0); }
	if (strcmp(cap,	"ich") == 0)	{ putp(parm_ich);	return(0); }
	if (strcmp(cap,	"indn") == 0)	{ putp(parm_index);	return(0); }
	if (strcmp(cap,	"il") == 0)	{ putp(parm_insert_line);	return(0); }
	if (strcmp(cap,	"cub") == 0)	{ putp(parm_left_cursor);	return(0); }
	if (strcmp(cap,	"cuf") == 0)	{ putp(parm_right_cursor);	return(0); }
	if (strcmp(cap,	"rin") == 0)	{ putp(parm_rindex);	return(0); }
	if (strcmp(cap,	"cuu") == 0)	{ putp(parm_up_cursor);	return(0); }
	if (strcmp(cap,	"pfkey") == 0)	{ putp(pkey_key);	return(0); }
	if (strcmp(cap,	"pfloc") == 0)	{ putp(pkey_local);	return(0); }
	if (strcmp(cap,	"pfx") == 0)	{ putp(pkey_xmit);	return(0); }
	if (strcmp(cap,	"mc0") == 0)	{ putp(print_screen);	return(0); }
	if (strcmp(cap,	"mc4") == 0)	{ putp(prtr_off);	return(0); }
	if (strcmp(cap,	"mc5") == 0)	{ putp(prtr_on);	return(0); }
	if (strcmp(cap,	"rep") == 0)	{ putp(repeat_char);	return(0); }
	if (strcmp(cap,	"rs1") == 0)	{ putp(reset_1string);	return(0); }
	if (strcmp(cap,	"rs2") == 0)	{ putp(reset_2string);	return(0); }
	if (strcmp(cap,	"rs3") == 0)	{ putp(reset_3string);	return(0); }
	if (strcmp(cap,	"rf") == 0)	{ putp(reset_file);	return(0); }
	if (strcmp(cap,	"rc") == 0)	{ putp(restore_cursor);	return(0); }
	if (strcmp(cap,	"vpa") == 0)	{ putp(row_address);	return(0); }
	if (strcmp(cap,	"sc") == 0)	{ putp(save_cursor);	return(0); }
	if (strcmp(cap,	"ind") == 0)	{ putp(scroll_forward);	return(0); }
	if (strcmp(cap,	"ri") == 0)	{ putp(scroll_reverse);	return(0); }
	if (strcmp(cap,	"sgr") == 0)	{ putp(set_attributes);	return(0); }
	if (strcmp(cap,	"hts") == 0)	{ putp(set_tab);	return(0); }
	if (strcmp(cap,	"wind") == 0)	{ putp(set_window);	return(0); }
	if (strcmp(cap,	"ht") == 0)	{ putp(tab);	return(0); }
	if (strcmp(cap,	"tsl") == 0)	{ putp(to_status_line);	return(0); }
	if (strcmp(cap,	"uc") == 0)	{ putp(underline_char);	return(0); }
	if (strcmp(cap,	"hu") == 0)	{ putp(up_half_line);	return(0); }
	if (strcmp(cap,	"iprog") == 0)	{ putp(init_prog);	return(0); }
	if (strcmp(cap,	"ka1") == 0)	{ putp(key_a1);	return(0); }
	if (strcmp(cap,	"ka3") == 0)	{ putp(key_a3);	return(0); }
	if (strcmp(cap,	"kb2") == 0)	{ putp(key_b2);	return(0); }
	if (strcmp(cap,	"kc1") == 0)	{ putp(key_c1);	return(0); }
	if (strcmp(cap,	"kc3") == 0)	{ putp(key_c3);	return(0); }
	if (strcmp(cap,	"mc5p") == 0)	{ putp(prtr_non);	return(0); }

	fprintf(stderr,"tput: unknown capname %s\n",cap);	
	return(-3);	
}	
