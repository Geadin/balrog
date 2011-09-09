/***************************************************************************
 *  File: olc.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/



#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"
#include "tables.h"
#include "lookup.h"
#include "interp.h"


/*
 * Local functions.
 */
AREA_DATA *get_area_data args ((int vnum));

COMMAND( do_purge )

MOB_INDEX_DATA		xMob;
OBJ_INDEX_DATA		xObj;
ROOM_INDEX_DATA		xRoom;
struct	skill_type	xSkill;
struct	race_type	xRace;
MPROG_CODE		xProg;
struct	cmd_type	xCmd;
struct	social_type	xSoc;

const	struct	olc_comm_type	mob_olc_comm_table	[]	=
{
 { "name",	(void *) &xMob.player_name,	ed_line_string,		NULL		},
 { "short",	(void *) &xMob.short_descr,	ed_line_string,		NULL		},
 { "long",	(void *) &xMob.long_descr,	ed_line_string,		(void *) 1	},
 { "material",	(void *) &xMob.material,	ed_line_string,		NULL		},
 { "desc",	(void *) &xMob.description,	ed_desc,		NULL		},
 { "level",	(void *) &xMob.level,		ed_numero_niv,		NULL		},
 { "align",	(void *) &xMob.alignment,	ed_numero_align,	NULL		},
 { "group",	(void *) &xMob.group,		ed_olded,		medit_group	},
 { "imm",	(void *) &xMob.imm_flags,	ed_flag_toggle,		imm_flags	},
 { "res",	(void *) &xMob.res_flags,	ed_flag_toggle,		res_flags	},
 { "vuln",	(void *) &xMob.vuln_flags,	ed_flag_toggle,		vuln_flags	},
 { "act",	(void *) &xMob.act,		ed_flag_toggle,		act_flags	},
 { "affect",	(void *) &xMob.affected_by,	ed_flag_toggle,		affect_flags	},
 { "off",	(void *) &xMob.off_flags,	ed_flag_toggle,		off_flags	},
 { "form",	(void *) &xMob.form,		ed_flag_toggle,		form_flags	},
 { "parts",	(void *) &xMob.parts,		ed_flag_toggle,		part_flags	},
 { "shop",	(void *) &xMob,			ed_shop,		NULL		},
 { "create",	NULL,				ed_new_mob,		NULL		},
 { "spec",	(void *) &xMob.spec_fun,	ed_gamespec,		NULL		},
 { "recval",	(void *) &xMob,			ed_recval,		NULL		},
 { "sex",	(void *) &xMob.sex,		ed_shintlookup,		sex_lookup	},
 { "size",	(void *) &xMob.size,		ed_shintlookup,		size_lookup	},
 { "startpos",	(void *) &xMob.start_pos,	ed_shintlookup,		position_lookup	},
 { "defaultpos",(void *) &xMob.default_pos,	ed_shintlookup,		position_lookup	},
 { "damtype",	(void *) &xMob.dam_type,	ed_shintposlookup,	attack_lookup	},
 { "race",	(void *) &xMob,			ed_race,		NULL		},
 { "armor",	(void *) &xMob,			ed_ac,			NULL		},
 { "hitdice",	(void *) &xMob.hit[0],		ed_dice,		NULL		},
 { "manadice",	(void *) &xMob.mana[0],		ed_dice,		NULL		},
 { "damdice",	(void *) &xMob.damage[0],	ed_dice,		NULL		},
 { "hitroll",	(void *) &xMob.hitroll,		ed_numero_s_pos,	NULL		},
 { "wealth",	(void *) &xMob.wealth,		ed_numero_l_pos,	NULL		},
 { "addprog",	(void *) &xMob.mprogs,		ed_addprog,		NULL		},
 { "delprog",	(void *) &xMob.mprogs,		ed_delprog,		NULL		},
 { "mshow",	NULL,				ed_olded,		medit_show	},
 { "oshow",	NULL,				ed_olded,		oedit_show	},
 { "olist",	(void *) &xMob.area,		ed_olist,		NULL		},
 { "copiar",	NULL,				ed_olded,		medit_copy	},
 { "commands",	NULL,				ed_olded,		show_commands	},
 { "?",		NULL,				ed_olded,		show_help	},
 { "version",	NULL,				ed_olded,		show_version	},
 { NULL,	NULL,				NULL,			NULL		}
};

const	struct	olc_comm_type	obj_olc_comm_table	[]	=
{
 { "name",	(void *) &xObj.name,		ed_line_string,		NULL		},
 { "short",	(void *) &xObj.short_descr,	ed_line_string,		NULL		},
 { "long",	(void *) &xObj.description,	ed_line_string,		NULL		},
 { "material",	(void *) &xObj.material,	ed_line_string,		NULL		},
 { "cost",	(void *) &xObj.cost,		ed_numero_pos,		NULL		},
 { "level",	(void *) &xObj.level,		ed_numero_niv,		NULL		},
 { "condition",	(void *) &xObj.condition,	ed_numero_s_pos,	NULL		},
 { "weight",	(void *) &xObj.weight,		ed_numero_s_pos,	NULL		},
 { "extra",	(void *) &xObj.extra_flags,	ed_flag_toggle,		extra_flags	},
 { "wear",	(void *) &xObj.wear_flags,	ed_flag_toggle,		wear_flags	},
 { "ed",	(void *) &xObj.extra_descr,	ed_ed,			NULL		},
 { "type",	(void *) &xObj.item_type,	ed_flag_set_sh,		type_flags	},
 { "addaffect",	(void *) &xObj,			ed_addaffect,		NULL		},
 { "delaffect",	(void *) &xObj.affected,	ed_delaffect,		NULL		},
 { "addapply",	(void *) &xObj,			ed_addapply,		NULL		},
 { "v0",	NULL,				ed_value,		0		},
 { "v1",	NULL,				ed_value,		(void *) 1	},
 { "v2",	NULL,				ed_value,		(void *) 2	},
 { "v3",	NULL,				ed_value,		(void *) 3	},
 { "v4",	NULL,				ed_value,		(void *) 4	},
 { "create",	NULL,				ed_new_obj,		NULL		},
 { "mshow",	NULL,				ed_olded,		medit_show	},
 { "oshow",	NULL,				ed_olded,		oedit_show	},
 { "olist",	(void *) &xObj.area,		ed_olist,		NULL		},
 { "recval",	(void *) &xObj,			ed_objrecval,		NULL		},
 { "commands",	NULL,				ed_olded,		show_commands	},
 { "?",		NULL,				ed_olded,		show_help	},
 { "version",	NULL,				ed_olded,		show_version	},
 { NULL,	NULL,				NULL,			NULL		}
};

const	struct	olc_comm_type	room_olc_comm_table	[]	=
{
 { "name",	(void *) &xRoom.name,		ed_line_string,		NULL		},
 { "desc",	(void *) &xRoom.description,	ed_desc,		NULL		},
 { "ed",	(void *) &xRoom.extra_descr,	ed_ed,			NULL		},
 { "heal",	(void *) &xRoom.heal_rate,	ed_numero_s_pos,	NULL		},
 { "mana",	(void *) &xRoom.mana_rate,	ed_numero_s_pos,	NULL		},
 { "owner",	(void *) &xRoom.owner,		ed_line_string,		NULL		},
 { "roomflags",	(void *) &xRoom.room_flags,	ed_flag_toggle,		room_flags	},
 { "clan",	(void *) &xRoom.clan,		ed_shintlookup,		clan_lookup	},
 { "sector",	(void *) &xRoom.sector_type,	ed_flag_set_sh,		sector_flags	},
 { "north",	NULL,				ed_direccion,		(void *) DIR_NORTH	},
 { "south",	NULL,				ed_direccion,		(void *) DIR_SOUTH	},
 { "east",	NULL,				ed_direccion,		(void *) DIR_EAST	},
 { "west",	NULL,				ed_direccion,		(void *) DIR_WEST	},
 { "up",	NULL,				ed_direccion,		(void *) DIR_UP		},
 { "down",	NULL,				ed_direccion,		(void *) DIR_DOWN	},
 { "rlist",	NULL,				ed_olded,		redit_rlist	},
 { "mlist",	NULL,				ed_olded,		redit_mlist	},
 { "olist",	(void *) &xRoom.area,		ed_olist,		NULL		},
 { "copiar",	NULL,				ed_olded,		redit_copy	},
 { "listreset",	NULL,				ed_olded,		redit_listreset },
 { "checkobj",	NULL,				ed_olded,		redit_checkobj	},
 { "checkmob",	NULL,				ed_olded,		redit_checkmob	},
 { "checkrooms",NULL,				ed_olded,		redit_checkrooms },
 { "mreset",	NULL,				ed_olded,		redit_mreset	},
 { "oreset",	NULL,				ed_olded,		redit_oreset	},
 { "create",	NULL,				ed_olded,		redit_create	},
 { "format",	NULL,				ed_olded,		redit_format	},
 { "mshow",	NULL,				ed_olded,		medit_show	},
 { "oshow",	NULL,				ed_olded,		oedit_show	},
 { "purge",	NULL,				ed_docomm,		do_purge	},
 { "limpiar",	NULL,				ed_olded,		redit_limpiar	},
 { "commands",	NULL,				ed_olded,		show_commands	},
 { "?",		NULL,				ed_olded,		show_help	},
 { "version",	NULL,				ed_olded,		show_version	},
 { NULL,	NULL,				NULL,			NULL		}
};

void set_editor( DESCRIPTOR_DATA *d, int editor, void * param )
{
	d->editor = editor;
	d->pEdit = param;
	if (d->pagina < 1)
		d->pagina = 1;
	InitScreen(d);
}

/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool    run_olc_editor (DESCRIPTOR_DATA * d, char * incomm)
{
	switch (d->editor)
	{
		case ED_AREA:
			aedit (d->character, incomm);
			break;
		case ED_ROOM:
			redit (d->character, incomm);
			break;
		case ED_OBJECT:
			oedit (d->character, incomm);
			break;
		case ED_MOBILE:
			medit (d->character, incomm);
			break;
		case ED_PROG:
			pedit (d->character, incomm);
			break;
		case ED_RACE:
			raedit (d->character, incomm);
			break;
		case ED_SOCIAL:
			sedit (d->character, incomm);
			break;
		case ED_SKILL:
			skedit (d->character, incomm);
			break;
		case ED_CMD:
			cmdedit (d->character, incomm);
			break;
		case ED_GROUP:
			gedit (d->character, incomm);
			break;
		case ED_HELP:
			hedit (d->character, incomm);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

char   *olc_ed_name (CHAR_DATA * ch)
{
	static char buf[10];

	buf[0] = '\0';
	switch (ch->desc->editor)
	{
		case ED_AREA:
			sprintf (buf, "AEdit");
			break;
		case ED_ROOM:
			sprintf (buf, "REdit");
			break;
		case ED_OBJECT:
			sprintf (buf, "OEdit");
			break;
		case ED_MOBILE:
			sprintf (buf, "MEdit");
			break;
		case ED_PROG:
			sprintf (buf, "MPEdit");
			break;
		case ED_RACE:
			sprintf (buf, "RAEdit");
			break;
		case ED_SOCIAL:
			sprintf (buf, "SEdit");
			break;
		case ED_SKILL:
			sprintf (buf, "SKEdit");
			break;
		case ED_CMD:
			sprintf (buf, "CMDEdit");
			break;
		case ED_GROUP:
			sprintf (buf, "GEdit");
			break;
		case ED_HELP:
			sprintf (buf, "HEdit");
			break;
		default:
			sprintf (buf, " ");
			break;
	}
	return buf;
}



char   *olc_ed_vnum (CHAR_DATA * ch)
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoom;
	OBJ_INDEX_DATA *pObj;
	MOB_INDEX_DATA *pMob;
	MPROG_CODE *pMcode;
	HELP_DATA *pHelp;
	struct race_type *pRace;
	struct social_type *pSocial;
	struct skill_type *pSkill;
	struct cmd_type *pCmd;
	static char buf[10];

	buf[0] = '\0';
	switch (ch->desc->editor)
	{
		case ED_AREA:
			pArea = (AREA_DATA *) ch->desc->pEdit;
			sprintf (buf, "%d", pArea ? pArea->vnum : 0);
			break;
		case ED_ROOM:
			pRoom = ch->in_room;
			sprintf (buf, "%d", pRoom ? pRoom->vnum : 0);
			break;
		case ED_OBJECT:
			pObj = (OBJ_INDEX_DATA *) ch->desc->pEdit;
			sprintf (buf, "%d", pObj ? pObj->vnum : 0);
			break;
		case ED_MOBILE:
			pMob = (MOB_INDEX_DATA *) ch->desc->pEdit;
			sprintf (buf, "%d", pMob ? pMob->vnum : 0);
			break;
		case ED_PROG:
			pMcode = (MPROG_CODE *) ch->desc->pEdit;
			sprintf (buf, "%d", pMcode ? pMcode->vnum : 0);
			break;
		case ED_RACE:
			pRace = (struct race_type *) ch->desc->pEdit;
			sprintf (buf, "%s", pRace ? pRace->name : "");
			break;
		case ED_SOCIAL:
			pSocial = (struct social_type *) ch->desc->pEdit;
			sprintf (buf, "%s", pSocial ? pSocial->name : "");
			break;
		case ED_SKILL:
			pSkill = (struct skill_type *) ch->desc->pEdit;
			sprintf (buf, "%s", pSkill ? pSkill->name : "" );
			break;
		case ED_CMD:
			pCmd = (struct cmd_type *) ch->desc->pEdit;
			sprintf (buf, "%s", pCmd ? pCmd->name : "" );
			break;
		case ED_HELP:
			pHelp = (HELP_DATA *) ch->desc->pEdit;
			sprintf (buf, "%s", pHelp ? pHelp->keyword : "" );
			break;
		default:
			sprintf (buf, " ");
			break;
	}

	return buf;
}

const struct olc_comm_type * get_olc_tabla( int editor )
{
	switch(editor)
	{
		case ED_MOBILE:	return mob_olc_comm_table;
		case ED_OBJECT:	return obj_olc_comm_table;
		case ED_ROOM:	return room_olc_comm_table;
		case ED_SKILL:	return skill_olc_comm_table;
		case ED_RACE:	return race_olc_comm_table;
		case ED_CMD:	return cmd_olc_comm_table;
		case ED_PROG:	return prog_olc_comm_table;
		case ED_SOCIAL:	return social_olc_comm_table;
	}
	return NULL;
}

/*****************************************************************************
 Name:		show_olc_cmds
 Purpose:	Format up the commands from given table.
 Called by:	show_commands(olc_act.c).
 ****************************************************************************/
void show_olc_cmds( CHAR_DATA *ch )
{
	char    buf[MAX_STRING_LENGTH];
	char    buf1[MAX_STRING_LENGTH];
	const struct olc_comm_type * tabla;
	int     cmd;
	int     col;

	buf1[0] = '\0';
	col = 0;

	tabla = get_olc_tabla(ch->desc->editor);

	if (tabla == NULL)
	{
		bugf( "slow_olc_cmds : tabla NULL, editor %d",
			ch->desc->editor );
		return;
	}

	for (cmd = 0; tabla[cmd].nombre != NULL; cmd++)
	{
		sprintf (buf, "%-15.15s", tabla[cmd].nombre);
		strcat (buf1, buf);
		if (++col % 5 == 0)
			strcat (buf1, "\n\r");
	}

	if (col % 5 != 0)
		strcat (buf1, "\n\r");

	send_to_char (buf1, ch);
	return;
}

/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
bool    show_commands (CHAR_DATA * ch, char *argument)
{
	show_olc_cmds(ch);

	return FALSE;
}

/*****************************************************************************
 *                           Interpreter Tables.                             *
 *****************************************************************************/
const struct olc_cmd_type aedit_table[] =
{
/* {	command		function	}, */

   {	"age", 		aedit_age	},
   {	"builder", 	aedit_builder	},	/* s removed -- Hugin */
   {	"commands", 	show_commands	},
   {	"create", 	aedit_create	},
   {	"filename", 	aedit_file	},
   {	"name", 	aedit_name	},
/* {	"recall",	aedit_recall    },   ROM OLC */
   {	"reset", 	aedit_reset	},
   {	"security", 	aedit_security	},
   {	"show", 	aedit_show	},
   {	"vnum", 	aedit_vnum	},
   {	"lvnum", 	aedit_lvnum	},
   {	"uvnum", 	aedit_uvnum	},
   {	"credits", 	aedit_credits	},
   {	"lowrange", 	aedit_lowrange	},
   {	"highrange", 	aedit_highrange	},

   {	"?",		show_help	},
   {	"version", 	show_version	},

   {	NULL, 		0		}
};

/*****************************************************************************
 *                          End Interpreter Tables.                          *
 *****************************************************************************/



/*****************************************************************************
 Name:		get_area_data
 Purpose:	Returns pointer to area with given vnum.
 Called by:	do_aedit(olc.c).
 ****************************************************************************/
AREA_DATA *get_area_data (int vnum)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea; pArea = pArea->next)
	{
		if (pArea->vnum == vnum)
			return pArea;
	}

	return 0;
}



/*****************************************************************************
 Name:		edit_done
 Purpose:	Resets builder information on completion.
 Called by:	aedit, redit, oedit, medit(olc.c)
 ****************************************************************************/
bool    edit_done (CHAR_DATA * ch)
{
COMMAND(do_clear)

	if (ch->desc->editor != ED_NONE)
		send_to_char("Saliendo del editor.\n\r", ch);
	ch->desc->pEdit = NULL;
	ch->desc->editor = ED_NONE;
	ch->desc->pagina = 0;
	if (IS_SET(ch->comm, COMM_OLCX))
	{
		do_clear(ch,"reset");
		InitScreen(ch->desc);
	}
	return FALSE;
}

/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/


/* Area Interpreter, called by do_aedit. */
void    aedit (CHAR_DATA * ch, char *argument)
{
	AREA_DATA *pArea;
	char    command[MAX_INPUT_LENGTH];
	char    arg[MAX_INPUT_LENGTH];
	int     cmd;
	int     value;

	EDIT_AREA (ch, pArea);

	strcpy (arg, argument);
	argument = one_argument (argument, command);

	if (!IS_BUILDER (ch, pArea))
	{
		send_to_char ("AEdit:  Insufficient security to modify area.\n\r", ch);
		edit_done (ch);
		return;
	}

	if (!str_cmp (command, "done"))
	{
		edit_done (ch);
		return;
	}

	if (command[0] == '\0')
	{
		aedit_show (ch, argument);
		return;
	}

	if ((value = flag_value (area_flags, command)) != NO_FLAG)
	{
		TOGGLE_BIT (pArea->area_flags, value);

		send_to_char ("Flag toggled.\n\r", ch);
		return;
	}

	/* Search Table and Dispatch Command. */
	for (cmd = 0; aedit_table[cmd].name != NULL; cmd++)
	{
		if (!str_prefix (command, aedit_table[cmd].name))
		{
			if ((*aedit_table[cmd].olc_fun) (ch, argument))
			{
				SET_BIT (pArea->area_flags, AREA_CHANGED);
				return;
			}
			else
				return;
		}
	}

	/* Default to Standard Interpreter. */
	interpret (ch, arg);
	return;
}

/* Room Interpreter, called by do_redit. */
void    redit (CHAR_DATA * ch, char *argument)
{
	ROOM_INDEX_DATA *pRoom;
	AREA_DATA *pArea;

	EDIT_ROOM (ch, pRoom);
	pArea = pRoom->area;

	if (!IS_BUILDER (ch, pArea))
	{
		send_to_char ("REdit:  Insufficient security to modify room.\n\r", ch);
		edit_done (ch);
		return;
	}

	if (!str_cmp (argument, "done"))
	{
		edit_done (ch);
		return;
	}

	if ( emptystring(argument) )
	{
		redit_show (ch, argument);
		return;
	}

	/* Search Table and Dispatch Command. */
	if ( !procesar_comando_olc(ch, argument, room_olc_comm_table) )
		interpret(ch, argument);

	return;
}

/* Object Interpreter, called by do_oedit. */
void    oedit (CHAR_DATA * ch, char *argument)
{
	AREA_DATA *pArea;
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ (ch, pObj);
	pArea = pObj->area;

	if (!IS_BUILDER (ch, pArea))
	{
		send_to_char ("OEdit: Insufficient security to modify area.\n\r", ch);
		edit_done (ch);
		return;
	}

	if (!str_cmp (argument, "done"))
	{
		edit_done (ch);
		return;
	}

	if (emptystring(argument))
	{
		oedit_show (ch, argument);
		return;
	}

	/* Search Table and Dispatch Command. */
	if ( !procesar_comando_olc(ch, argument, obj_olc_comm_table) )
		interpret(ch, argument);

	return;
}

/* Mobile Interpreter, called by do_medit. */
void    medit (CHAR_DATA * ch, char *argument)
{
	AREA_DATA *pArea;
	MOB_INDEX_DATA *pMob;

	EDIT_MOB (ch, pMob);
	pArea = pMob->area;

	if (!IS_BUILDER (ch, pArea))
	{
		send_to_char ("MEdit: Insufficient security to modify area.\n\r", ch);
		edit_done (ch);
		return;
	}

	if (!str_cmp (argument, "done"))
	{
		edit_done (ch);
		return;
	}

	if (emptystring(argument))
	{
		medit_show (ch, argument);
		return;
	}

	/* Search Table and Dispatch Command. */
	if (!procesar_comando_olc(ch, argument, mob_olc_comm_table))
		interpret (ch, argument);

	return;
}




const struct editor_cmd_type editor_table[] =
{
/* {	command		function	}, */

   {	"area",		do_aedit	},
   {	"room",		do_redit	},
   {	"object",	do_oedit	},
   {	"mobile",	do_medit	},
   {	"program",	do_pedit	},
   {	"race",		do_raedit	},
   {	"social",	do_sedit	},
   {	"skill",	do_skedit	},
   {	"command",	do_cmdedit	},
   {	"group",	do_gedit	},
   {	"help",		do_hedit	},

   {	NULL,		0		}
};


/* Entry point for all editors. */
void    do_olc (CHAR_DATA * ch, char *argument)
{
	char    command[MAX_INPUT_LENGTH];
	int     cmd;

	argument = one_argument (argument, command);

	if (command[0] == '\0')
	{
		do_help (ch, "olc");
		return;
	}

	/* Search Table and Dispatch Command. */
	for (cmd = 0; editor_table[cmd].name != NULL; cmd++)
	{
		if (!str_prefix (command, editor_table[cmd].name))
		{
			(*editor_table[cmd].do_fun) (ch, argument);
			return;
		}
	}

	/* Invalid command, send help. */
	do_help (ch, "olc");
	return;
}



/* Entry point for editing area_data. */
void    do_aedit (CHAR_DATA * ch, char *argument)
{
	AREA_DATA *pArea;
	int     value;
	char    arg[MAX_STRING_LENGTH];

	pArea = ch->in_room->area;

	argument = one_argument (argument, arg);
	if (is_number (arg))
	{
		value = atoi (arg);
		if (!(pArea = get_area_data (value)))
		{
			send_to_char ("That area vnum does not exist.\n\r", ch);
			return;
		}
	}
	else if (!str_cmp (arg, "create"))
	{
		if (!aedit_create (ch, argument))
			return;
		else
			pArea = area_last;
	}

	if (!IS_BUILDER (ch, pArea) || ch->pcdata->security < 9)
	{
		send_to_char ("Insuficiente seguridad para editar areas.\n\r", ch);
		return;
	}

	set_editor(ch->desc, ED_AREA, pArea);
/*	ch->desc->pEdit = (void *) pArea;
	ch->desc->editor = ED_AREA; */
	return;
}

/* Entry point for editing room_index_data. */
void    do_redit (CHAR_DATA * ch, char *argument)
{
	ROOM_INDEX_DATA *pRoom;
	char    arg1[MIL];

	argument = one_argument (argument, arg1);

	pRoom = ch->in_room;

	if (!str_cmp (arg1, "reset"))
	{
		if (!IS_BUILDER (ch, pRoom->area))
		{
			send_to_char ("Insuficiente seguridad para modificar cuartos.\n\r", ch);
			return;
		}

		reset_room (pRoom);
		send_to_char ("Cuarto reseteado.\n\r", ch);
		return;
	}
	else if (!str_cmp (arg1, "create"))
	{
		if (argument[0] == '\0' || atoi (argument) == 0)
		{
			send_to_char ("Sintaxis : edit room create [vnum]\n\r", ch);
			return;
		}

		if (redit_create (ch, argument))
		{
			char_from_room (ch);
			char_to_room (ch, ch->desc->pEdit);
			SET_BIT (pRoom->area->area_flags, AREA_CHANGED);
			pRoom = ch->in_room;
		}
	}
	else if ( !IS_NULLSTR(arg1) )
	{
		pRoom = get_room_index (atoi (arg1));

		if ( pRoom == NULL )
		{
			send_to_char( "Cuarto inexistente.\n\r", ch );
			return;
		}
	}

	if (!IS_BUILDER (ch, pRoom->area))
	{
		send_to_char ("Insuficiente seguridad para modificar cuartos.\n\r", ch);
		return;
	}

	if ( pRoom == NULL )
		bugf( "do_redit : NULL pRoom, ch %s!", ch->name );

	if (ch->in_room != pRoom)
	{
		char_from_room(ch);
		char_to_room(ch, pRoom);
	}

	set_editor(ch->desc, ED_ROOM, pRoom);

/*	ch->desc->editor	= ED_ROOM;
	ch->desc->pEdit		= pRoom;

	if (ch->desc->pagina == 0)
		ch->desc->pagina = 1; */

	return;
}

/* Entry point for editing obj_index_data. */
void    do_oedit (CHAR_DATA * ch, char *argument)
{
	OBJ_INDEX_DATA *pObj;
	AREA_DATA *pArea;
	char    arg1[MAX_STRING_LENGTH];
	int     value;

	if (IS_NPC (ch))
		return;

	argument = one_argument (argument, arg1);

	if (is_number (arg1))
	{
		value = atoi (arg1);
		if (!(pObj = get_obj_index (value)))
		{
			send_to_char ("OEdit:  That vnum does not exist.\n\r", ch);
			return;
		}

		if (!IS_BUILDER (ch, pObj->area))
		{
			send_to_char ("Insuficiente seguridad para modificar objetos.\n\r", ch);
			return;
		}

		set_editor(ch->desc, ED_OBJECT, pObj);
/*		ch->desc->pEdit = (void *) pObj;
		ch->desc->editor = ED_OBJECT; */
		return;
	}
	else
	{
		if (!str_cmp (arg1, "create"))
		{
			value = atoi (argument);
			if (argument[0] == '\0' || value == 0)
			{
				send_to_char ("Syntax:  edit object create [vnum]\n\r", ch);
				return;
			}

			pArea = get_vnum_area (value);

			if (!pArea)
			{
				send_to_char ("OEdit:  That vnum is not assigned an area.\n\r", ch);
				return;
			}

			if (!IS_BUILDER (ch, pArea))
			{
				send_to_char ("Insuficiente seguridad para modificar objetos.\n\r", ch);
				return;
			}

			if (oedit_create (ch, argument))
			{
				SET_BIT (pArea->area_flags, AREA_CHANGED);
				ch->desc->editor = ED_OBJECT;
			}
			return;
		}
	}

	send_to_char ("OEdit:  There is no default object to edit.\n\r", ch);
	return;
}



/* Entry point for editing mob_index_data. */
void    do_medit (CHAR_DATA * ch, char *argument)
{
	MOB_INDEX_DATA *pMob;
	AREA_DATA *pArea;
	int     value;
	char    arg1[MAX_STRING_LENGTH];

	argument = one_argument (argument, arg1);

	if (is_number (arg1))
	{
		value = atoi (arg1);
		if (!(pMob = get_mob_index (value)))
		{
			send_to_char ("MEdit:  That vnum does not exist.\n\r", ch);
			return;
		}

		if (!IS_BUILDER (ch, pMob->area))
		{
			send_to_char ("Insuficiente seguridad para modificar mobs.\n\r", ch);
			return;
		}

		set_editor(ch->desc, ED_MOBILE, pMob);
/*		ch->desc->pEdit = (void *) pMob;
		ch->desc->editor = ED_MOBILE; */
		return;
	}
	else
	{
		if (!str_cmp (arg1, "create"))
		{
			value = atoi (argument);
			if (arg1[0] == '\0' || value == 0)
			{
				send_to_char ("Syntax:  edit mobile create [vnum]\n\r", ch);
				return;
			}

			pArea = get_vnum_area (value);

			if (!pArea)
			{
				send_to_char ("MEdit:  That vnum is not assigned an area.\n\r", ch);
				return;
			}

			if (!IS_BUILDER (ch, pArea))
			{
				send_to_char ("Insuficiente seguridad para modificar mobs.\n\r", ch);
				return;
			}

			if (ed_new_mob ("create", ch, argument, NULL, NULL) )
			{
				SET_BIT (pArea->area_flags, AREA_CHANGED);
				ch->desc->editor = ED_MOBILE;
			}
			return;
		}
	}

	send_to_char ("MEdit:  There is no default mobile to edit.\n\r", ch);
	return;
}

void    display_resets (CHAR_DATA * ch, ROOM_INDEX_DATA * pRoom)
{
	RESET_DATA *pReset;
	MOB_INDEX_DATA *pMob = NULL;
	char    buf[MAX_STRING_LENGTH];
	char    final[MAX_STRING_LENGTH];
	int     iReset = 0;

	final[0] = '\0';

	send_to_char (
	   " No.  Loads    Description       Location         Vnum   Ar Rm Description"
	   "\n\r"
	   "==== ======== ============= =================== ======== ===== ==========="
	   "\n\r", ch);

	for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
	{
		OBJ_INDEX_DATA *pObj;
		MOB_INDEX_DATA *pMobIndex;
		OBJ_INDEX_DATA *pObjIndex;
		OBJ_INDEX_DATA *pObjToIndex;
		ROOM_INDEX_DATA *pRoomIndex;

		final[0] = '\0';
		sprintf (final, "[%2d] ", ++iReset);

		switch (pReset->command)
		{
			default:
				sprintf (buf, "Bad reset command: %c.", pReset->command);
				strcat (final, buf);
				break;

			case 'M':
				if (!(pMobIndex = get_mob_index (pReset->arg1)))
				{
					sprintf (buf, "Load Mobile - Bad Mob %d\n\r", pReset->arg1);
					strcat (final, buf);
					continue;
				}

				if (!(pRoomIndex = get_room_index (pReset->arg3)))
				{
					sprintf (buf, "Load Mobile - Bad Room %d\n\r", pReset->arg3);
					strcat (final, buf);
					continue;
				}

				pMob = pMobIndex;
				sprintf (buf, "M[%5d] %-13.13s en el suelo         R[%5d] %2d-%2d %-15.15s\n\r",
				   pReset->arg1, pMob->short_descr, pReset->arg3,
				   pReset->arg2, pReset->arg4, pRoomIndex->name);
				strcat (final, buf);

				/*
				 * Check for pet shop.
				 * -------------------
				 */
				{
					ROOM_INDEX_DATA *pRoomIndexPrev;

					pRoomIndexPrev = get_room_index (pRoomIndex->vnum - 1);
					if (pRoomIndexPrev
					   && IS_SET (pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
						final[5] = 'P';
				}

				break;

			case 'O':
				if (!(pObjIndex = get_obj_index (pReset->arg1)))
				{
					sprintf (buf, "Load Object - Bad Object %d\n\r",
					   pReset->arg1);
					strcat (final, buf);
					continue;
				}

				pObj = pObjIndex;

				if (!(pRoomIndex = get_room_index (pReset->arg3)))
				{
					sprintf (buf, "Load Object - Bad Room %d\n\r", pReset->arg3);
					strcat (final, buf);
					continue;
				}

				sprintf (buf, "O[%5d] %-13.13s en el suelo         "
				   "R[%5d]       %-15.15s\n\r",
				   pReset->arg1, pObj->short_descr,
				   pReset->arg3, pRoomIndex->name);
				strcat (final, buf);

				break;

			case 'P':
				if (!(pObjIndex = get_obj_index (pReset->arg1)))
				{
					sprintf (buf, "Put Object - Bad Object %d\n\r",
					   pReset->arg1);
					strcat (final, buf);
					continue;
				}

				pObj = pObjIndex;

				if (!(pObjToIndex = get_obj_index (pReset->arg3)))
				{
					sprintf (buf, "Put Object - Bad To Object %d\n\r",
					   pReset->arg3);
					strcat (final, buf);
					continue;
				}

				sprintf (buf,
				   "O[%5d] %-13.13s inside              O[%5d] %2d-%2d %-15.15s\n\r",
				   pReset->arg1,
				   pObj->short_descr,
				   pReset->arg3,
				   pReset->arg2,
				   pReset->arg4,
				   pObjToIndex->short_descr);
				strcat (final, buf);

				break;

			case 'G':
			case 'E':
				if (!(pObjIndex = get_obj_index (pReset->arg1)))
				{
					sprintf (buf, "Give/Equip Object - Bad Object %d\n\r",
					   pReset->arg1);
					strcat (final, buf);
					continue;
				}

				pObj = pObjIndex;

				if (!pMob)
				{
					sprintf (buf, "Give/Equip Object - No Previous Mobile\n\r");
					strcat (final, buf);
					break;
				}

				if (pMob->pShop)
				{
					sprintf (buf,
					   "O[%5d] %-13.13s en el inventario de S[%5d]       %-15.15s\n\r",
					   pReset->arg1,
					   pObj->short_descr,
					   pMob->vnum,
					   pMob->short_descr);
				}
				else
					sprintf (buf,
					   "O[%5d] %-13.13s %-19.19s M[%5d]       %-15.15s\n\r",
					   pReset->arg1,
					   pObj->short_descr,
					   (pReset->command == 'G') ?
					   flag_string (wear_loc_strings, WEAR_NONE)
					   : flag_string (wear_loc_strings, pReset->arg3),
					   pMob->vnum,
					   pMob->short_descr);
				strcat (final, buf);
				break;

				/*
				 * Doors are set in rs_flags don't need to be displayed.
				 * If you want to display them then uncomment the new_reset
				 * line in the case 'D' in load_resets in db.c and here.
				 */
			case 'D':
				pRoomIndex = get_room_index (pReset->arg1);
				sprintf (buf, "R[%5d] %s door of %-19.19s reset to %s\n\r",
				   pReset->arg1,
				   capitalize (dir_name[pReset->arg2]),
				   pRoomIndex->name,
				   flag_string (door_resets, pReset->arg3));
				strcat (final, buf);

				break;
				/*
				 * End Doors Comment.
				 */
			case 'R':
				if (!(pRoomIndex = get_room_index (pReset->arg1)))
				{
					sprintf (buf, "Randomize Exits - Bad Room %d\n\r",
					   pReset->arg1);
					strcat (final, buf);
					continue;
				}

				sprintf (buf, "R[%5d] Exits are randomized in %s\n\r",
				   pReset->arg1, pRoomIndex->name);
				strcat (final, buf);

				break;
		}
		send_to_char (final, ch);
	}

	return;
}



/*****************************************************************************
 Name:		add_reset
 Purpose:	Inserts a new reset in the given index slot.
 Called by:	do_resets(olc.c).
 ****************************************************************************/
void    add_reset (ROOM_INDEX_DATA * room, RESET_DATA * pReset, int indice)
{
	RESET_DATA *reset;
	int     iReset = 0;

	if (!room->reset_first)
	{
		room->reset_first = pReset;
		room->reset_last = pReset;
		pReset->next = NULL;
		return;
	}

	indice--;

	if (indice == 0)				/* First slot (1) selected. */
	{
		pReset->next = room->reset_first;
		room->reset_first = pReset;
		return;
	}

	/*
	 * If negative slot( <= 0 selected) then this will find the last.
	 */
	for (reset = room->reset_first; reset->next; reset = reset->next)
	{
		if (++iReset == indice)
			break;
	}

	pReset->next = reset->next;
	reset->next = pReset;
	if (!pReset->next)
		room->reset_last = pReset;
	return;
}

void    do_resets (CHAR_DATA * ch, char *argument)
{
	char    arg1[MAX_INPUT_LENGTH];
	char    arg2[MAX_INPUT_LENGTH];
	char    arg3[MAX_INPUT_LENGTH];
	char    arg4[MAX_INPUT_LENGTH];
	char    arg5[MAX_INPUT_LENGTH];
	char    arg6[MAX_INPUT_LENGTH];
	char    arg7[MAX_INPUT_LENGTH];
	RESET_DATA *pReset = NULL;

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	argument = one_argument (argument, arg3);
	argument = one_argument (argument, arg4);
	argument = one_argument (argument, arg5);
	argument = one_argument (argument, arg6);
	argument = one_argument (argument, arg7);

	if (!IS_BUILDER (ch, ch->in_room->area))
	{
		send_to_char ("Resets: Invalid security for editing this area.\n\r",
		   ch);
		return;
	}

	/*
	 * Display resets in current room.
	 * -------------------------------
	 */
	if (arg1[0] == '\0')
	{
		if (ch->in_room->reset_first)
		{
			send_to_char (
			   "Resets: M = mobile, R = room, O = object, "
			   "P = pet, S = shopkeeper\n\r", ch);
			display_resets (ch, ch->in_room);
		}
		else
			send_to_char ("No resets in this room.\n\r", ch);
	}

	/*
	 * Take index number and search for commands.
	 * ------------------------------------------
	 */
	if (is_number (arg1))
	{
		ROOM_INDEX_DATA *pRoom = ch->in_room;

		/*
		 * Delete a reset.
		 * ---------------
		 */
		if (!str_cmp (arg2, "delete"))
		{
			int     insert_loc = atoi (arg1);

			if (!ch->in_room->reset_first)
			{
				send_to_char ("No resets in this area.\n\r", ch);
				return;
			}

			if (insert_loc - 1 <= 0)
			{
				pReset = pRoom->reset_first;
				pRoom->reset_first = pRoom->reset_first->next;
				if (!pRoom->reset_first)
					pRoom->reset_last = NULL;
			}
			else
			{
				int     iReset = 0;
				RESET_DATA *prev = NULL;

				for (pReset = pRoom->reset_first;
				   pReset;
				   pReset = pReset->next)
				{
					if (++iReset == insert_loc)
						break;
					prev = pReset;
				}

				if (!pReset)
				{
					send_to_char ("Reset not found.\n\r", ch);
					return;
				}

				if (prev)
					prev->next = prev->next->next;
				else
					pRoom->reset_first = pRoom->reset_first->next;

				for (pRoom->reset_last = pRoom->reset_first;
				   pRoom->reset_last->next;
				   pRoom->reset_last = pRoom->reset_last->next) ;
			}

			free_reset_data (pReset);
			send_to_char ("Reset deleted.\n\r", ch);
			SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
		}
		else
			/*
			 * Add a reset.
			 * ------------
			 */
			if ((!str_cmp (arg2, "mob") && is_number (arg3))
		   || (!str_cmp (arg2, "obj") && is_number (arg3)))
		{
			/*
			 * Check for Mobile reset.
			 * -----------------------
			 */
			if (!str_cmp (arg2, "mob"))
			{
				if (get_mob_index (is_number (arg3) ? atoi (arg3) : 1) == NULL)
				{
					send_to_char ("Mob no existe.\n\r", ch);
					return;
				}
				pReset = new_reset_data ();
				pReset->command = 'M';
				pReset->arg1 = atoi (arg3);
				pReset->arg2 = is_number (arg4) ? atoi (arg4) : 1;	/* Max # */
				pReset->arg3 = ch->in_room->vnum;
				pReset->arg4 = is_number (arg5) ? atoi (arg5) : 1;	/* Min # */
			}
			else
				/*
				 * Check for Object reset.
				 * -----------------------
				 */
			if (!str_cmp (arg2, "obj"))
			{
				/*
				 * Inside another object.
				 * ----------------------
				 */
				if (!str_prefix (arg4, "inside"))
				{
					OBJ_INDEX_DATA *temp;

					temp = get_obj_index (is_number (arg5) ? atoi (arg5) : 1);
					if ((temp->item_type != ITEM_CONTAINER) &&
					   (temp->item_type != ITEM_CORPSE_NPC))
					{
						send_to_char ("Objeto 2 no es container.\n\r", ch);
						return;
					}
					pReset = new_reset_data ();
					pReset->arg1 = atoi (arg3);
					pReset->command = 'P';
					pReset->arg2 = is_number (arg6) ? atoi (arg6) : 1;
					pReset->arg3 = is_number (arg5) ? atoi (arg5) : 1;
					pReset->arg4 = is_number (arg7) ? atoi (arg7) : 1;
				}
				else
					/*
					 * Inside the room.
					 * ----------------
					 */
				if (!str_cmp (arg4, "room"))
				{
					if (get_obj_index (atoi (arg3)) == NULL)
					{
						send_to_char ("Vnum no existe.\n\r", ch);
						return;
					}
					pReset = new_reset_data ();
					pReset->arg1 = atoi (arg3);
					pReset->command = 'O';
					pReset->arg2 = 0;
					pReset->arg3 = ch->in_room->vnum;
					pReset->arg4 = 0;
				}
				else
					/*
					 * Into a Mobile's inventory.
					 * --------------------------
					 */
				{
					int blah = flag_value(wear_loc_flags, arg4);

					if (blah == NO_FLAG)
					{
						send_to_char ("Resets: '? wear-loc'\n\r", ch);
						return;
					}
					if (get_obj_index (atoi (arg3)) == NULL)
					{
						send_to_char ("Vnum no existe.\n\r", ch);
						return;
					}
					pReset = new_reset_data ();
					pReset->arg1 = atoi (arg3);
					pReset->arg3 = blah;
					if (pReset->arg3 == WEAR_NONE)
						pReset->command = 'G';
					else
						pReset->command = 'E';
				}
			}
			add_reset (ch->in_room, pReset, atoi (arg1));
			SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
			send_to_char ("Reset added.\n\r", ch);
		}
		else if (!str_cmp (arg2, "random") && is_number (arg3))
		{
			if (atoi (arg3) < 1 || atoi (arg3) > 6)
			{
				send_to_char ("Invalid argument.\n\r", ch);
				return;
			}
			pReset = new_reset_data ();
			pReset->command = 'R';
			pReset->arg1 = ch->in_room->vnum;
			pReset->arg2 = atoi (arg3);
			add_reset (ch->in_room, pReset, atoi (arg1));
			SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);
			send_to_char ("Random exits reset added.\n\r", ch);
		}
		else
		{
			send_to_char ("Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r", ch);
			send_to_char ("        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]\n\r", ch);
			send_to_char ("        RESET <number> OBJ <vnum> room\n\r", ch);
			send_to_char ("        RESET <number> MOB <vnum> [max #x area] [max #x room]\n\r", ch);
			send_to_char ("        RESET <number> DELETE\n\r", ch);
			send_to_char ("        RESET <number> RANDOM [#x exits]\n\r", ch);
		}
	}
	else // arg1 no es un numero
	{
		if ( !str_cmp(arg1, "anadir") )
		{
			int tvar = 0, found = 0;
			char * arg;

			if ( is_number(arg2) )
			{
				send_to_char(	"Sintaxis erronea.\n\r"
						"Las posibilidades son :\n\r"
						"reset anadir mob [vnum/nombre]\n\r"
						"reset anadir obj [vnum/nombre]\n\r"
						"reset anadir [nombre]\n\r", ch );
				return;
			}

			if ( !str_cmp(arg2, "mob") )
			{
				tvar = 1;
				arg = arg3;
			}
			else
			if ( !str_cmp(arg2, "obj") )
			{
				tvar = 2;
				arg = arg3;
			}
			else
				arg = arg2;

			if ( tvar == 0 || tvar == 1 )
			{
				if ( is_number(arg) )
					found = get_mob_index(atoi(arg)) ? atoi(arg) : 0;
				else
					found = get_vnum_mob_name_area(arg, ch->in_room->area);
				if (found)
					tvar = 1;
			}

			if ( found == 0 && (tvar == 0 || tvar == 2) )
			{
				if ( is_number(arg) )
					found = get_obj_index(atoi(arg)) ? atoi(arg) : 0;
				else
					found = get_vnum_obj_name_area(arg, ch->in_room->area);
				if (found)
					tvar = 2;
			}

			if (found == 0)
			{
				printf_to_char(ch, "%s no encontrado en el area.\n\r",
					(tvar == 0) ? "Mob/objeto" : ((tvar == 1) ? "Mob" : "Objeto") );
				return;
			}
			pReset		= new_reset_data ();
			pReset->command	= tvar == 1 ? 'M' : 'O';
			pReset->arg1	= found;
			pReset->arg2	= (tvar == 2) ? 0 : MAX_MOB;	/* Max # */
			pReset->arg3	= ch->in_room->vnum;
			pReset->arg4	= (tvar == 2) ? 0 : MAX_MOB;	/* Min # */

			printf_to_char(ch, "Anadiendo reset del %s %d...", tvar == 1 ? "mob" : "objeto", found );
			add_reset(ch->in_room, pReset, -1); // al final
			SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);
			send_to_char("hecho.\n\r", ch);
		} // anadir
	}

	return;
}

/*****************************************************************************
 Name:		do_alist
 Purpose:	Normal command to list areas and display area information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void    do_alist (CHAR_DATA * ch, char *argument)
{
	char    buf[MAX_STRING_LENGTH];
	char    result[MAX_STRING_LENGTH * 2];	/* May need tweaking. */
	AREA_DATA *pArea;

	sprintf (result, "[%3s] [%-27s] (%-5s-%5s) [%-10s] %3s [%-10s]\n\r",
	   "Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec", "Builders");

	for (pArea = area_first; pArea; pArea = pArea->next)
	{
		sprintf (buf, "[%3d] %-29.29s (%-5d-%5d) %-12.12s [%d] [%-10.10s]\n\r",
		   pArea->vnum,
		   pArea->name,
		   pArea->min_vnum,
		   pArea->max_vnum,
		   pArea->file_name,
		   pArea->security,
		   pArea->builders);
		strcat (result, buf);
	}

	send_to_char (result, ch);
	return;
}

bool procesar_comando_olc( CHAR_DATA *ch, char *argument, const struct olc_comm_type * tabla )
{
	char arg[MIL];
	MOB_INDEX_DATA *pMob;
	OBJ_INDEX_DATA *pObj;
	ROOM_INDEX_DATA *pRoom;
	struct race_type *pRace;
	struct skill_type *pSkill;
	struct cmd_type *pCmd;
	AREA_DATA *tArea;
	MPROG_CODE * pProg;
	struct social_type *pSoc;
	int temp;
	void * puntero;

	argument = one_argument( argument, arg );

	for ( temp = 0; tabla[temp].nombre; temp++ )
	{
		if ( LOWER(arg[0]) == LOWER(tabla[temp].nombre[0])
		&&  !str_prefix(arg, tabla[temp].nombre) )
		{
			switch(ch->desc->editor)
			{
				case ED_MOBILE:
				EDIT_MOB(ch, pMob);
				tArea = pMob->area;
				if (tabla[temp].argumento)
					puntero = (void *) ((int) tabla[temp].argumento - (int) &xMob + (int) pMob);
				else
					puntero = NULL;
				if ( (*tabla[temp].funcion) (tabla[temp].nombre, ch, argument, puntero, tabla[temp].parametro )
				     && tArea )
					SET_BIT(tArea->area_flags, AREA_CHANGED);
				return TRUE;
				break;

				case ED_OBJECT:
				EDIT_OBJ(ch, pObj);
				tArea = pObj->area;
				if (tabla[temp].argumento)
					puntero = (void *) ((int) tabla[temp].argumento - (int) &xObj + (int) pObj);
				else
					puntero = NULL;
				if ( (*tabla[temp].funcion) (tabla[temp].nombre, ch, argument, puntero, tabla[temp].parametro )
				     && tArea != NULL )
					SET_BIT(tArea->area_flags, AREA_CHANGED);
				return TRUE;
				break;

				case ED_ROOM:
				EDIT_ROOM(ch, pRoom);
				tArea = pRoom->area;
				if (tabla[temp].argumento)
					puntero = (void *) ((int) tabla[temp].argumento - (int) &xRoom + (int) pRoom);
				else
					puntero = NULL;
				if ( (*tabla[temp].funcion) (tabla[temp].nombre, ch, argument, puntero, tabla[temp].parametro )
				     && tArea != NULL )
					SET_BIT(tArea->area_flags, AREA_CHANGED);
				return TRUE;
				break;

				case ED_SKILL:
				EDIT_SKILL(ch, pSkill);
				if (tabla[temp].argumento)
					puntero = (void *) ((int) tabla[temp].argumento - (int) &xSkill + (int) pSkill);
				else
					puntero = NULL;
				if ( (*tabla[temp].funcion) (tabla[temp].nombre, ch, argument, puntero, tabla[temp].parametro ) )
					grabar_skills();
				return TRUE;
				break;

				case ED_RACE:
				EDIT_RACE(ch, pRace);
				if (tabla[temp].argumento)
					puntero = (void *) ((int) tabla[temp].argumento - (int) &xRace + (int) pRace);
				else
					puntero = NULL;
				if ( (*tabla[temp].funcion) (tabla[temp].nombre, ch, argument, puntero, tabla[temp].parametro ) )
					grabar_razas();
				return TRUE;
				break;

				case ED_PROG:
				EDIT_PROG(ch, pProg);
				if (tabla[temp].argumento)
					puntero = (void *) ((int) tabla[temp].argumento - (int) &xProg + (int) pProg);
				else
					puntero = NULL;
				if ( (*tabla[temp].funcion) (tabla[temp].nombre, ch, argument, puntero, tabla[temp].parametro ) )
					pProg->changed = TRUE;
				return TRUE;
				break;

				case ED_CMD:
				EDIT_CMD(ch, pCmd);
				if (tabla[temp].argumento)
					puntero = (void *) ((int) tabla[temp].argumento - (int) &xCmd + (int) pCmd);
				else
					puntero = NULL;
				if ( (*tabla[temp].funcion) (tabla[temp].nombre, ch, argument, puntero, tabla[temp].parametro ) )
					grabar_tabla_comandos();
				return TRUE;
				break;

				case ED_SOCIAL:
				EDIT_SOCIAL(ch, pSoc);
				if (tabla[temp].argumento)
					puntero = (void *) ((int) tabla[temp].argumento - (int) &xSoc + (int) pSoc);
				else
					puntero = NULL;
				if ( (*tabla[temp].funcion) (tabla[temp].nombre, ch, argument, puntero, tabla[temp].parametro ) )
					grabar_socials();
				return TRUE;
				break;
			}
		}
	}

	return FALSE;
}

DO_FUN_DEC(do_pagina)
{
extern	void UpdateOLCScreen	(DESCRIPTOR_DATA *);
	int num;

	if (IS_NPC(ch)
	||  ch->desc == NULL
	||  ch->desc->editor == ED_NONE )
	{
		send_to_char( "No estas editando nada.\n\r", ch );
		return;
	}

	if (!is_number(argument))
	{
		send_to_char( "A que pagina te quieres cambiar?\n\r", ch );
		return;
	}

	num = atoi(argument);

	if ( num <= 0 )
	{
		send_to_char( "No seas estupido.\n\r", ch );
		return;
	}

	ch->desc->pagina = num;

	InitScreen(ch->desc);
	UpdateOLCScreen(ch->desc);

	send_to_char( "Pagina cambiada. Si no ves nada, cambiate a otra.\n\r", ch );
	return;
}
