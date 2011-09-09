
/***************************************************************************
 *  File: olc_act.c                                                        *
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
#include "recycle.h"
#include "lookup.h"
#include "interp.h"

void recalc( MOB_INDEX_DATA * );
COMMAND( do_clear )

/* Return TRUE if area changed, FALSE if not. */
#define REDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define OEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define MEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define AEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )

struct olc_help_type
{
    char *command;
    const void *structure;
    char *desc;
};



bool show_version( CHAR_DATA *ch, char *argument )
{
    send_to_char( OLC_VERSION, ch );
    send_to_char( "\n\r", ch );
    send_to_char( OLC_AUTHOR, ch );
    send_to_char( "\n\r", ch );
    send_to_char( OLC_DATE, ch );
    send_to_char( "\n\r", ch );
    send_to_char( OLC_CREDITS, ch );
    send_to_char( "\n\r", ch );

    return FALSE;
}    

/*
 * This table contains help commands and a brief description of each.
 * ------------------------------------------------------------------
 */
const struct olc_help_type help_table[] =
{
    {	"area",		area_flags,	"Atributos de Area."		},
    {	"room",		room_flags,	"Atributos de Cuartos."		},
    {	"sector",	sector_flags,	"Tipos de Sector(terreno)."	},
    {	"exit",		exit_flags,	"Tipos de Salidas."		},
    {	"type",		type_flags,	"Tipos de Objetos."		},
    {	"extra",	extra_flags,	"Atributos de Objetos."		},
    {	"wear",		wear_flags,	"Where to wear object."		},
    {	"spec",		spec_table,	"Procedimientos especiales."	},
    {	"sex",		sex_table,	"Sexos."			},
    {	"act",		act_flags,	"Atributos de Mobs."		},
    {	"affect",	affect_flags,	"Affects de Mobs."		},
    {	"wear-loc",	wear_loc_flags,	"Donde un mob lleva objetos."	},
    {	"spells",	NULL,		"Nombres de Spells."		},
    {	"container",	container_flags,"Container status."		},
    {	"armor",	ac_type,	"Ac para ataques diferentes."	},
    {   "apply",	apply_flags,	"Tipos de Applys."		},
    {	"form",		form_flags,	"Forma del cuerpo."		},
    {	"part",		part_flags,	"Partes del cuerpo."		},
    {	"imm",		imm_flags,	"Tipos de Inmunidad."		},
    {	"res",		res_flags,	"Tipos de Resistencia."		},
    {	"vuln",		vuln_flags,	"Tipos de Vulnerabilidad."	},
    {	"off",		off_flags,	"Comportamiento ofensivo mob."	},
    {	"size",		size_table,	"Tabla de tamanos."		},
    {   "wclass",       weapon_class,   "Clases de Armas."		}, 
    {   "wtype",        weapon_type2,   "Tipos especiales de Armas."	},
    {	"portal",	portal_flags,	"Tipos de Portales."		},
    {	"furniture",	furniture_flags,"Tipos de Muebles."		},
    {	"liquid",	liq_table,	"Tipos de liquidos."		},
    {	"damtype",	attack_table,	"Tipos de Armas."		},
    {	"weapon",	attack_table,	NULL				},
    {   "position",	position_table,	"Posiciones."			},
    {	"mprog",	mprog_flags,	"Tipos de Mob Progs."		},
    {	"apptype",	apply_types,	"Tipos de Applys (2)."		},
    {	"target",	target_table,	"Objetivos de spells."		},
    {	"damclass",	dam_classes,	"Clases de danos."		},
    {	"log",		log_flags,	"Tipos de log."			},
    {	"show",		show_flags,	"Tipos de comandos."		},
    {	NULL,		NULL,		NULL				}
};



/*****************************************************************************
 Name:		show_flag_cmds
 Purpose:	Displays settable flags and stats.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds( CHAR_DATA *ch, const struct flag_type *flag_table )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  flag;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
	if ( flag_table[flag].settable )
	{
	    sprintf( buf, "%-19.18s", flag_table[flag].name );
	    strcat( buf1, buf );
	    if ( ++col % 4 == 0 )
		strcat( buf1, "\n\r" );
	}
    }
 
    if ( col % 4 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}


/*****************************************************************************
 Name:		show_skill_cmds
 Purpose:	Displays all skill functions.
 		Does remove those damn immortal commands from the list.
 		Could be improved by:
 		(1) Adding a check for a particular class.
 		(2) Adding a check for a level range.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds( CHAR_DATA *ch, int tar )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH*2 ];
    int  sn;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
	if ( !skill_table[sn].name )
	    break;

	if ( !str_cmp( skill_table[sn].name, "reserved" )
	  || skill_table[sn].spell_fun == spell_null )
	    continue;

	if ( tar == -1 || skill_table[sn].target == tar )
	{
	    sprintf( buf, "%-19.18s", skill_table[sn].name );
	    strcat( buf1, buf );
	    if ( ++col % 4 == 0 )
		strcat( buf1, "\n\r" );
	}
    }
 
    if ( col % 4 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}



/*****************************************************************************
 Name:		show_spec_cmds
 Purpose:	Displays settable special functions.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_spec_cmds( CHAR_DATA *ch )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  spec;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    send_to_char( "Preceed special functions with 'spec_'\n\r\n\r", ch );
    for (spec = 0; spec_table[spec].function != NULL; spec++)
    {
	sprintf( buf, "%-19.18s", &spec_table[spec].name[5] );
	strcat( buf1, buf );
	if ( ++col % 4 == 0 )
	    strcat( buf1, "\n\r" );
    }
 
    if ( col % 4 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}

/*****************************************************************************
 Name:		show_help
 Purpose:	Displays help for many tables used in OLC.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_help( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char spell[MAX_INPUT_LENGTH];
    int cnt;

    argument = one_argument( argument, arg );
    one_argument( argument, spell );

    /*
     * Display syntax.
     */
    if ( arg[0] == '\0' )
    {
	int blah = 0;

	send_to_char( "Syntax:  ? [command]\n\r\n\r", ch );
	sprintf( buf, "%-9.9s   %-26.26s %-9.9s   %-26.26s\n\r",
		"[comando]",	"[descripcion]",
		"[comando]",	"[descripcion]" );
	send_to_char( buf, ch );

	for (cnt = 0; help_table[cnt].command != NULL; cnt++)
		if ( help_table[cnt].desc )
		{
			sprintf( buf, "#B%-9.9s#b - %-26.26s",
				capitalize( help_table[cnt].command ),
				help_table[cnt].desc );
			if ( blah % 2 == 1 )
				strcat( buf, "\n\r" );
			else
				strcat( buf, " " );
			send_to_char( buf, ch );
			blah++;
		}

	if ( blah % 2 == 1 )
		send_to_char( "\n\r", ch);

	return FALSE;
    }

    /*
     * Find the command, show changeable data.
     * ---------------------------------------
     */
    for (cnt = 0; help_table[cnt].command != NULL; cnt++)
    {
        if (  arg[0] == help_table[cnt].command[0]
          && !str_prefix( arg, help_table[cnt].command ) )
	{
	    if ( help_table[cnt].structure == spec_table )
	    {
		show_spec_cmds( ch );
		return FALSE;
	    }
	    else
	    if ( help_table[cnt].structure == liq_table )
	    {
	        show_liqlist( ch );
	        return FALSE;
	    }
	    else
	    if ( help_table[cnt].structure == attack_table )
	    {
	        show_damlist( ch );
	        return FALSE;
	    }
	    else
	    if ( help_table[cnt].structure == position_table )
	    {
	        show_poslist( ch );
	        return FALSE;
	    }
	    else
	    if ( help_table[cnt].structure == sex_table )
	    {
	        show_sexlist( ch );
	        return FALSE;
	    }
	    else
	    if ( help_table[cnt].structure == size_table )
	    {
	        show_sizelist( ch );
	        return FALSE;
	    }
	    else
	    if ( !str_prefix(arg, "spells") && help_table[cnt].structure == NULL )
	    {

		if ( spell[0] == '\0' )
		{
		    send_to_char( "Syntax:  ? spells "
		        "[ignore/attack/defend/self/object/all]\n\r", ch );
		    return FALSE;
		}

		if ( !str_prefix( spell, "all" ) )
		    show_skill_cmds( ch, -1 );
		else if ( !str_prefix( spell, "ignore" ) )
		    show_skill_cmds( ch, TAR_IGNORE );
		else if ( !str_prefix( spell, "attack" ) )
		    show_skill_cmds( ch, TAR_CHAR_OFFENSIVE );
		else if ( !str_prefix( spell, "defend" ) )
		    show_skill_cmds( ch, TAR_CHAR_DEFENSIVE );
		else if ( !str_prefix( spell, "self" ) )
		    show_skill_cmds( ch, TAR_CHAR_SELF );
		else if ( !str_prefix( spell, "object" ) )
		    show_skill_cmds( ch, TAR_OBJ_INV );
		else
		    send_to_char( "Syntax:  ? spell "
		        "[ignore/attack/defend/self/object/all]\n\r", ch );
		    
		return FALSE;
	    }
	    else
	    {
		show_flag_cmds( ch, help_table[cnt].structure );
		return FALSE;
	    }
	}
    }

    show_help( ch, "" );
    return FALSE;
}

REDIT( redit_rlist )
{
    ROOM_INDEX_DATA	*pRoomIndex;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH   ];
    BUFFER		*buf1;
    char		arg  [ MAX_INPUT_LENGTH    ];
    bool found;
    int vnum;
    int  col = 0;

    one_argument( argument, arg );

    pArea = ch->in_room->area;
    buf1=new_buf();
/*    buf1[0] = '\0'; */
    found   = FALSE;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pRoomIndex = get_room_index( vnum ) ) )
	{
		found = TRUE;
		sprintf( buf, "[%5d] %-17.16s#n",
		    vnum, capitalize( pRoomIndex->name ) );
		add_buf( buf1, buf );
		if ( ++col % 3 == 0 )
		    add_buf( buf1, "\n\r" );
	}
    }

    if ( !found )
    {
	send_to_char( "Room(s) not found in this area.\n\r", ch);
	return FALSE;
    }

    if ( col % 3 != 0 )
	add_buf( buf1, "\n\r" );

    page_to_char( buf_string(buf1), ch );
    free_buf(buf1);
    return FALSE;
}

REDIT( redit_mlist )
{
    MOB_INDEX_DATA	*pMobIndex;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH   ];
    BUFFER		*buf1;
    char		arg  [ MAX_INPUT_LENGTH    ];
    bool fAll, found;
    int vnum;
    int  col = 0;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax:  mlist <all/name>\n\r", ch );
	return FALSE;
    }

    buf1=new_buf();
    pArea = ch->in_room->area;
/*    buf1[0] = '\0'; */
    fAll    = !str_cmp( arg, "all" );
    found   = FALSE;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    if ( fAll || is_name( arg, pMobIndex->player_name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %-17.16s",
		    pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
		add_buf( buf1, buf );
		if ( ++col % 3 == 0 )
		    add_buf( buf1, "\n\r" );
	    }
	}
    }

    if ( !found )
    {
	send_to_char( "Mobile(s) not found in this area.\n\r", ch);
	return FALSE;
    }

    if ( col % 3 != 0 )
	add_buf( buf1, "\n\r" );

    page_to_char( buf_string(buf1), ch );
    free_buf(buf1);
    return FALSE;
}


/*****************************************************************************
 Name:		check_range( lower vnum, upper vnum )
 Purpose:	Ensures the range spans only one area.
 Called by:	aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range( int lower, int upper )
{
    AREA_DATA *pArea;
    int cnt = 0;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
	/*
	 * lower < area < upper
	 */
        if ( ( lower <= pArea->min_vnum && pArea->min_vnum <= upper )
	||   ( lower <= pArea->max_vnum && pArea->max_vnum <= upper ) )
	    ++cnt;

	if ( cnt > 1 )
	    return FALSE;
    }
    return TRUE;
}



AREA_DATA *get_vnum_area( int vnum )
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        if ( vnum >= pArea->min_vnum
          && vnum <= pArea->max_vnum )
            return pArea;
    }

    return 0;
}



/*
 * Area Editor Functions.
 */
AEDIT( aedit_show )
{
    AREA_DATA *pArea;
    char buf  [MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    sprintf( buf, "Name:     [%5d] %s\n\r", pArea->vnum, pArea->name );
    send_to_char( buf, ch );

#if 0  /* ROM OLC */
    sprintf( buf, "Recall:   [%5d] %s\n\r", pArea->recall,
	get_room_index( pArea->recall )
	? get_room_index( pArea->recall )->name : "none" );
    send_to_char( buf, ch );
#endif /* ROM */

    sprintf( buf, "File:     %s\n\r", pArea->file_name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnums:    [%d-%d]\n\r", pArea->min_vnum, pArea->max_vnum );
    send_to_char( buf, ch );

    sprintf( buf, "Age:      [%d]\n\r",	pArea->age );
    send_to_char( buf, ch );

    sprintf( buf, "Players:  [%d]\n\r", pArea->nplayer );
    send_to_char( buf, ch );

    sprintf( buf, "Security: [%d]\n\r", pArea->security );
    send_to_char( buf, ch );

    sprintf( buf, "Builders: [%s]\n\r", pArea->builders );
    send_to_char( buf, ch );

    sprintf( buf, "Credits : [%s]\n\r", pArea->credits );
    send_to_char( buf, ch );

    sprintf( buf, "Lo range: [%d]\n\r", pArea->low_range );
    send_to_char( buf, ch );
    
    sprintf( buf, "Hi range: [%d]\n\r", pArea->high_range );
    send_to_char( buf, ch );

    sprintf( buf, "Flags:    [%s]\n\r", flag_string( area_flags, pArea->area_flags ) );
    send_to_char( buf, ch );

    return FALSE;
}



AEDIT( aedit_reset )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    reset_area( pArea );
    send_to_char( "Area reset.\n\r", ch );

    return FALSE;
}



AEDIT( aedit_create )
{
    AREA_DATA *pArea;

    if ( IS_NPC(ch) || ch->pcdata->security < 9 )
    {
    	send_to_char( "Insuficiente seguridad para crear areas.\n\r", ch );
    	return FALSE;
    }

    pArea               =   new_area();
    area_last->next     =   pArea;
    area_last		=   pArea;	/* Thanks, Walker. */

    set_editor(ch->desc, ED_AREA, pArea);
/*    ch->desc->pEdit     =   (void *)pArea; */

    SET_BIT( pArea->area_flags, AREA_ADDED );
    send_to_char( "Area Created.\n\r", ch );
    return TRUE;
}



AEDIT( aedit_name )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:   name [$name]\n\r", ch );
	return FALSE;
    }

    free_string( pArea->name );
    pArea->name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch );
    return TRUE;
}

AEDIT( aedit_credits )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:   credits [$credits]\n\r", ch );
	return FALSE;
    }

    free_string( pArea->credits );
    pArea->credits = str_dup( argument );

    send_to_char( "Credits set.\n\r", ch );
    return TRUE;
}


AEDIT( aedit_file )
{
    AREA_DATA *pArea;
    char file[MAX_STRING_LENGTH];
    int i, length;

    EDIT_AREA(ch, pArea);

    one_argument( argument, file );	/* Forces Lowercase */

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  filename [$file]\n\r", ch );
	return FALSE;
    }

    /*
     * Simple Syntax Check.
     */
    length = strlen( argument );
    if ( length > 8 )
    {
	send_to_char( "No more than eight characters allowed.\n\r", ch );
	return FALSE;
    }
    
    /*
     * Allow only letters and numbers.
     */
    for ( i = 0; i < length; i++ )
    {
	if ( !isalnum( file[i] ) )
	{
	    send_to_char( "Only letters and numbers are valid.\n\r", ch );
	    return FALSE;
	}
    }    

    free_string( pArea->file_name );
    strcat( file, ".are" );
    pArea->file_name = str_dup( file );

    send_to_char( "Filename set.\n\r", ch );
    return TRUE;
}

AEDIT( aedit_lowrange )
{
	AREA_DATA *pArea;
	int low_r;
	
	EDIT_AREA(ch, pArea);
	
	if ( argument[0] == '\0' || !is_number(argument) )
	{
		send_to_char( "Sintaxis : lowrange [nivel-minimo]\n\r", ch );
		return FALSE;
	}
	
	low_r = atoi(argument);
	
	if ( low_r < 0 || low_r > MAX_LEVEL )
	{
		send_to_char( "Valores entre 0 y MAX_LEVEL solamente.\n\r", ch );
		return FALSE;
	}

	if ( low_r > pArea->high_range )
	{
		send_to_char( "Argumento es mayor que high_range.\n\r", ch );
		return FALSE;
	}

	pArea->low_range = low_r;
	
	send_to_char( "Nivel minimo seteado.\n\r", ch );
	return TRUE;
}

AEDIT( aedit_highrange )
{
	AREA_DATA *pArea;
	int high_r;
	
	EDIT_AREA(ch, pArea);
	
	if ( argument[0] == '\0' || !is_number(argument) )
	{
		send_to_char( "Sintaxis : highrange [nivel-maximo]\n\r", ch );
		return FALSE;
	}
	
	high_r = atoi(argument);
	
	if ( high_r < 0 || high_r > MAX_LEVEL )
	{
		send_to_char( "Valores entre 0 y MAX_LEVEL solamente.\n\r", ch );
		return FALSE;
	}

	if ( high_r < pArea->low_range )
	{
		send_to_char( "Argumento es menor que low_range.\n\r", ch );
		return FALSE;
	}

	pArea->high_range = high_r;
	
	send_to_char( "Nivel maximo seteado.\n\r", ch );
	return TRUE;
}

AEDIT( aedit_age )
{
    AREA_DATA *pArea;
    char age[MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    one_argument( argument, age );

    if ( !is_number( age ) || age[0] == '\0' )
    {
	send_to_char( "Syntax:  age [#xage]\n\r", ch );
	return FALSE;
    }

    pArea->age = atoi( age );

    send_to_char( "Age set.\n\r", ch );
    return TRUE;
}


#if 0 /* ROM OLC */
AEDIT( aedit_recall )
{
    AREA_DATA *pArea;
    char room[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, room );

    if ( !is_number( argument ) || argument[0] == '\0' )
    {
	send_to_char( "Syntax:  recall [#xrvnum]\n\r", ch );
	return FALSE;
    }

    value = atoi( room );

    if ( !get_room_index( value ) )
    {
	send_to_char( "AEdit:  Room vnum does not exist.\n\r", ch );
	return FALSE;
    }

    pArea->recall = value;

    send_to_char( "Recall set.\n\r", ch );
    return TRUE;
}
#endif /* ROM OLC */


AEDIT( aedit_security )
{
    AREA_DATA *pArea;
    char sec[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, sec );

    if ( !is_number( sec ) || sec[0] == '\0' )
    {
	send_to_char( "Syntax:  security [#xlevel]\n\r", ch );
	return FALSE;
    }

    value = atoi( sec );

    if ( value > ch->pcdata->security || value < 0 )
    {
	if ( ch->pcdata->security != 0 )
	{
	    sprintf( buf, "Security is 0-%d.\n\r", ch->pcdata->security );
	    send_to_char( buf, ch );
	}
	else
	    send_to_char( "Security is 0 only.\n\r", ch );
	return FALSE;
    }

    pArea->security = value;

    send_to_char( "Security set.\n\r", ch );
    return TRUE;
}



AEDIT( aedit_builder )
{
    AREA_DATA *pArea;
    char name[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    first_arg( argument, name, FALSE );

    if ( name[0] == '\0' )
    {
	send_to_char( "Syntax:  builder [$name]  -toggles builder\n\r", ch );
	send_to_char( "Syntax:  builder All      -allows everyone\n\r", ch );
	return FALSE;
    }

    name[0] = UPPER( name[0] );

    if ( strstr( pArea->builders, name ) != '\0' )
    {
	pArea->builders = string_replace( pArea->builders, name, "\0" );
	pArea->builders = string_unpad( pArea->builders );

	if ( pArea->builders[0] == '\0' )
	{
	    free_string( pArea->builders );
	    pArea->builders = str_dup( "None" );
	}
	send_to_char( "Builder removed.\n\r", ch );
	return TRUE;
    }
    else
    {
	buf[0] = '\0';
	if ( strstr( pArea->builders, "None" ) != '\0' )
	{
	    pArea->builders = string_replace( pArea->builders, "None", "\0" );
	    pArea->builders = string_unpad( pArea->builders );
	}

	if (pArea->builders[0] != '\0' )
	{
	    strcat( buf, pArea->builders );
	    strcat( buf, " " );
	}
	strcat( buf, name );
	free_string( pArea->builders );
	pArea->builders = string_proper( str_dup( buf ) );

	send_to_char( "Builder added.\n\r", ch );
	send_to_char( pArea->builders,ch);
	send_to_char( "\n\r", ch);
	return TRUE;
    }

    return FALSE;
}



AEDIT( aedit_vnum )
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    argument = one_argument( argument, lower );
    one_argument( argument, upper );

    if ( !is_number( lower ) || lower[0] == '\0'
    || !is_number( upper ) || upper[0] == '\0' )
    {
	send_to_char( "Syntax:  vnum [#xlower] [#xupper]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = atoi( upper ) ) )
    {
	send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
	return FALSE;
    }
    
    if ( !check_range( atoi( lower ), atoi( upper ) ) )
    {
	send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( ilower )
    && get_vnum_area( ilower ) != pArea )
    {
	send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->min_vnum = ilower;
    send_to_char( "Lower vnum set.\n\r", ch );

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
	send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
	return TRUE;	/* The lower value has been set. */
    }

    pArea->max_vnum = iupper;
    send_to_char( "Upper vnum set.\n\r", ch );

    return TRUE;
}



AEDIT( aedit_lvnum )
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, lower );

    if ( !is_number( lower ) || lower[0] == '\0' )
    {
	send_to_char( "Syntax:  min_vnum [#xlower]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = pArea->max_vnum ) )
    {
	send_to_char( "AEdit:  Value must be less than the max_vnum.\n\r", ch );
	return FALSE;
    }
    
    if ( !check_range( ilower, iupper ) )
    {
	send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( ilower )
    && get_vnum_area( ilower ) != pArea )
    {
	send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->min_vnum = ilower;
    send_to_char( "Lower vnum set.\n\r", ch );
    return TRUE;
}



AEDIT( aedit_uvnum )
{
    AREA_DATA *pArea;
    char upper[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, upper );

    if ( !is_number( upper ) || upper[0] == '\0' )
    {
	send_to_char( "Syntax:  max_vnum [#xupper]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = pArea->min_vnum ) > ( iupper = atoi( upper ) ) )
    {
	send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
	return FALSE;
    }
    
    if ( !check_range( ilower, iupper ) )
    {
	send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
	send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->max_vnum = iupper;
    send_to_char( "Upper vnum set.\n\r", ch );

    return TRUE;
}



/*
 * Room Editor Functions.
 */
REDIT( redit_show )
{
    ROOM_INDEX_DATA	*pRoom;
    char		buf  [MAX_STRING_LENGTH];
    char		buf1 [2*MAX_STRING_LENGTH];
    OBJ_DATA		*obj;
    CHAR_DATA		*rch;
    int			cnt = 0;
    bool		fcnt;
    
    EDIT_ROOM(ch, pRoom);

    buf1[0] = '\0';
    
    sprintf( buf, "Description:\n\r%s", pRoom->description );
    strcat( buf1, buf );

    sprintf( buf, "Name:       [%s]\n\rArea:       [%5d] %s\n\r",
	    pRoom->name, pRoom->area->vnum, pRoom->area->name );
    strcat( buf1, buf );

    sprintf( buf, "Vnum:       [%5d]\n\rSector:     [%s]\n\r",
	    pRoom->vnum, flag_string( sector_flags, pRoom->sector_type ) );
    strcat( buf1, buf );

    sprintf( buf, "Room flags: [%s]\n\r",
	    flag_string( room_flags, pRoom->room_flags ) );
    strcat( buf1, buf );

    sprintf( buf, "Heal rec  : [%d]\n\rMana rec  : [%d]\n\r",
            pRoom->heal_rate , pRoom->mana_rate );
    strcat( buf1, buf );
        
    if ( pRoom->clan )
    {
	sprintf( buf, "Clan      : [%d] %s\n\r" , pRoom->clan ,
	((pRoom->clan > 0) ? clan_table[pRoom->clan].name : "none" ));
	strcat( buf1, buf );
    }
    
    if ( pRoom->owner && pRoom->owner[0] != '\0' )
    {
	sprintf( buf, "Owner     : [%s]\n\r", pRoom->owner );
	strcat( buf1, buf );
    }
    
    if ( pRoom->extra_descr )
    {
	EXTRA_DESCR_DATA *ed;

	strcat( buf1, "Desc Kwds:  [" );
	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    strcat( buf1, ed->keyword );
	    if ( ed->next )
		strcat( buf1, " " );
	}
	strcat( buf1, "]\n\r" );
    }

    strcat( buf1, "Characters: [" );
    fcnt = FALSE;
    for ( rch = pRoom->people; rch; rch = rch->next_in_room )
	if (IS_NPC(rch) || can_see(ch,rch))
	{
		one_argument( rch->name, buf );
		strcat( buf1, buf );
		strcat( buf1, " " );
		fcnt = TRUE;
	}

    if ( fcnt )
    {
	int end;

	end = strlen(buf1) - 1;
	buf1[end] = ']';
	strcat( buf1, "\n\r" );
    }
    else
	strcat( buf1, "none]\n\r" );

    strcat( buf1, "Objects:    [" );
    fcnt = FALSE;
    for ( obj = pRoom->contents; obj; obj = obj->next_content )
    {
	one_argument( obj->name, buf );
	strcat( buf1, buf );
	strcat( buf1, " " );
	fcnt = TRUE;
    }

    if ( fcnt )
    {
	int end;

	end = strlen(buf1) - 1;
	buf1[end] = ']';
	strcat( buf1, "\n\r" );
    }
    else
	strcat( buf1, "none]\n\r" );

    for ( cnt = 0; cnt < MAX_DIR; cnt++ )
    {
	    char word[MAX_INPUT_LENGTH];
	    char reset_state[MAX_STRING_LENGTH];
	    char *state;
	    int i, length;

	    if (pRoom->exit[cnt] == NULL)
	    	continue;

	    sprintf( buf, "-%-5s to [%5d] Key: [%5d] ",
		capitalize(dir_name[cnt]),
		pRoom->exit[cnt]->u1.to_room ? pRoom->exit[cnt]->u1.to_room->vnum : 0,      /* ROM OLC */
		pRoom->exit[cnt]->key );
	    strcat( buf1, buf );

	    /*
	     * Format up the exit info.
	     * Capitalize all flags that are not part of the reset info.
	     */
	    strcpy( reset_state, flag_string( exit_flags, pRoom->exit[cnt]->rs_flags ) );
	    state = flag_string( exit_flags, pRoom->exit[cnt]->exit_info );
	    strcat( buf1, " Exit flags: [" );
	    for (; ;)
	    {
		state = one_argument( state, word );

		if ( word[0] == '\0' )
		{
		    int end;

		    end = strlen(buf1) - 1;
		    buf1[end] = ']';
		    strcat( buf1, "\n\r" );
		    break;
		}

		if ( str_infix( word, reset_state ) )
		{
		    length = strlen(word);
		    for (i = 0; i < length; i++)
			word[i] = UPPER(word[i]);
		}
		strcat( buf1, word );
		strcat( buf1, " " );
	    }

	    if ( pRoom->exit[cnt]->keyword && pRoom->exit[cnt]->keyword[0] != '\0' )
	    {
		sprintf( buf, "Kwds: [%s]\n\r", pRoom->exit[cnt]->keyword );
		strcat( buf1, buf );
	    }
	    if ( pRoom->exit[cnt]->description && pRoom->exit[cnt]->description[0] != '\0' )
	    {
		sprintf( buf, "%s", pRoom->exit[cnt]->description );
		strcat( buf1, buf );
	    }
    }

    send_to_char( buf1, ch );
    return FALSE;
}

/* Local function. */
bool change_exit( CHAR_DATA *ch, char *argument, int door )
{
    ROOM_INDEX_DATA *pRoom;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int  value;

    EDIT_ROOM(ch, pRoom);

    /*
     * Set the exit flags, needs full argument.
     * ----------------------------------------
     */
    if ( ( value = flag_value( exit_flags, argument ) ) != NO_FLAG )
    {
	ROOM_INDEX_DATA *pToRoom;
	EXIT_DATA *pExit, *pNExit;
	sh_int rev;                                    /* ROM OLC */

	pExit = pRoom->exit[door];

	if ( !pExit )
	{
		send_to_char("Salida no existe.\n\r",ch);
		return FALSE;
	}

	/*
	 * This room.
	 */
	TOGGLE_BIT(pExit->rs_flags,value);

	/* Don't toggle exit_info because it can be changed by players. */
	pExit->exit_info = pExit->rs_flags;

	/*
	 * Connected room.
	 */
	pToRoom	= pExit->u1.to_room;     /* ROM OLC */
	rev	= rev_dir[door];
	pNExit	= pToRoom->exit[rev];

	if (pNExit)
	{
		TOGGLE_BIT(pNExit->rs_flags,value);
		pNExit->exit_info = pNExit->rs_flags;
	}

	send_to_char( "Exit flag toggled.\n\r", ch );
	return TRUE;
    }

    /*
     * Now parse the arguments.
     */
    argument = one_argument( argument, command );
    one_argument( argument, arg );

    if ( command[0] == '\0' && argument[0] == '\0' )	/* Move command. */
    {
	move_char( ch, door, TRUE );                    /* ROM OLC */
	return FALSE;
    }

    if ( command[0] == '?' )
    {
	do_help( ch, "EXIT" );
	return FALSE;
    }

    if ( !str_cmp( command, "delete" ) )
    {
	ROOM_INDEX_DATA *pToRoom;
	EXIT_DATA *pExit, *pNExit;
	sh_int rev;
	bool rDeleted = FALSE;
	
	pExit = pRoom->exit[door];

	if ( !pExit )
	{
	    send_to_char( "REdit:  Cannot delete a null exit.\n\r", ch );
	    return FALSE;
	}

	pToRoom = pExit->u1.to_room;

	/*
	 * Remove ToRoom Exit.
	 */
	if ( str_cmp( arg, "simple" ) && pToRoom )
	{
		rev	= rev_dir[door];
		pNExit	= pToRoom->exit[rev];

		if ( pNExit )
		{
			if ( pNExit->u1.to_room == pRoom )
			{
				rDeleted = TRUE;
				free_exit( pToRoom->exit[rev] );
				pToRoom->exit[rev] = NULL;
			}
			else
				printf_to_char( ch, "Salida %d del cuarto %d no apunta hacia aqui, asi que no fue borrada.\n\r",
					rev, pToRoom->vnum );
		}
	}

	/*
	 * Remove this exit.
	 */
	printf_to_char( ch, "Salida %s del cuarto %d borrada.\n\r",
		dir_name[door], pRoom->vnum );
	free_exit( pRoom->exit[door] );
	pRoom->exit[door] = NULL;

	if (rDeleted)
		printf_to_char( ch, "Salida %s del cuarto %d tambien fue borrada.\n\r",
			dir_name[rev_dir[door]], pToRoom->vnum );

	return TRUE;
    }

    if ( !str_cmp( command, "link" ) )
    {
	EXIT_DATA *pExit;
	ROOM_INDEX_DATA *pRoomIndex;

	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char( "Syntax:  [direction] link [vnum]\n\r", ch );
	    return FALSE;
	}

	pRoomIndex = get_room_index( atoi(arg) );

	if ( !pRoomIndex )
	{
	    send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
	    return FALSE;
	}

	if ( !IS_BUILDER(ch, pRoomIndex->area) )
	{
	    send_to_char( "REdit:  Cannot link to that area.\n\r", ch );
	    return FALSE;
	}

	pExit = pRoom->exit[door];

	if ( pExit )
	{
		send_to_char( "REdit : Salida ya existe.\n\r", ch );
		return FALSE;
	}

	pExit = pRoomIndex->exit[rev_dir[door]];

	if ( pExit )
	{
	    send_to_char( "REdit:  Remote side's exit already exists.\n\r", ch );
	    return FALSE;
	}

	pExit			= new_exit();
	pExit->u1.to_room	= pRoomIndex;
	pExit->orig_door	= door;
	pRoom->exit[door] = pExit;

	/* ahora el otro lado */
	door                    = rev_dir[door];
	pExit                   = new_exit();
	pExit->u1.to_room       = pRoom;
	pExit->orig_door	= door;
	pRoomIndex->exit[door]	= pExit;

	SET_BIT(pRoom->area->area_flags, AREA_CHANGED);

	send_to_char( "Two-way link established.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "dig" ) )
    {
	char buf[MAX_STRING_LENGTH];
	
	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char( "Syntax: [direction] dig <vnum>\n\r", ch );
	    return FALSE;
	}
	
	redit_create( ch, arg );
	sprintf( buf, "link %s", arg );
	change_exit( ch, buf, door);
	return TRUE;
    }

    if ( !str_cmp( command, "room" ) )
    {
	EXIT_DATA *pExit;
	ROOM_INDEX_DATA *target;

	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char( "Syntax:  [direction] room [vnum]\n\r", ch );
	    return FALSE;
	}

	value = atoi( arg );

	if ( (target = get_room_index( value )) == NULL )
	{
	    send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
	    return FALSE;
	}

	if ( !IS_BUILDER(ch, target->area) )
	{
		send_to_char( "REdit: Destino en un area que no puedes editar.\n\r", ch );
		return FALSE;
	}

	if ( (pExit = pRoom->exit[door]) == NULL )
	{
		pExit	= new_exit();
		pRoom->exit[door] = pExit;
	}

	pExit->u1.to_room	= target;
	pExit->orig_door	= door;

	if ( (pExit = target->exit[rev_dir[door]]) != NULL
	&&    pExit->u1.to_room != pRoom )
		printf_to_char( ch, "#BADVERTENCIA#b : la salida del cuarto %d no apunta hacia aqui.\n\r",
			target->vnum );

	send_to_char( "One-way link established.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "key" ) )
    {
	EXIT_DATA *pExit;
	OBJ_INDEX_DATA *pObj;

	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char( "Syntax:  [direction] key [vnum]\n\r", ch );
	    return FALSE;
	}

	if ( (pExit = pRoom->exit[door]) == NULL )
	{
		send_to_char("Salida no existe.\n\r",ch);
		return FALSE;
	}

	pObj = get_obj_index(atoi(arg));

	if ( !pObj )
	{
	    send_to_char( "REdit:  Item doesn't exist.\n\r", ch );
	    return FALSE;
	}

	if ( pObj->item_type != ITEM_KEY )
	{
	    send_to_char( "REdit:  Key doesn't exist.\n\r", ch );
	    return FALSE;
	}

	pExit->key = atoi(arg);

	send_to_char( "Exit key set.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "name" ) )
    {
	EXIT_DATA *pExit;

	if ( arg[0] == '\0' )
	{
	    send_to_char( "Syntax:  [direction] name [string]\n\r", ch );
	    send_to_char( "         [direction] name none\n\r", ch );
	    return FALSE;
	}

	if ( (pExit = pRoom->exit[door]) == NULL )
	{
		send_to_char("Salida no existe.\n\r",ch);
   		return FALSE;
	}

	free_string( pExit->keyword );

	if (str_cmp(arg,"none"))
		pExit->keyword = str_dup( arg );
	else
		pExit->keyword = str_dup( "" );

	send_to_char( "Exit name set.\n\r", ch );
	return TRUE;
    }

    if ( !str_prefix( command, "description" ) )
    {
	EXIT_DATA *pExit;

	if ( arg[0] == '\0' )
	{
	    if ( (pExit = pRoom->exit[door]) == NULL )
	    {
		send_to_char("Salida no existe.\n\r",ch);
   		return FALSE;
	    }

	    string_append( ch, &pExit->description );
	    return TRUE;
	}

	send_to_char( "Syntax:  [direction] desc\n\r", ch );
	return FALSE;
    }

    return FALSE;
}

REDIT( redit_create )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int value;
    int iHash;
    
    EDIT_ROOM(ch, pRoom);

    value = atoi( argument );

    if ( argument[0] == '\0' || value <= 0 )
    {
	send_to_char( "Syntax:  create [vnum > 0]\n\r", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );
    if ( !pArea )
    {
	send_to_char( "REdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "REdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    if ( get_room_index( value ) )
    {
	send_to_char( "REdit:  Room vnum already exists.\n\r", ch );
	return FALSE;
    }

    pRoom			= new_room_index();
    pRoom->area			= pArea;
    pRoom->vnum			= value;
    pRoom->room_flags		= 0;

    if ( value > top_vnum_room )
        top_vnum_room = value;

    iHash			= value % MAX_KEY_HASH;
    pRoom->next			= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoom;

    set_editor(ch->desc, ED_ROOM, pRoom);
/*    ch->desc->pEdit		= (void *)pRoom; */

    send_to_char( "Room created.\n\r", ch );
    return TRUE;
}

REDIT( redit_format )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    pRoom->description = format_string( pRoom->description );

    send_to_char( "String formatted.\n\r", ch );
    return TRUE;
}

REDIT( redit_mreset )
{
    ROOM_INDEX_DATA	*pRoom;
    MOB_INDEX_DATA	*pMobIndex;
    CHAR_DATA		*newmob;
    char		arg [ MAX_INPUT_LENGTH ];
    char		arg2 [ MAX_INPUT_LENGTH ];

    RESET_DATA		*pReset;
    char		output [ MAX_STRING_LENGTH ];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
	send_to_char ( "Syntax:  mreset <vnum> <max #x> <min #x>\n\r", ch );
	return FALSE;
    }

    if ( !( pMobIndex = get_mob_index( atoi( arg ) ) ) )
    {
	send_to_char( "REdit: No mobile has that vnum.\n\r", ch );
	return FALSE;
    }

    if ( pMobIndex->area != pRoom->area )
    {
	send_to_char( "REdit: No such mobile in this area.\n\r", ch );
	return FALSE;
    }

    /*
     * Create the mobile reset.
     */
    pReset              = new_reset_data();
    pReset->command	= 'M';
    pReset->arg1	= pMobIndex->vnum;
    pReset->arg2	= is_number( arg2 ) ? atoi( arg2 ) : MAX_MOB;
    pReset->arg3	= pRoom->vnum;
    pReset->arg4	= is_number( argument ) ? atoi (argument) : 1;
    add_reset( pRoom, pReset, 0/* Last slot*/ );

    /*
     * Create the mobile.
     */
    newmob = create_mobile( pMobIndex );
    char_to_room( newmob, pRoom );

    sprintf( output, "%s (%d) fue cargado y anadido a los resets.\n\r"
	"Habra un maximo de %d en esta area, y %d en el cuarto.\n\r",
	capitalize( pMobIndex->short_descr ),
	pMobIndex->vnum,
	pReset->arg2,
	pReset->arg4 );
    send_to_char( output, ch );
    act( "$n has created $N!", ch, NULL, newmob, TO_ROOM );
    return TRUE;
}



struct wear_type
{
    int	wear_loc;
    int	wear_bit;
};



const struct wear_type wear_table[] =
{
    {	WEAR_NONE,	ITEM_TAKE		},
    {	WEAR_LIGHT,	ITEM_LIGHT		},
    {	WEAR_FINGER_L,	ITEM_WEAR_FINGER	},
    {	WEAR_FINGER_R,	ITEM_WEAR_FINGER	},
    {	WEAR_NECK_1,	ITEM_WEAR_NECK		},
    {	WEAR_NECK_2,	ITEM_WEAR_NECK		},
    {	WEAR_BODY,	ITEM_WEAR_BODY		},
    {	WEAR_HEAD,	ITEM_WEAR_HEAD		},
    {	WEAR_LEGS,	ITEM_WEAR_LEGS		},
    {	WEAR_FEET,	ITEM_WEAR_FEET		},
    {	WEAR_HANDS,	ITEM_WEAR_HANDS		},
    {	WEAR_ARMS,	ITEM_WEAR_ARMS		},
    {	WEAR_SHIELD,	ITEM_WEAR_SHIELD	},
    {	WEAR_ABOUT,	ITEM_WEAR_ABOUT		},
    {	WEAR_WAIST,	ITEM_WEAR_WAIST		},
    {	WEAR_WRIST_L,	ITEM_WEAR_WRIST		},
    {	WEAR_WRIST_R,	ITEM_WEAR_WRIST		},
    {	WEAR_WIELD,	ITEM_WIELD		},
    {	WEAR_HOLD,	ITEM_HOLD		},
    {	NO_FLAG,	NO_FLAG			}
};



/*****************************************************************************
 Name:		wear_loc
 Purpose:	Returns the location of the bit that matches the count.
 		1 = first match, 2 = second match etc.
 Called by:	oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc(int bits, int count)
{
    int flag;
 
    for (flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++)
    {
        if ( IS_SET(bits, wear_table[flag].wear_bit) && --count < 1)
            return wear_table[flag].wear_loc;
    }
 
    return NO_FLAG;
}



/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit(int loc)
{
    int flag;
 
    for (flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++)
    {
        if ( loc == wear_table[flag].wear_loc )
            return wear_table[flag].wear_bit;
    }
 
    return 0;
}



REDIT( redit_oreset )
{
    ROOM_INDEX_DATA	*pRoom;
    OBJ_INDEX_DATA	*pObjIndex;
    OBJ_DATA		*newobj;
    OBJ_DATA		*to_obj;
    CHAR_DATA		*to_mob;
    char		arg1 [ MAX_INPUT_LENGTH ];
    char		arg2 [ MAX_INPUT_LENGTH ];
    int			olevel = 0;

    RESET_DATA		*pReset;
    char		output [ MAX_STRING_LENGTH ];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
	send_to_char ( "Syntax:  oreset <vnum> <args>\n\r", ch );
	send_to_char ( "        -no_args               = into room\n\r", ch );
	send_to_char ( "        -<obj_name>            = into obj\n\r", ch );
	send_to_char ( "        -<mob_name> <wear_loc> = into mob\n\r", ch );
	return FALSE;
    }

    if ( !( pObjIndex = get_obj_index( atoi( arg1 ) ) ) )
    {
	send_to_char( "REdit: No object has that vnum.\n\r", ch );
	return FALSE;
    }

    if ( pObjIndex->area != pRoom->area )
    {
	send_to_char( "REdit: No such object in this area.\n\r", ch );
	return FALSE;
    }

    /*
     * Load into room.
     */
    if ( arg2[0] == '\0' )
    {
	pReset		= new_reset_data();
	pReset->command	= 'O';
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= 0;
	pReset->arg3	= pRoom->vnum;
	pReset->arg4	= 0;
	add_reset( pRoom, pReset, 0/* Last slot*/ );

	newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
	obj_to_room( newobj, pRoom );

	sprintf( output, "%s (%d) ha sido cargado y anadido a los resets.\n\r",
	    capitalize( pObjIndex->short_descr ),
	    pObjIndex->vnum );
	send_to_char( output, ch );
    }
    else
    /*
     * Load into object's inventory.
     */
    if ( argument[0] == '\0'
    && ( ( to_obj = get_obj_list( ch, arg2, pRoom->contents ) ) != NULL ) )
    {
	pReset		= new_reset_data();
	pReset->command	= 'P';
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= 0;
	pReset->arg3	= to_obj->pIndexData->vnum;
	pReset->arg4	= 1;
	add_reset( pRoom, pReset, 0/* Last slot*/ );

	newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
	newobj->cost = 0;
	obj_to_obj( newobj, to_obj );

	sprintf( output, "%s (%d) fue cargado dentro de "
	    "%s (%d) y anadido a los resets.\n\r",
	    capitalize( newobj->short_descr ),
	    newobj->pIndexData->vnum,
	    to_obj->short_descr,
	    to_obj->pIndexData->vnum );
	send_to_char( output, ch );
    }
    else
    /*
     * Load into mobile's inventory.
     */
    if ( ( to_mob = get_char_room( ch, arg2 ) ) != NULL )
    {
	int	wearloc;

	/*
	 * Make sure the location on mobile is valid.
	 */
	if ( (wearloc = flag_value( wear_loc_flags, argument )) == NO_FLAG )
	{
	    send_to_char( "REdit: Invalid wear_loc.  '? wear-loc'\n\r", ch );
	    return FALSE;
	}

	/*
	 * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
	 */
	if ( !IS_SET( pObjIndex->wear_flags, wear_bit(wearloc) ) )
	{
	    sprintf( output,
	        "%s (%d) has wear flags: [%s]\n\r",
	        capitalize( pObjIndex->short_descr ),
	        pObjIndex->vnum,
		flag_string( wear_flags, pObjIndex->wear_flags ) );
	    send_to_char( output, ch );
	    return FALSE;
	}

	/*
	 * Can't load into same position.
	 */
	if ( get_eq_char( to_mob, wearloc ) )
	{
	    send_to_char( "REdit:  Object already equipped.\n\r", ch );
	    return FALSE;
	}

	pReset		= new_reset_data();
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= wearloc;
	if ( pReset->arg2 == WEAR_NONE )
	    pReset->command = 'G';
	else
	    pReset->command = 'E';
	pReset->arg3	= wearloc;

	add_reset( pRoom, pReset, 0/* Last slot*/ );

	olevel  = URANGE( 0, to_mob->level - 2, LEVEL_HERO );
        newobj = create_object( pObjIndex, number_fuzzy( olevel ) );

	if ( to_mob->pIndexData->pShop )	/* Shop-keeper? */
	{
	    switch ( pObjIndex->item_type )
	    {
	    default:		olevel = 0;				break;
	    case ITEM_PILL:	olevel = number_range(  0, 10 );	break;
	    case ITEM_POTION:	olevel = number_range(  0, 10 );	break;
	    case ITEM_SCROLL:	olevel = number_range(  5, 15 );	break;
	    case ITEM_WAND:	olevel = number_range( 10, 20 );	break;
	    case ITEM_STAFF:	olevel = number_range( 15, 25 );	break;
	    case ITEM_ARMOR:	olevel = number_range(  5, 15 );	break;
	    case ITEM_WEAPON:	if ( pReset->command == 'G' )
	    			    olevel = number_range( 5, 15 );
				else
				    olevel = number_fuzzy( olevel );
		break;
	    }

	    newobj = create_object( pObjIndex, olevel );
	    if ( pReset->arg2 == WEAR_NONE )
		SET_BIT( newobj->extra_flags, ITEM_INVENTORY );
	}
	else
	    newobj = create_object( pObjIndex, number_fuzzy( olevel ) );

	obj_to_char( newobj, to_mob );
	if ( pReset->command == 'E' )
	    equip_char( to_mob, newobj, pReset->arg3 );

	sprintf( output, "%s (%d) has been loaded "
	    "%s of %s (%d) and added to resets.\n\r",
	    capitalize( pObjIndex->short_descr ),
	    pObjIndex->vnum,
	    flag_string( wear_loc_strings, pReset->arg3 ),
	    to_mob->short_descr,
	    to_mob->pIndexData->vnum );
	send_to_char( output, ch );
    }
    else	/* Display Syntax */
    {
	send_to_char( "REdit:  That mobile isn't here.\n\r", ch );
	return FALSE;
    }

    act( "$n has created $p!", ch, newobj, NULL, TO_ROOM );
    return TRUE;
}



/*
 * Object Editor Functions.
 */
void show_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *obj )
{
    char buf[MAX_STRING_LENGTH];

    switch( obj->item_type )
    {
	default:	/* No values. */
	    break;
            
	case ITEM_LIGHT:
            if ( obj->value[2] == -1 || obj->value[2] == 999 ) /* ROM OLC */
		sprintf( buf, "[v2] Light:  Infinite[-1]\n\r" );
            else
		sprintf( buf, "[v2] Light:  [%d]\n\r", obj->value[2] );
	    send_to_char( buf, ch );
	    break;

	case ITEM_WAND:
	case ITEM_STAFF:
            sprintf( buf,
		"[v0] Level:          [%d]\n\r"
		"[v1] Charges Total:  [%d]\n\r"
		"[v2] Charges Left:   [%d]\n\r"
		"[v3] Spell:          %s\n\r",
		obj->value[0],
		obj->value[1],
		obj->value[2],
		obj->value[3] != -1 ? skill_table[obj->value[3]].name
		                    : "none" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_PORTAL:
	    {
	    	char buf2[MIL];
	    	
	    	sprintf( buf2, "%s", flag_string( exit_flags, obj->value[1]) );

		sprintf( buf,
			"[v0] Charges:        [%d]\n\r"
			"[v1] Exit Flags:     %s\n\r"
			"[v2] Portal Flags:   %s\n\r"
			"[v3] Goes to (vnum): [%d]\n\r",
			obj->value[0],
			buf2,
			flag_string( portal_flags , obj->value[2]),
			obj->value[3] );
		send_to_char( buf, ch);
	    }
	    break;
	    
	case ITEM_FURNITURE:          
	    sprintf( buf,
	        "[v0] Max people:      [%d]\n\r"
	        "[v1] Max weight:      [%d]\n\r"
	        "[v2] Furniture Flags: %s\n\r"
	        "[v3] Heal bonus:      [%d]\n\r"
	        "[v4] Mana bonus:      [%d]\n\r",
	        obj->value[0],
	        obj->value[1],
	        flag_string( furniture_flags, obj->value[2]),
	        obj->value[3],
	        obj->value[4] );
	    send_to_char( buf, ch );
	    break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
            sprintf( buf,
		"[v0] Level:  [%d]\n\r"
		"[v1] Spell:  %s\n\r"
		"[v2] Spell:  %s\n\r"
		"[v3] Spell:  %s\n\r"
		"[v4] Spell:  %s\n\r",
		obj->value[0],
		obj->value[1] != -1 ? skill_table[obj->value[1]].name
		                    : "none",
		obj->value[2] != -1 ? skill_table[obj->value[2]].name
                                    : "none",
		obj->value[3] != -1 ? skill_table[obj->value[3]].name
		                    : "none",
		obj->value[4] != -1 ? skill_table[obj->value[4]].name
		                    : "none" );
	    send_to_char( buf, ch );
	    break;

/* ARMOR for ROM */

        case ITEM_ARMOR:
	    sprintf( buf,
		"[v0] Ac pierce       [%d]\n\r"
		"[v1] Ac bash         [%d]\n\r"
		"[v2] Ac slash        [%d]\n\r"
		"[v3] Ac exotic       [%d]\n\r",
		obj->value[0],
		obj->value[1],
		obj->value[2],
		obj->value[3] );
	    send_to_char( buf, ch );
	    break;

/* WEAPON changed in ROM: */
/* I had to split the output here, I have no idea why, but it helped -- Hugin */
/* It somehow fixed a bug in showing scroll/pill/potions too ?! */
	case ITEM_WEAPON:
            sprintf( buf, "[v0] Weapon class:   %s\n\r",
		     flag_string( weapon_class, obj->value[0] ) );
	    send_to_char( buf, ch );
	    sprintf( buf, "[v1] Number of dice: [%d]\n\r", obj->value[1] );
	    send_to_char( buf, ch );
	    sprintf( buf, "[v2] Type of dice:   [%d]\n\r", obj->value[2] );
	    send_to_char( buf, ch );
	    sprintf( buf, "[v3] Type:           %s\n\r",
		    attack_table[obj->value[3]].name );
	    send_to_char( buf, ch );
 	    sprintf( buf, "[v4] Special type:   %s\n\r",
		     flag_string( weapon_type2,  obj->value[4] ) );
	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	    sprintf( buf,
		"[v0] Weight:     [%d kg]\n\r"
		"[v1] Flags:      [%s]\n\r"
		"[v2] Key:     %s [%d]\n\r"
		"[v3] Capacity    [%d]\n\r"
		"[v4] Weight Mult [%d]\n\r",
		obj->value[0],
		flag_string( container_flags, obj->value[1] ),
                get_obj_index(obj->value[2])
                    ? get_obj_index(obj->value[2])->short_descr
                    : "none",
                obj->value[2],
                obj->value[3],
                obj->value[4] );
	    send_to_char( buf, ch );
	    break;

	case ITEM_DRINK_CON:
	    sprintf( buf,
	        "[v0] Liquid Total: [%d]\n\r"
	        "[v1] Liquid Left:  [%d]\n\r"
	        "[v2] Liquid:       %s\n\r"
	        "[v3] Poisoned:     %s\n\r",
	        obj->value[0],
	        obj->value[1],
		liq_table[obj->value[2]].liq_name,
	        obj->value[3] != 0 ? "Yes" : "No" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_FOUNTAIN:
	    sprintf( buf,
	        "[v0] Liquid Total: [%d]\n\r"
	        "[v1] Liquid Left:  [%d]\n\r"
	        "[v2] Liquid:       %s\n\r",
	        obj->value[0],
	        obj->value[1],
	        liq_table[obj->value[2]].liq_name );
	    send_to_char( buf,ch );
	    break;
	        
	case ITEM_FOOD:
	    sprintf( buf,
		"[v0] Food hours: [%d]\n\r"
		"[v1] Full hours: [%d]\n\r"
		"[v3] Poisoned:   %s\n\r",
		obj->value[0],
		obj->value[1],
		obj->value[3] != 0 ? "Yes" : "No" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_MONEY:
            sprintf( buf, "[v0] Gold:   [%d]\n\r"
            		  "[v1] Silver: [%d]\n\r",
            obj->value[0],
            obj->value[1] );
	    send_to_char( buf, ch );
	    break;
    }

    return;
}



bool set_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, int value_num, char *argument)
{
    int tmp;

    switch( pObj->item_type )
    {
        default:
            break;
            
        case ITEM_LIGHT:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_LIGHT" );
	            return FALSE;
	        case 2:
	            send_to_char( "HOURS OF LIGHT SET.\n\r\n\r", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	    }
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_STAFF_WAND" );
	            return FALSE;
	        case 0:
	            send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "TOTAL NUMBER OF CHARGES SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "CURRENT NUMBER OF CHARGES SET.\n\r\n\r", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "SPELL TYPE SET.\n\r", ch );
	            pObj->value[3] = skill_lookup( argument );
	            break;
	    }
            break;

        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_SCROLL_POTION_PILL" );
	            return FALSE;
	        case 0:
	            send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "SPELL TYPE 1 SET.\n\r\n\r", ch );
	            pObj->value[1] = skill_lookup( argument );
	            break;
	        case 2:
	            send_to_char( "SPELL TYPE 2 SET.\n\r\n\r", ch );
	            pObj->value[2] = skill_lookup( argument );
	            break;
	        case 3:
	            send_to_char( "SPELL TYPE 3 SET.\n\r\n\r", ch );
	            pObj->value[3] = skill_lookup( argument );
	            break;
	        case 4:
	            send_to_char( "SPELL TYPE 4 SET.\n\r\n\r", ch );
	            pObj->value[4] = skill_lookup( argument );
	            break;
 	    }
	    break;

/* ARMOR for ROM: */

        case ITEM_ARMOR:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_ARMOR" );
		    return FALSE;
	        case 0:
		    send_to_char( "AC PIERCE SET.\n\r\n\r", ch );
		    pObj->value[0] = atoi( argument );
		    break;
	        case 1:
		    send_to_char( "AC BASH SET.\n\r\n\r", ch );
		    pObj->value[1] = atoi( argument );
		    break;
	        case 2:
		    send_to_char( "AC SLASH SET.\n\r\n\r", ch );
		    pObj->value[2] = atoi( argument );
		    break;
	        case 3:
		    send_to_char( "AC EXOTIC SET.\n\r\n\r", ch );
		    pObj->value[3] = atoi( argument );
		    break;
	    }
	    break;

/* WEAPONS changed in ROM */

        case ITEM_WEAPON:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_WEAPON" );
	            return FALSE;
	        case 0:
		    send_to_char( "WEAPON CLASS SET.\n\r\n\r", ch );
		    pObj->value[0] = flag_value( weapon_class, argument );
		    break;
	        case 1:
	            send_to_char( "NUMBER OF DICE SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "TYPE OF DICE SET.\n\r\n\r", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "WEAPON TYPE SET.\n\r\n\r", ch );
	            pObj->value[3] = attack_lookup( argument );
	            break;
	        case 4:
                    send_to_char( "SPECIAL WEAPON TYPE TOGGLED.\n\r\n\r", ch );
		    pObj->value[4] ^= (flag_value( weapon_type2, argument ) != NO_FLAG
		    ? flag_value( weapon_type2, argument ) : 0 );
		    break;
	    }
            break;

	case ITEM_PORTAL:
	    switch ( value_num )
	    {
	        default:
	            do_help(ch, "ITEM_PORTAL" );
	            return FALSE;
	            
	    	case 0:
	    	    send_to_char( "CHARGES SET.\n\r\n\r", ch);
	    	    pObj->value[0] = atoi ( argument );
	    	    break;
	    	case 1:
	    	    send_to_char( "EXIT FLAGS SET.\n\r\n\r", ch);
	    	    pObj->value[1] = ( (tmp = flag_value(exit_flags,argument)) == NO_FLAG ? 0 : tmp);
	    	    break;
	    	case 2:
	    	    send_to_char( "PORTAL FLAGS SET.\n\r\n\r", ch);
	    	    pObj->value[2] = ( (tmp = flag_value(portal_flags,argument)) == NO_FLAG ? 0 : tmp);
	    	    break;
	    	case 3:
	    	    send_to_char( "EXIT VNUM SET.\n\r\n\r", ch);
	    	    pObj->value[3] = atoi ( argument );
	    	    break;
	   }
	   break;

	case ITEM_FURNITURE:
	    switch ( value_num )
	    {
	        default:
	            do_help( ch, "ITEM_FURNITURE" );
	            return FALSE;
	            
	        case 0:
	            send_to_char( "NUMBER OF PEOPLE SET.\n\r\n\r", ch);
	            pObj->value[0] = atoi ( argument );
	            break;
	        case 1:
	            send_to_char( "MAX WEIGHT SET.\n\r\n\r", ch);
	            pObj->value[1] = atoi ( argument );
	            break;
	        case 2:
	            send_to_char( "FURNITURE FLAGS TOGGLED.\n\r\n\r", ch);
	            pObj->value[2] ^= (flag_value( furniture_flags, argument ) != NO_FLAG
	            ? flag_value( furniture_flags, argument ) : 0);
	            break;
	        case 3:
	            send_to_char( "HEAL BONUS SET.\n\r\n\r", ch);
	            pObj->value[3] = atoi ( argument );
	            break;
	        case 4:
	            send_to_char( "MANA BONUS SET.\n\r\n\r", ch);
	            pObj->value[4] = atoi ( argument );
	            break;
	    }
	    break;
	   
        case ITEM_CONTAINER:
	    switch ( value_num )
	    {
		int value;
		
		default:
		    do_help( ch, "ITEM_CONTAINER" );
	            return FALSE;
		case 0:
	            send_to_char( "WEIGHT CAPACITY SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
		case 1:
	            if ( ( value = flag_value( container_flags, argument ) )
	              != NO_FLAG )
	        	TOGGLE_BIT(pObj->value[1], value);
		    else
		    {
			do_help ( ch, "ITEM_CONTAINER" );
			return FALSE;
		    }
	            send_to_char( "CONTAINER TYPE SET.\n\r\n\r", ch );
	            break;
		case 2:
		    if ( atoi(argument) != 0 )
		    {
			if ( !get_obj_index( atoi( argument ) ) )
			{
			    send_to_char( "THERE IS NO SUCH ITEM.\n\r\n\r", ch );
			    return FALSE;
			}

			if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
			{
			    send_to_char( "THAT ITEM IS NOT A KEY.\n\r\n\r", ch );
			    return FALSE;
			}
		    }
		    send_to_char( "CONTAINER KEY SET.\n\r\n\r", ch );
		    pObj->value[2] = atoi( argument );
		    break;
		case 3:
		    send_to_char( "CONTAINER MAX WEIGHT SET.\n\r", ch);
		    pObj->value[3] = atoi( argument );
		    break;
		case 4:
		    send_to_char( "WEIGHT MULTIPLIER SET.\n\r\n\r", ch );
		    pObj->value[4] = atoi ( argument );
		    break;
	    }
	    break;

	case ITEM_DRINK_CON:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_DRINK" );
/* OLC		    do_help( ch, "liquids" );    */
	            return FALSE;
	        case 0:
	            send_to_char( "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "LIQUID TYPE SET.\n\r\n\r", ch );
	            pObj->value[2] = ( liq_lookup(argument) != -1 ?
	            		       liq_lookup(argument) : 0 );
	            break;
	        case 3:
	            send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
	            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	    }
            break;

	case ITEM_FOUNTAIN:
	    switch (value_num)
	    {
	    	default:
		    do_help( ch, "ITEM_FOUNTAIN" );
/* OLC		    do_help( ch, "liquids" );    */
	            return FALSE;
	        case 0:
	            send_to_char( "Cantidad maxima de liquido seteada.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "Cantidad actual de liquido seteada.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "Tipo de liquido seteado.\n\r\n\r", ch );
	            pObj->value[2] = ( liq_lookup( argument ) != -1 ?
	            		       liq_lookup( argument ) : 0 );
	            break;
            }
	break;
		    	
	case ITEM_FOOD:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_FOOD" );
	            return FALSE;
	        case 0:
	            send_to_char( "HOURS OF FOOD SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "HOURS OF FULL SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
	            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	    }
            break;

	case ITEM_MONEY:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_MONEY" );
	            return FALSE;
	        case 0:
	            send_to_char( "GOLD AMOUNT SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
		case 1:
		    send_to_char( "SILVER AMOUNT SET.\n\r\n\r", ch );
		    pObj->value[1] = atoi( argument );
		    break;
	    }
            break;
    }

    show_obj_values( ch, pObj );

    return TRUE;
}

OEDIT( oedit_show )
{
    OBJ_INDEX_DATA *pObj;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    int cnt;

    argument = one_argument( argument, buf );

    if ( IS_NULLSTR(buf) )
    {
	if ( ch->desc->editor == ED_OBJECT )
		EDIT_OBJ(ch, pObj);
	else
	{
		send_to_char( "Sintaxis : oshow [vnum]\n\r", ch );
		return FALSE;
	}
    }
    else
    {
    	pObj = get_obj_index(atoi(buf));

	if ( !pObj )
	{
		send_to_char( "ERROR : Objeto inexistente.\n\r", ch );
		return FALSE;
	}

	if ( !IS_BUILDER(ch, pObj->area) )
	{
		send_to_char( "ERROR : Objeto de un area que no puedes editar.\n\r", ch );
		return FALSE;
	}
    }

    sprintf( buf, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
	pObj->name,
	!pObj->area ? -1        : pObj->area->vnum,
	!pObj->area ? "No Area" : pObj->area->name );
    send_to_char( buf, ch );


    sprintf( buf, "Vnum:        [%5d]\n\rType:        [%s]\n\r",
	pObj->vnum,
	flag_string( type_flags, pObj->item_type ) );
    send_to_char( buf, ch );

    sprintf( buf, "Level:       [%5d]\n\r", pObj->level );
    send_to_char( buf, ch );

    sprintf( buf, "Wear flags:  [%s]\n\r",
	flag_string( wear_flags, pObj->wear_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Extra flags: [%s]\n\r",
	flag_string( extra_flags, pObj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Material:    [%s]\n\r",                /* ROM */
	pObj->material);
    send_to_char( buf, ch );

    sprintf( buf, "Condition:   [%5d]\n\r",               /* ROM */
	pObj->condition );
    send_to_char( buf, ch );
    
    sprintf( buf, "Weight:      [%5d]\n\rCost:        [%5d]\n\r",
	pObj->weight, pObj->cost );
    send_to_char( buf, ch );

    if ( pObj->extra_descr )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Ex desc kwd: ", ch );

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( "[", ch );
	    send_to_char( ed->keyword, ch );
	    send_to_char( "]", ch );
	}

	send_to_char( "\n\r", ch );
    }

    sprintf( buf, "Short desc:  %s\n\rLong desc:\n\r     %s\n\r",
	pObj->short_descr, pObj->description );
    send_to_char( buf, ch );

    for ( cnt = 0, paf = pObj->affected; paf; paf = paf->next )
    {
	if ( cnt == 0 )
	{
	    send_to_char( "Number Modifier Affects      Adds\n\r", ch );
	    send_to_char( "------ -------- ------------ ----\n\r", ch );
	}
	sprintf( buf, "[%4d] %-8d %-12s ", cnt,
	    paf->modifier,
	    flag_string( apply_flags, paf->location ) );
	send_to_char( buf, ch );
	sprintf( buf, "%s ", flag_string( bitvector_type[paf->where].table, paf->bitvector ) );
	send_to_char( buf, ch );
	sprintf( buf, "%s\n\r", flag_string( apply_types, paf->where ) );
	send_to_char( buf, ch );
	cnt++;
    }

    show_obj_values( ch, pObj );

    return FALSE;
}


/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OEDIT( oedit_addaffect )
{
    int value;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, loc );
    one_argument( argument, mod );

    if ( loc[0] == '\0' || mod[0] == '\0' || !is_number( mod ) )
    {
	send_to_char( "Syntax:  addaffect [location] [#xmod]\n\r", ch );
	return FALSE;
    }

    if ( ( value = flag_value( apply_flags, loc ) ) == NO_FLAG ) /* Hugin */
    {
        send_to_char( "Valid affects are:\n\r", ch );
	show_help( ch, "apply" );
	return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   value;
    pAf->modifier   =   atoi( mod );
    pAf->where	    =   TO_OBJECT;
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Affect added.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_addapply )
{
    int value,bv,typ;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];
    char type[MAX_STRING_LENGTH];
    char bvector[MAX_STRING_LENGTH];

    EDIT_OBJ(ch, pObj);

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  addapply [type] [location] [#xmod] [bitvector]\n\r", ch );
	return FALSE;
    }

    argument = one_argument( argument, type );
    argument = one_argument( argument, loc );
    argument = one_argument( argument, mod );
    one_argument( argument, bvector );

    if ( type[0] == '\0' || ( typ = flag_value( apply_types, type ) ) == NO_FLAG )
    {
    	send_to_char( "Invalid apply type. Valid apply types are:\n\r", ch);
    	show_help( ch, "apptype" );
    	return FALSE;
    }

    if ( loc[0] == '\0' || ( value = flag_value( apply_flags, loc ) ) == NO_FLAG )
    {
        send_to_char( "Valid applys are:\n\r", ch );
	show_help( ch, "apply" );
	return FALSE;
    }

    if ( bvector[0] == '\0' || ( bv = flag_value( bitvector_type[typ].table, bvector ) ) == NO_FLAG )
    {
    	send_to_char( "Invalid bitvector type.\n\r", ch );
	send_to_char( "Valid bitvector types are:\n\r", ch );
	show_help( ch, bitvector_type[typ].help );
    	return FALSE;
    }

    if ( mod[0] == '\0' || !is_number( mod ) )
    {
	send_to_char( "Syntax:  addapply [type] [location] [#xmod] [bitvector]\n\r", ch );
	return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   value;
    pAf->modifier   =   atoi( mod );
    pAf->where	    =   apply_flags[typ].bit;
    pAf->type	    =	-1;
    pAf->duration   =   -1;
    pAf->bitvector  =   bv;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Apply added.\n\r", ch);
    return TRUE;
}

/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OEDIT( oedit_delaffect )
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    AFFECT_DATA *pAf_next;
    char affect[MAX_STRING_LENGTH];
    int  value;
    int  cnt = 0;

    EDIT_OBJ(ch, pObj);

    one_argument( argument, affect );

    if ( !is_number( affect ) || affect[0] == '\0' )
    {
	send_to_char( "Syntax:  delaffect [#xaffect]\n\r", ch );
	return FALSE;
    }

    value = atoi( affect );

    if ( value < 0 )
    {
	send_to_char( "Only non-negative affect-numbers allowed.\n\r", ch );
	return FALSE;
    }

    if ( !( pAf = pObj->affected ) )
    {
	send_to_char( "OEdit:  Non-existant affect.\n\r", ch );
	return FALSE;
    }

    if( value == 0 )	/* First case: Remove first affect */
    {
	pAf = pObj->affected;
	pObj->affected = pAf->next;
	free_affect( pAf );
    }
    else		/* Affect to remove is not the first */
    {
	while ( ( pAf_next = pAf->next ) && ( ++cnt < value ) )
	     pAf = pAf_next;

	if( pAf_next )		/* See if it's the next affect */
	{
	    pAf->next = pAf_next->next;
	    free_affect( pAf_next );
	}
	else                                 /* Doesn't exist */
	{
	     send_to_char( "No such affect.\n\r", ch );
	     return FALSE;
	}
    }

    send_to_char( "Affect removed.\n\r", ch);
    return TRUE;
}

bool set_value( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, char *argument, int value )
{
    if ( argument[0] == '\0' )
    {
	set_obj_values( ch, pObj, -1, "" );     /* '\0' changed to "" -- Hugin */
	return FALSE;
    }

    if ( set_obj_values( ch, pObj, value, argument ) )
	return TRUE;

    return FALSE;
}



/*****************************************************************************
 Name:		oedit_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values( CHAR_DATA *ch, char *argument, int value )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( set_value( ch, pObj, argument, value ) )
        return TRUE;

    return FALSE;
}

OEDIT( oedit_create )
{
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    int  value;
    int  iHash;

    value = atoi( argument );
    if ( argument[0] == '\0' || value == 0 )
    {
	send_to_char( "Syntax:  oedit create [vnum]\n\r", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );
    if ( !pArea )
    {
	send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "OEdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    if ( get_obj_index( value ) )
    {
	send_to_char( "OEdit:  Object vnum already exists.\n\r", ch );
	return FALSE;
    }
        
    pObj			= new_obj_index();
    pObj->vnum			= value;
    pObj->area			= pArea;
    pObj->extra_flags		= 0;
        
    if ( value > top_vnum_obj )
	top_vnum_obj = value;

    iHash			= value % MAX_KEY_HASH;
    pObj->next			= obj_index_hash[iHash];
    obj_index_hash[iHash]	= pObj;

    set_editor(ch->desc, ED_OBJECT, pObj);
/*    ch->desc->pEdit		= (void *)pObj; */

    send_to_char( "Object Created.\n\r", ch );
    return TRUE;
}

/*
 * Mobile Editor Functions.
 */
MEDIT( medit_show )
{
    MOB_INDEX_DATA *pMob;
    char buf[MAX_STRING_LENGTH];
    MPROG_LIST *list;
    int cnt;
    BUFFER *buffer;

    argument = one_argument( argument, buf );

    if ( IS_NULLSTR(buf) )
    {
    	if ( ch->desc->editor == ED_MOBILE )
		EDIT_MOB(ch, pMob);
	else
	{
		send_to_char( "ERROR : debes especificar un vnum como argumento.\n\r", ch );
		return FALSE;
	}
    }
    else
    {
    	pMob = get_mob_index(atoi(buf));

	if ( !pMob )
	{
		send_to_char( "ERROR : Mob inexistente.\n\r", ch );
		return FALSE;
	}

	if ( !IS_BUILDER(ch, pMob->area) )
	{
		send_to_char( "ERROR : Mob de un area que no puedes editar.\n\r", ch );
		return FALSE;
	}
    }

    buffer = new_buf();

    sprintf( buf, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
	pMob->player_name,
	!pMob->area ? -1        : pMob->area->vnum,
	!pMob->area ? "No Area" : pMob->area->name );
    add_buf( buffer, buf );

    sprintf( buf, "Act:         [%s]\n\r",
	flag_string( act_flags, pMob->act ) );
    add_buf( buffer, buf );

    sprintf( buf, "Vnum:        [%5d] Sex:   [%6s]    Group: [%5d]\n\r"
                  "Level:       [%2d]    Align: [%4d]   Dam type: [%s]\n\r",
	pMob->vnum,
	( (pMob->sex > -1 && pMob->sex < 4) ? sex_table[pMob->sex].name : "ERROR" ),
	pMob->group,
	pMob->level,
	pMob->alignment,
	attack_table[pMob->dam_type].name);
    add_buf( buffer, buf );

/* ROM values: */

    sprintf( buf, "Hit dice:    [%2dd%-3d+%4d] "
		  "Damage dice: [%2dd%-3d+%4d]\n\r"
		  "Mana dice:   [%2dd%-3d+%4d] "
		  "Hitroll:     [%d]\n\r",
	     pMob->hit[DICE_NUMBER],
	     pMob->hit[DICE_TYPE],
	     pMob->hit[DICE_BONUS],
	     pMob->damage[DICE_NUMBER],
	     pMob->damage[DICE_TYPE],
	     pMob->damage[DICE_BONUS],
	     pMob->mana[DICE_NUMBER],
	     pMob->mana[DICE_TYPE],
	     pMob->mana[DICE_BONUS],
	     pMob->hitroll);
    add_buf( buffer, buf );

/* ROM values end */

    sprintf( buf, "Race:        [%16s] "		/* ROM OLC */
		  "Size:         [%16s]\n\r",
	race_table[pMob->race].name,
	( (pMob->size > -1 && pMob->size < 6) ?
	size_table[pMob->size].name : "ERROR") );
    add_buf( buffer, buf );

    sprintf( buf, "Material:    [%16s] "
    		  "Wealth:       [%5ld]\n\r",
        pMob->material,
        pMob->wealth );
    add_buf( buffer, buf );

    sprintf( buf, "Start pos.:  [%16s] "
		  "Default pos.: [%16s]\n\r",
	position_table[pMob->start_pos].name,
	position_table[pMob->default_pos].name );
    add_buf( buffer, buf );

    sprintf( buf, "Affected by: [%s]\n\r",
	flag_string( affect_flags, pMob->affected_by ) );
    add_buf( buffer, buf );

/* ROM values: */

    sprintf( buf, "Armor:       [pierce: %d  bash: %d  slash: %d  magic: %d]\n\r",
	pMob->ac[AC_PIERCE], pMob->ac[AC_BASH],
	pMob->ac[AC_SLASH], pMob->ac[AC_EXOTIC] );
    add_buf( buffer, buf );

    sprintf( buf, "Form:        [%s]\n\r",
	flag_string( form_flags, pMob->form ) );
    add_buf( buffer, buf );

    sprintf( buf, "Parts:       [%s]\n\r",
	flag_string( part_flags, pMob->parts ) );
    add_buf( buffer, buf );

    sprintf( buf, "Imm:         [%s]\n\r",
	flag_string( imm_flags, pMob->imm_flags ) );
    add_buf( buffer, buf );

    sprintf( buf, "Res:         [%s]\n\r",
	flag_string( res_flags, pMob->res_flags ) );
    add_buf( buffer, buf );

    sprintf( buf, "Vuln:        [%s]\n\r",
	flag_string( vuln_flags, pMob->vuln_flags ) );
    add_buf( buffer, buf );

    sprintf( buf, "Off:         [%s]\n\r",
	flag_string( off_flags,  pMob->off_flags ) );
    add_buf( buffer, buf );

/* ROM values end */

    if ( pMob->spec_fun )
    {
	sprintf( buf, "Spec fun:    [%s]\n\r",  spec_name( pMob->spec_fun ) );
	add_buf( buffer, buf );
    }

    sprintf( buf, "Short descr: %s\n\rLong descr:\n\r%s",
	pMob->short_descr,
	pMob->long_descr );
    add_buf( buffer, buf );

    sprintf( buf, "Description:\n\r%s", pMob->description );
    add_buf( buffer, buf );

    if ( pMob->pShop )
    {
	SHOP_DATA *pShop;
	int iTrade;

	pShop = pMob->pShop;

	sprintf( buf,
	  "Shop data for [%5d]:\n\r"
	  "  Markup for purchaser: %d%%\n\r"
	  "  Markdown for seller:  %d%%\n\r",
	    pShop->keeper, pShop->profit_buy, pShop->profit_sell );
	add_buf( buffer, buf );
	sprintf( buf, "  Hours: %d to %d.\n\r",
	    pShop->open_hour, pShop->close_hour );
	add_buf( buffer, buf );

	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	{
	    if ( pShop->buy_type[iTrade] != 0 )
	    {
		if ( iTrade == 0 ) {
		    add_buf( buffer, "  Number Trades Type\n\r" );
		    add_buf( buffer, "  ------ -----------\n\r" );
		}
		sprintf( buf, "  [%4d] %s\n\r", iTrade,
		    flag_string( type_flags, pShop->buy_type[iTrade] ) );
		add_buf( buffer, buf );
	    }
	}
    }

    if ( pMob->mprogs )
    {
    sprintf(buf,
               "\n\rMOBPrograms for [%5d]:\n\r", pMob->vnum);
    add_buf( buffer, buf );

        for (cnt=0, list=pMob->mprogs; list; list=list->next)
        {
              if (cnt ==0)
              {
                      add_buf ( buffer, " Number Vnum Trigger Phrase\n\r" );
                      add_buf ( buffer, " ------ ---- ------- ------\n\r" );
              }

              sprintf(buf, "[%5d] %4d %7s %s\n\r", cnt,
                     list->vnum,mprog_type_to_name(list->trig_type),
                     list->trig_phrase);
              add_buf( buffer, buf );
              cnt++;
        }
    }

    page_to_char( buf_string(buffer), ch );

    free_buf(buffer);

    return FALSE;
}

MEDIT( medit_group )
{
    MOB_INDEX_DATA *pMob;
    MOB_INDEX_DATA *pMTemp;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int temp;
    BUFFER *buffer;
    bool found = FALSE;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
    	send_to_char( "Sintaxis: group [numero]\n\r", ch);
    	send_to_char( "          group show [numero]\n\r", ch);
    	return FALSE;
    }

    if (is_number(argument))
    {
	pMob->group = atoi(argument);
    	send_to_char( "Group set.\n\r", ch );
	return TRUE;
    }

    argument = one_argument( argument, arg );

    if ( !strcmp( arg, "show" ) && is_number( argument ) )
    {
	if (atoi(argument) == 0)
	{
		send_to_char( "Estas loco?\n\r", ch);
		return FALSE;
	}

	buffer = new_buf ();

    	for (temp = 0; temp < MAX_VNUM; temp++)
    	{
    		pMTemp = get_mob_index(temp);
    		if ( pMTemp && ( pMTemp->group == atoi(argument) ) )
    		{
			found = TRUE;
    			sprintf( buf, "[%5d] %s\n\r", pMTemp->vnum, pMTemp->player_name );
			add_buf( buffer, buf );
    		}
    	}

	if (found)
		page_to_char( buf_string(buffer), ch );
	else
		send_to_char( "No existen mobs en ese grupo.\n\r", ch );

	free_buf( buffer );
    }

    return FALSE;
}

void show_liqlist(CHAR_DATA *ch)
{
    int liq;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    
    buffer = new_buf();
    
    for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
	if ( (liq % 21) == 0 )
	    add_buf(buffer,"Name                 Color          Proof Full Thirst Food Ssize\n\r");

	sprintf(buf, "%-20s %-14s %5d %4d %6d %4d %5d\n\r",
		liq_table[liq].liq_name,liq_table[liq].liq_color,
		liq_table[liq].liq_affect[0],liq_table[liq].liq_affect[1],
		liq_table[liq].liq_affect[2],liq_table[liq].liq_affect[3],
		liq_table[liq].liq_affect[4] );
	add_buf(buffer,buf);
    }

    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);

return;
}

void show_damlist(CHAR_DATA *ch)
{
    int att;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    
    buffer = new_buf();
    
    for ( att = 0; attack_table[att].name != NULL; att++)
    {
	if ( (att % 21) == 0 )
	    add_buf(buffer,"Name                 Noun\n\r");

	sprintf(buf, "%-20s %-20s\n\r",
		attack_table[att].name,attack_table[att].noun );
	add_buf(buffer,buf);
    }

    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);

return;
}

void show_poslist (CHAR_DATA *ch)
{
    int pos;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    
    buffer = new_buf();
    
    for ( pos = 0; position_table[pos].name != NULL; pos++)
    {
	if ( (pos % 21) == 0 )
	    add_buf(buffer,"Name                 Short name\n\r");

	sprintf(buf, "%-20s %-20s\n\r",
		position_table[pos].name,position_table[pos].short_name );
	add_buf(buffer,buf);
    }

    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);

return;
}

void show_sexlist (CHAR_DATA *ch)
{
    int sex;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    
    buffer = new_buf();
    
    for ( sex = 0; sex_table[sex].name != NULL; sex++)
    {
	if ( (sex % 3) == 0 )
	    add_buf(buffer,"\n\r");

	sprintf(buf, "%-20s ",
		sex_table[sex].name);
	add_buf(buffer,buf);
    }

    add_buf(buffer, "\n\r");
    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);

return;
}

void show_sizelist (CHAR_DATA *ch)
{
    int size;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    
    buffer = new_buf();
    
    for ( size = 0; size_table[size].name != NULL; size++)
    {
	if ( (size % 3) == 0 )
	    add_buf(buffer,"\n\r");

	sprintf(buf, "%-20s ",
		size_table[size].name);
	add_buf(buffer,buf);
    }

    add_buf(buffer, "\n\r");
    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);

return;
}

REDIT( redit_owner )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  owner [owner]\n\r", ch );
	send_to_char( "         owner none\n\r", ch );
	return FALSE;
    }

    free_string( pRoom->owner );
    if (!str_cmp(argument, "none"))
    	pRoom->owner = str_dup("");
    else
	pRoom->owner = str_dup( argument );

    send_to_char( "Owner set.\n\r", ch );
    return TRUE;
}

void showresets(CHAR_DATA *ch, BUFFER *buf, AREA_DATA *pArea, MOB_INDEX_DATA *mob, OBJ_INDEX_DATA *obj)
{
	ROOM_INDEX_DATA *room;
	MOB_INDEX_DATA *pLastMob;
	RESET_DATA *reset;
	char buf2[MIL];
	int key, lastmob;

	for ( key = 0; key < MAX_KEY_HASH; ++key )
		for ( room = room_index_hash[key]; room; room = room->next )
			if ( room->area == pArea )
			{
				lastmob = -1;
				pLastMob = NULL;

				for ( reset = room->reset_first; reset; reset = reset->next )
				{
					if ( reset->command == 'M' )
					{
						lastmob = reset->arg1;
						pLastMob = get_mob_index(lastmob);
						if ( pLastMob == NULL )
						{
							bugf( "Showresets : reset invalido (mob %d) en cuarto %d", lastmob, room->vnum );
							return;
						}
						if ( mob && lastmob == mob->vnum )
						{
							sprintf( buf2, "%-5d %-15.15s %-5d\n\r", lastmob, mob->player_name, room->vnum);
							add_buf( buf, buf2 );
						}
					}
					if ( obj && reset->command == 'O' && reset->arg1 == obj->vnum )
					{
						sprintf( buf2, "%-5d %-15.15s %-5d\n\r", obj->vnum, obj->name, room->vnum );
						add_buf( buf, buf2 );
					}
					if ( obj && (reset->command == 'G' || reset->command == 'E') && reset->arg1 == obj->vnum )
					{
						sprintf( buf2, "%-5d %-15.15s %-5d %-5d %-15.15s\n\r", obj->vnum, obj->name, room->vnum, lastmob, pLastMob ? pLastMob->player_name : "" );
						add_buf( buf, buf2 );
					}
				}
			}
}

void listobjreset(CHAR_DATA *ch, BUFFER *buf, AREA_DATA *pArea)
{
	OBJ_INDEX_DATA *obj;
	int key;

	add_buf(buf, "#UVnum  Nombre          Room  En mob#u\n\r");

	for ( key = 0; key < MAX_KEY_HASH; ++key )
		for ( obj = obj_index_hash[key]; obj; obj = obj->next )
			if ( obj->area == pArea )
				showresets(ch, buf, pArea, 0, obj);
}

void listmobreset(CHAR_DATA *ch, BUFFER *buf, AREA_DATA *pArea)
{
	MOB_INDEX_DATA *mob;
	int key;

	add_buf(buf, "#UVnum  Nombre          Room #u\n\r");

	for ( key = 0; key < MAX_KEY_HASH; ++key )
		for ( mob = mob_index_hash[key]; mob; mob = mob->next )
			if ( mob->area == pArea )
				showresets(ch, buf, pArea, mob, 0);
}

REDIT( redit_listreset )
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoom;
	BUFFER *buf;

	EDIT_ROOM(ch, pRoom);

	pArea = pRoom->area;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : listreset [obj/mob]\n\r", ch );
		return FALSE;
	}

	buf = new_buf();

	if ( !str_cmp(argument, "obj") )
		listobjreset(ch,buf,pArea);
	else if ( !str_cmp(argument, "mob") )
		listmobreset(ch,buf,pArea);
	else
	{
		send_to_char( "AEdit : Argumento invalido.\n\r", ch );
		free_buf(buf);
		return FALSE;
	}

	page_to_char(buf_string(buf), ch);

	return FALSE;
}

REDIT( redit_checkobj )
{
	OBJ_INDEX_DATA *obj;
	int key;
	bool fAll = !str_cmp(argument, "all");
	ROOM_INDEX_DATA *room;

	EDIT_ROOM(ch, room);

	for ( key = 0; key < MAX_KEY_HASH; ++key )
		for ( obj = obj_index_hash[key]; obj; obj = obj->next )
			if ( obj->reset_num == 0 && (fAll || obj->area == room->area) )
				printf_to_char( ch, "Obj #B%-5.5d#b [%-20.20s] no esta reseteado.\n\r", obj->vnum, obj->name );

	return FALSE;
}

REDIT( redit_checkrooms )
{
	ROOM_INDEX_DATA *room, *thisroom;
	int iHash;
	bool fAll = FALSE;

	if ( !str_cmp( argument, "all" ) )
		fAll = TRUE;
	else
	if ( !IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : checkrooms\n\r"
			      "           checkrooms all\n\r", ch );
		return FALSE;
	}

	EDIT_ROOM(ch, thisroom);

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
		for ( room = room_index_hash[iHash]; room; room = room->next )
			if ( room->reset_num == 0 && (fAll || room->area == thisroom->area) )
				printf_to_char( ch, "Cuarto %d no esta linkeado.\n\r", room->vnum );

	return FALSE;
}

REDIT( redit_checkmob )
{
	MOB_INDEX_DATA *mob;
	ROOM_INDEX_DATA *room;
	int key;
	bool fAll = !str_cmp(argument, "all");

	EDIT_ROOM(ch, room);

	for ( key = 0; key < MAX_KEY_HASH; ++key )
		for ( mob = mob_index_hash[key]; mob; mob = mob->next )
			if ( mob->reset_num == 0 && (fAll || mob->area == room->area) )
				printf_to_char( ch, "Mob #B%-5.5d#b [%-20.20s] no esta reseteado.\n\r", mob->vnum, mob->player_name );

	return FALSE;
}

REDIT( redit_copy )
{
	ROOM_INDEX_DATA *this, *that;
	int vnum;

	EDIT_ROOM(ch, this);

	if ( IS_NULLSTR(argument) || !is_number(argument) )
	{
		send_to_char( "Sintaxis : copy [vnum]\n\r", ch );
		return FALSE;
	}

	vnum = atoi(argument);

	if ( vnum < 1 || vnum >= MAX_VNUM)
	{
		send_to_char( "REdit : argumento invalido.\n\r", ch );
		return FALSE;
	}

	that = get_room_index(vnum);

	if ( !that || !IS_BUILDER(ch, that->area) || that == this )
	{
		send_to_char( "REdit : cuarto inexistente o incopiable.\n\r", ch );
		return FALSE;
	}

	free_string(this->name);
	free_string(this->description);
	free_string(this->owner);

	this->name		= str_dup(that->name);
	this->description	= str_dup(that->description);
	this->owner		= str_dup(that->owner);

	this->room_flags	= that->room_flags;
	this->sector_type	= that->sector_type;
	this->clan		= that->clan;
	this->heal_rate		= that->heal_rate;
	this->mana_rate		= that->mana_rate;

	send_to_char( "Ok. Cuarto copiado.\n\r", ch );
	return TRUE;
}

MEDIT(medit_copy)
{
	int vnum;
	MOB_INDEX_DATA *pMob, *mob2;

	EDIT_MOB(ch, pMob);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : copiar [vnum]\n\r", ch );
		return FALSE;
	}

	if ( !is_number(argument) || (vnum = atoi(argument)) < 0 )
	{
		send_to_char( "ERROR : Argumento invalido.\n\r", ch );
		return FALSE;
	}

	if ( (mob2 = get_mob_index(vnum)) == NULL )
	{
		send_to_char( "ERROR : Mob inexistente.\n\r", ch );
		return FALSE;
	}

	if ( !IS_BUILDER(ch, mob2->area) )
	{
		send_to_char( "ERROR : Mob de un area que no puedes editar.\n\r", ch );
		return FALSE;
	}

	free_string(pMob->player_name);
	pMob->player_name = str_dup(mob2->player_name);
	free_string(pMob->short_descr);
	pMob->short_descr = str_dup(mob2->short_descr);
	free_string(pMob->long_descr);
	pMob->long_descr = str_dup(mob2->long_descr);
	free_string(pMob->description);
	pMob->description = str_dup(mob2->description);

	pMob->group		= mob2->group;
	pMob->act		= mob2->act;
	pMob->affected_by	= mob2->affected_by;
	pMob->alignment		= mob2->alignment;
	pMob->level		= mob2->level;
	pMob->hitroll		= mob2->hitroll;
	ARRAY_COPY(pMob->hit, mob2->hit, 3);
	ARRAY_COPY(pMob->mana, mob2->mana, 3);
	ARRAY_COPY(pMob->damage, mob2->damage, 3);
	ARRAY_COPY(pMob->ac, mob2->ac, 4);
	pMob->dam_type		= mob2->dam_type;
	pMob->size		= mob2->size;
	pMob->off_flags		= mob2->off_flags;
	pMob->imm_flags		= mob2->imm_flags;
	pMob->res_flags		= mob2->res_flags;
	pMob->vuln_flags	= mob2->vuln_flags;
	pMob->start_pos		= mob2->start_pos;
	pMob->default_pos	= mob2->default_pos;
	pMob->sex		= mob2->sex;
	pMob->race		= mob2->race;
	pMob->wealth		= mob2->wealth;
	pMob->form		= mob2->form;
	pMob->parts		= mob2->parts;
	free_string(pMob->material);
	pMob->material		= str_dup(mob2->material);

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

ED_FUN_DEC(ed_line_string)
{
	char ** string = (char **) arg;
	char buf[MIL];

	if (IS_NULLSTR(argument))
	{
		sprintf( buf, "Sintaxis : %s <cadena>\n\r", n_fun );
		send_to_char( buf, ch );
		return FALSE;
	}

	if ( !str_cmp(argument, "null") )
	{
		free_string(*string);
		*string = str_dup( "" );
	}
	else
	{
		sprintf( buf, "%s%s", argument, par == NULL ? "" : "\n\r" );

		free_string(*string);
		*string = str_dup( buf );
	}

	send_to_char( "Ok.\n\r", ch );

	return TRUE;
}

#define CORTO 0
#define NORMAL 1
#define LARGO 2

bool numedit( char * n_fun, CHAR_DATA *ch, char *argument, void * arg, sh_int tipo, long min, long max )
{
	int temp;
	int * valor = (int *) arg;
	sh_int * shvalor = (sh_int *) arg;
	long * lvalor = (long *) arg;

	if ( IS_NULLSTR(argument) )
	{
		printf_to_char( ch, "Sintaxis : %s [numero]\n\r", n_fun );
		return FALSE;
	}

	if ( !is_number(argument) )
	{
		send_to_char( "ERROR : Argumento debe ser un numero.\n\r", ch );
		return FALSE;
	}

	temp = atoi(argument);

	if ( min != -1 && temp < min )
	{
		printf_to_char( ch, "ERROR : Numero debe ser mayor que %d.\n\r", min );
		return FALSE;
	}

	if ( max != -1 && temp > max )
	{
		printf_to_char( ch, "ERROR : Numero debe ser menor que %d.\n\r", max );
		return FALSE;
	}

	if (tipo == CORTO)
		*shvalor = temp;
	else if (tipo == NORMAL)
		*valor = temp;
	else
		*lvalor = temp;

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

ED_FUN_DEC(ed_numero_align)
{
	return numedit( n_fun, ch, argument, arg, CORTO, -1000, 1000 );
}

ED_FUN_DEC(ed_numero_pos)
{
	return numedit( n_fun, ch, argument, arg, NORMAL, 0, -1 );
}

ED_FUN_DEC(ed_numero_s_pos)
{
	return numedit( n_fun, ch, argument, arg, CORTO, 0, -1 );
}

ED_FUN_DEC(ed_numero_l_pos)
{
	return numedit( n_fun, ch, argument, arg, LARGO, 0, -1 );
}

ED_FUN_DEC(ed_numero_niv)
{
	return numedit( n_fun, ch, argument, arg, CORTO, 0, MAX_LEVEL );
}

ED_FUN_DEC(ed_desc)
{
    if ( emptystring(argument) )
    {
	if (IS_SET(ch->comm, COMM_OLCX))
		do_clear(ch,"reset");

	string_append( ch, (char **) arg );
	return TRUE;
    }

    send_to_char( "Sintaxis : desc\n\r", ch );

    return FALSE;
}

ED_FUN_DEC(ed_bool)
{
	if ( emptystring(argument) )
	{
		printf_to_char( ch, "Sintaxis : %s [true/false]\n\r", n_fun );
		return FALSE;
	}

	if ( !str_cmp( argument, "true" ) )
		*(bool *) arg = TRUE;
	else
	if ( !str_cmp( argument, "false" ) )
		*(bool *) arg = FALSE;
	else
	{
		send_to_char( "ERROR : argumento invalido.\n\r", ch );
		return FALSE;
	}

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

ED_FUN_DEC(ed_flag_toggle)
{
    int value;

    if ( !emptystring(argument) )
    {
	if ( ( value = flag_value( (struct flag_type *) par, argument ) ) != NO_FLAG )
	{
	    *(long *) arg ^= value;

	    printf_to_char( ch, "%c%s flag toggled.\n\r",
	    	toupper(n_fun[0]),
	    	&n_fun[1] );
	    return TRUE;
	}
    }

    printf_to_char( ch, "Sintaxis : %s [flags]\n\r", n_fun );

    return FALSE;
}

ED_FUN_DEC(ed_flag_set_long)
{
    int value;

    if ( !emptystring(argument) )
    {
	if ( ( value = flag_value( (struct flag_type *) par, argument ) ) != NO_FLAG )
	{
	    *(long *) arg = value;

	    printf_to_char( ch, "%c%s flag seteada.\n\r",
	    	toupper(n_fun[0]),
	    	&n_fun[1] );
	    return TRUE;
	}
    }

    printf_to_char( ch, "Sintaxis : %s [flags]\n\r", n_fun );

    return FALSE;
}

ED_FUN_DEC(ed_flag_set_sh)
{
    int value;

    if ( !emptystring(argument) )
    {
	if ( ( value = flag_value( (struct flag_type *) par, argument ) ) != NO_FLAG )
	{
	    *(sh_int *) arg = value;

	    printf_to_char( ch, "%c%s flag seteada.\n\r",
	    	toupper(n_fun[0]),
	    	&n_fun[1] );
	    return TRUE;
	}
    }

    printf_to_char( ch, "Sintaxis : %s [flags]\n\r", n_fun );

    return FALSE;
}

ED_FUN_DEC( ed_shop )
{
    MOB_INDEX_DATA * pMob = (MOB_INDEX_DATA *) arg;
    char command[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );
    argument = one_argument( argument, arg1 );

    if ( command[0] == '\0' )
    {
	send_to_char( "Sintaxis : shop hours [abrir] [cerrar]\n\r", ch );
	send_to_char( "           shop profit [%% compra] [%% venta]\n\r", ch );
	send_to_char( "           shop type [0-4] [tipo obj]\n\r", ch );
	send_to_char( "           shop type [0-4] none\n\r", ch );
	send_to_char( "           shop assign\n\r", ch );
	send_to_char( "           shop remove\n\r", ch );

	return FALSE;
    }

    if ( !str_cmp( command, "hours" ) )
    {
	if ( arg1[0] == '\0' || !is_number( arg1 )
	|| argument[0] == '\0' || !is_number( argument ) )
	{
	    send_to_char( "Sintaxis : shop hours [abrir] [cerrar]\n\r", ch );
	    return FALSE;
	}

	if ( !pMob->pShop )
	{
	    send_to_char( "ERROR : Debes crear un shop primero (shop assign).\n\r", ch );
	    return FALSE;
	}

	pMob->pShop->open_hour = atoi( arg1 );
	pMob->pShop->close_hour = atoi( argument );

	send_to_char( "Horas seteadas.\n\r", ch);
	return TRUE;
    }

    if ( !str_cmp( command, "profit" ) )
    {
	if ( arg1[0] == '\0' || !is_number( arg1 )
	|| argument[0] == '\0' || !is_number( argument ) )
	{
	    send_to_char( "Sintaxis : shop profit [%% compra] [%% venta]\n\r", ch );
	    return FALSE;
	}

	if ( !pMob->pShop )
	{
	    send_to_char( "ERROR : Debes crear un shop primero (shop assign).\n\r", ch );
	    return FALSE;
	}

	pMob->pShop->profit_buy     = atoi( arg1 );
	pMob->pShop->profit_sell    = atoi( argument );

	send_to_char( "Shop profit seteado.\n\r", ch);
	return TRUE;
    }

    if ( !str_cmp( command, "type" ) )
    {
	char buf[MAX_INPUT_LENGTH];
	int value = 0;

	if ( arg1[0] == '\0' || !is_number( arg1 )
	|| argument[0] == '\0' )
	{
	    send_to_char( "Syntax:  shop type [#x0-4] [item type]\n\r", ch );
	    return FALSE;
	}

	if ( atoi( arg1 ) >= MAX_TRADE )
	{
	    sprintf( buf, "MEdit:  May sell %d items max.\n\r", MAX_TRADE );
	    send_to_char( buf, ch );
	    return FALSE;
	}

	if ( !pMob->pShop )
	{
	    send_to_char( "MEdit:  Debes crear un shop primero (shop assign).\n\r", ch );
	    return FALSE;
	}

	if ( str_cmp(argument, "none") && ( value = flag_value( type_flags, argument ) ) == NO_FLAG )
	{
	    send_to_char( "MEdit:  That type of item is not known.\n\r", ch );
	    return FALSE;
	}

	pMob->pShop->buy_type[atoi( arg1 )] = !str_cmp(argument, "none") ? 0 : value;

	send_to_char( "Shop type set.\n\r", ch);
	return TRUE;
    }

    /* shop assign && shop delete by Phoenix */
    if ( !str_prefix(command, "assign") )
    {
    	if ( pMob->pShop )
    	{
        	send_to_char("Mob already has a shop assigned to it.\n\r", ch);
        	return FALSE;
	}

	pMob->pShop		= new_shop();
	if ( !shop_first )
        	shop_first	= pMob->pShop;
	if ( shop_last )
		shop_last->next	= pMob->pShop;
	shop_last		= pMob->pShop;

	pMob->pShop->keeper	= pMob->vnum;

	send_to_char("New shop assigned to mobile.\n\r", ch);
	return TRUE;
    }

    if ( !str_prefix(command, "remove") )
    {
	SHOP_DATA *pShop;

	pShop		= pMob->pShop;
	pMob->pShop	= NULL;

	if ( pShop == shop_first )
	{
		if ( !pShop->next )
		{
			shop_first = NULL;
			shop_last = NULL;
		}
		else
			shop_first = pShop->next;
	}
	else
	{
		SHOP_DATA *ipShop;

		for ( ipShop = shop_first; ipShop; ipShop = ipShop->next )
		{
			if ( ipShop->next == pShop )
			{
				if ( !pShop->next )
				{
					shop_last = ipShop;
					shop_last->next = NULL;
				}
				else
					ipShop->next = pShop->next;
			}
		}
	}

	free_shop(pShop);

	send_to_char("Mobile is no longer a shopkeeper.\n\r", ch);
	return TRUE;
    }

    ed_shop( n_fun, ch, "", arg, par );

    return FALSE;
}

ED_FUN_DEC( ed_new_mob )
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int  value;
    int  iHash;

    value = atoi( argument );

    if ( argument[0] == '\0' || value == 0 )
    {
	send_to_char( "Sintaxis : medit create [vnum]\n\r", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );

    if ( !pArea )
    {
	send_to_char( "MEdit : ese vnum no esta asignado a ningun area.\n\r", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "MEdit : ese vnum es de un area que no puedes editar.\n\r", ch );
	return FALSE;
    }

    if ( get_mob_index( value ) )
    {
	send_to_char( "MEdit : un mob con ese vnum ya existe.\n\r", ch );
	return FALSE;
    }

    pMob			= new_mob_index();
    pMob->vnum			= value;
    pMob->area			= pArea;
    pMob->act			= ACT_IS_NPC;
        
    if ( value > top_vnum_mob )
	top_vnum_mob = value;        

    SET_BIT(pArea->area_flags, AREA_CHANGED);

    iHash			= value % MAX_KEY_HASH;
    pMob->next			= mob_index_hash[iHash];
    mob_index_hash[iHash]	= pMob;

    set_editor(ch->desc, ED_MOBILE, pMob);
/*    ch->desc->pEdit		= (void *)pMob; */

    send_to_char( "Mob creado.\n\r", ch );

    return TRUE;
}

ED_FUN_DEC( ed_commands )
{
	show_commands( ch, "" );
	return FALSE;
}

ED_FUN_DEC( ed_gamespec )
{
    SPEC_FUN ** spec = (SPEC_FUN **) arg;

    if ( argument[0] == '\0' )
    {
	printf_to_char( ch, "Sintaxis : %s [%s]\n\r",
		n_fun,
		n_fun );
	return FALSE;
    }

    if ( !str_cmp( argument, "none" ) )
    {
	*spec = NULL;
	printf_to_char( ch, "%s eliminado.\n\r", n_fun );
        return TRUE;
    }

   	if ( spec_lookup(argument) )
    	{
		* spec = spec_lookup( argument );
		send_to_char( "Spec set.\n\r", ch);
		return TRUE;
	}
	else
	{
		send_to_char( "ERROR : Spec inexistente.\n\r", ch );
		return FALSE;
	}

    return FALSE;
}

ED_FUN_DEC( ed_objrecval )
{
	OBJ_INDEX_DATA *pObj = (OBJ_INDEX_DATA *) arg;

	switch(pObj->item_type)
	{
		default:
		send_to_char( "No puedes recalcular este objeto.\n\r", ch );
		return FALSE;

		case ITEM_WEAPON:
		pObj->value[1] = UMIN(pObj->level/4 + 1, 5);
		pObj->value[2] = (pObj->level + 7) / pObj->value[1];
		break;
	}

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

ED_FUN_DEC( ed_recval )
{
    MOB_INDEX_DATA *pMob = (MOB_INDEX_DATA *) arg;

    if ( pMob->level < 1 || pMob->level > 60 )
    {
	send_to_char( "Nivel del mob debe ser entre 1 y 60.\n\r", ch );
	return FALSE;
    }

    recalc(pMob);

    send_to_char( "Ok.\n\r", ch );

    return TRUE;
}

bool templookup( char * n_fun, CHAR_DATA *ch, char *argument, void *arg, const void *par, int temp )
{
    int value;
    LOOKUP_F * blah = (LOOKUP_F *) par;

    if ( !emptystring(argument) )
    {
	if ( ( value = ((*blah) (argument)) ) > temp )
	{
	    *(sh_int *) arg = value;
	    printf_to_char( ch, "%s seteado.\n\r",
	    	n_fun );
	    return TRUE;
	}
	else
	{
		printf_to_char( ch, "ERROR : %s inexistente.\n\r",
			n_fun );
		return FALSE;
	}
    }

    printf_to_char( ch, "Sintaxis : %s [%s]\n\r"
    			"Escribe '? %s' para los posibles argumentos.\n\r",
    			n_fun,
    			n_fun,
    			n_fun );

    return FALSE;
}

ED_FUN_DEC( ed_shintposlookup )
{
	return templookup( n_fun, ch, argument, arg, par, 0 );
}

ED_FUN_DEC( ed_shintlookup )
{
	return templookup( n_fun, ch, argument, arg, par, -1 );
}

ED_FUN_DEC( ed_ac )
{
    MOB_INDEX_DATA * pMob = (MOB_INDEX_DATA *) arg;
    char blarg[MAX_INPUT_LENGTH];
    int pierce, bash, slash, exotic;

    do   /* So that I can use break and send the syntax in one place */
    {
	if ( emptystring(argument) )  break;

	argument = one_argument( argument, blarg );

	if ( !is_number( blarg ) )  break;
	pierce = atoi( blarg );
	argument = one_argument( argument, blarg );

	if ( blarg[0] != '\0' )
	{
	    if ( !is_number( blarg ) )  break;
	    bash = atoi( blarg );
	    argument = one_argument( argument, blarg );
	}
	else
	    bash = pMob->ac[AC_BASH];

	if ( blarg[0] != '\0' )
	{
	    if ( !is_number( blarg ) )  break;
	    slash = atoi( blarg );
	    argument = one_argument( argument, blarg );
	}
	else
	    slash = pMob->ac[AC_SLASH];

	if ( blarg[0] != '\0' )
	{
	    if ( !is_number( blarg ) )  break;
	    exotic = atoi( blarg );
	}
	else
	    exotic = pMob->ac[AC_EXOTIC];

	pMob->ac[AC_PIERCE] = pierce;
	pMob->ac[AC_BASH]   = bash;
	pMob->ac[AC_SLASH]  = slash;
	pMob->ac[AC_EXOTIC] = exotic;
	
	send_to_char( "Ac set.\n\r", ch );
	return TRUE;
    } while ( FALSE );    /* Just do it once.. */

    send_to_char( "Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
		  "help MOB_AC  gives a list of reasonable ac-values.\n\r", ch );

    return FALSE;
}

ED_FUN_DEC(ed_dice)
{
    static char syntax[] = "Syntax:  hitdice <number> d <type> + <bonus>\n\r";
    char *numb, *type, *bonus, *cp;
    sh_int * arreglo = (sh_int *) arg;

    if ( emptystring(argument) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    numb = cp = argument;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) ) ++cp;
    if ( *cp != '\0' ) *cp = '\0';

    if ( ( !is_number( numb   ) || atoi( numb   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    arreglo[DICE_NUMBER]	= atoi( numb   );
    arreglo[DICE_TYPE]		= atoi( type  );
    arreglo[DICE_BONUS]		= atoi( bonus );

    printf_to_char( ch, "%s seteado.\n\r",
    	n_fun );

    return TRUE;
}

ED_FUN_DEC( ed_addprog )
{
  int value;
  const struct flag_type * flagtable;
  MPROG_LIST *list, **mprogs = (MPROG_LIST **) arg;
  MPROG_CODE *code;
  MOB_INDEX_DATA *pMob;
  char trigger[MAX_STRING_LENGTH];
  char numb[MAX_STRING_LENGTH];

  argument=one_argument(argument, numb);
  argument=one_argument(argument, trigger);

  if (!is_number(numb) || trigger[0] =='\0' || argument[0] =='\0' )
  {
        send_to_char("Syntax:   addmprog [vnum] [trigger] [phrase]\n\r",ch);
        return FALSE;
  }

  switch(ch->desc->editor)
  {
  	case ED_MOBILE:	flagtable = mprog_flags;	break;
  	default:	send_to_char( "ERROR : Editor invalido.\n\r", ch );
  			return FALSE;
  }

  if ( (value = flag_value (flagtable, trigger) ) == NO_FLAG)
  {
        send_to_char("Valid flags are:\n\r",ch);
        show_help( ch, "mprog");
        return FALSE;
  }

  if ( ( code = pedir_prog (atoi(numb) ) ) == NULL)
  {
        send_to_char("No such MOBProgram.\n\r",ch);
        return FALSE;
  }

  list                  = new_mprog();
  list->vnum            = atoi(numb);
  list->trig_type       = value;
  list->trig_phrase     = str_dup(argument);
  list->code            = code->code;
  list->next            = *mprogs;
  *mprogs		= list;

  switch(ch->desc->editor)
  {
  	case ED_MOBILE:
  	EDIT_MOB(ch, pMob);
	SET_BIT(pMob->mprog_flags,value);
	break;
  }

  send_to_char( "Mprog Added.\n\r",ch);
  return TRUE;
}

ED_FUN_DEC(ed_delprog)
{
    MPROG_LIST *list;
    MPROG_LIST *list_next;
    MPROG_LIST ** mprogs = (MPROG_LIST **) arg;
    MOB_INDEX_DATA *pMob;
    char mprog[MAX_STRING_LENGTH];
    int value;
    int cnt = 0, t2rem;

    one_argument( argument, mprog );

    if (!is_number( mprog ) || mprog[0] == '\0' )
    {
       send_to_char("Syntax:  delmprog [#mprog]\n\r",ch);
       return FALSE;
    }

    value = atoi ( mprog );

    if ( value < 0 )
    {
        send_to_char("Only non-negative mprog-numbers allowed.\n\r",ch);
        return FALSE;
    }

    if ( !(list= *mprogs) )
    {
        send_to_char("MEdit:  Non existant mprog.\n\r",ch);
        return FALSE;
    }

    if ( value == 0)
    {
        list = *mprogs;
        t2rem = list->trig_type;
        *mprogs = list->next;
        free_mprog( list );
    }
    else
    {
        while ( (list_next = list->next) && (++cnt < value ) )
                list = list_next;

        if ( list_next )
        {
		t2rem = list_next->trig_type;
                list->next = list_next->next;
                free_mprog(list_next);
        }
        else
        {
                send_to_char("No such mprog.\n\r",ch);
                return FALSE;
        }
    }

    switch(ch->desc->editor)
    {
  	case ED_MOBILE:
  	EDIT_MOB(ch, pMob);
	REMOVE_BIT(pMob->mprog_flags,t2rem);
	break;
  }

    send_to_char("Mprog removed.\n\r", ch);
    return TRUE;
}

ED_FUN_DEC( ed_ed )
{
    EXTRA_DESCR_DATA *ed;
    EXTRA_DESCR_DATA **pEd = (EXTRA_DESCR_DATA **) arg;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );
    argument = one_argument( argument, keyword );

    if ( command[0] == '\0' )
    {
	send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	send_to_char( "         ed delete [keyword]\n\r", ch );
	send_to_char( "         ed edit [keyword]\n\r", ch );
	send_to_char( "         ed format [keyword]\n\r", ch );
	send_to_char( "         ed rename [keyword]\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( command, "add" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	    return FALSE;
	}

	ed                  =   new_extra_descr();
	ed->keyword         =   str_dup( keyword );
	ed->next            =   *pEd;
	*pEd		    =   ed;

	string_append( ch, &ed->description );

	return TRUE;
    }

    if ( !str_cmp( command, "edit" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed edit [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = *pEd; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	}

	if ( !ed )
	{
	    send_to_char( "ERROR : Descripcion Extra no encontrada.\n\r", ch );
	    return FALSE;
	}

	string_append( ch, &ed->description );

	return TRUE;
    }

    if ( !str_cmp( command, "delete" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed delete [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = *pEd; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
	    send_to_char( "ERROR : Descripcion Extra no encontrada.\n\r", ch );
	    return FALSE;
	}

	if ( !ped )
	    *pEd = ed->next;
	else
	    ped->next = ed->next;

	free_extra_descr( ed );

	send_to_char( "Extra description deleted.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "format" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed format [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = *pEd; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
                send_to_char( "ERROR : Descripcion Extra no encontrada.\n\r", ch );
                return FALSE;
	}

	ed->description = format_string( ed->description );

	send_to_char( "Extra description formatted.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "rename" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed rename [antigua] [nueva]\n\r", ch );
	    return FALSE;
	}

	for ( ed = *pEd; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
                send_to_char( "ERROR : Descripcion Extra no encontrada.\n\r", ch );
                return FALSE;
	}

	free_string(ed->keyword);
	ed->keyword = str_dup( argument );

	send_to_char( "Descripcion extra renombrada.\n\r", ch );
	return TRUE;
    }

    return ed_ed( n_fun, ch, "", arg, par );
}

ED_FUN_DEC( ed_addaffect )
{
    int value;
    AFFECT_DATA *pAf;
    OBJ_INDEX_DATA *pObj = (OBJ_INDEX_DATA *) arg;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];

    argument = one_argument( argument, loc );
    one_argument( argument, mod );

    if ( loc[0] == '\0' || mod[0] == '\0' || !is_number( mod ) )
    {
	send_to_char( "Syntax:  addaffect [location] [#xmod]\n\r", ch );
	return FALSE;
    }

    if ( ( value = flag_value( apply_flags, loc ) ) == NO_FLAG ) /* Hugin */
    {
        send_to_char( "Valid affects are:\n\r", ch );
	show_help( ch, "apply" );
	return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   value;
    pAf->modifier   =   atoi( mod );
    pAf->where	    =   TO_OBJECT;
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Affect added.\n\r", ch);
    return TRUE;
}

ED_FUN_DEC( ed_delaffect )
{
    AFFECT_DATA *pAf;
    AFFECT_DATA *pAf_next;
    AFFECT_DATA **pNaf = (AFFECT_DATA **) arg;
    char affect[MAX_STRING_LENGTH];
    int  value;
    int  cnt = 0;

    one_argument( argument, affect );

    if ( !is_number( affect ) || affect[0] == '\0' )
    {
	send_to_char( "Syntax:  delaffect [#xaffect]\n\r", ch );
	return FALSE;
    }

    value = atoi( affect );

    if ( value < 0 )
    {
	send_to_char( "Only non-negative affect-numbers allowed.\n\r", ch );
	return FALSE;
    }

    if ( !( pAf = *pNaf ) )
    {
	send_to_char( "OEdit:  Non-existant affect.\n\r", ch );
	return FALSE;
    }

    if( value == 0 )	/* First case: Remove first affect */
    {
	pAf = *pNaf;
	*pNaf = pAf->next;
	free_affect( pAf );
    }
    else		/* Affect to remove is not the first */
    {
	while ( ( pAf_next = pAf->next ) && ( ++cnt < value ) )
	     pAf = pAf_next;

	if( pAf_next )		/* See if it's the next affect */
	{
	    pAf->next = pAf_next->next;
	    free_affect( pAf_next );
	}
	else                                 /* Doesn't exist */
	{
	     send_to_char( "No such affect.\n\r", ch );
	     return FALSE;
	}
    }

    send_to_char( "Affect removed.\n\r", ch);
    return TRUE;
}

ED_FUN_DEC( ed_addapply )
{
    int value,bv,typ;
    OBJ_INDEX_DATA *pObj = (OBJ_INDEX_DATA *) arg;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];
    char type[MAX_STRING_LENGTH];
    char bvector[MAX_STRING_LENGTH];

    if ( IS_NULLSTR(argument) )
    {
	send_to_char( "Syntax:  addapply [type] [location] [#xmod] [bitvector]\n\r", ch );
	return FALSE;
    }

    argument = one_argument( argument, type );
    argument = one_argument( argument, loc );
    argument = one_argument( argument, mod );
    one_argument( argument, bvector );

    if ( type[0] == '\0' || ( typ = flag_value( apply_types, type ) ) == NO_FLAG )
    {
    	send_to_char( "Invalid apply type. Valid apply types are:\n\r", ch);
    	show_help( ch, "apptype" );
    	return FALSE;
    }

    if ( loc[0] == '\0' || ( value = flag_value( apply_flags, loc ) ) == NO_FLAG )
    {
        send_to_char( "Valid applys are:\n\r", ch );
	show_help( ch, "apply" );
	return FALSE;
    }

    if ( bvector[0] == '\0' || ( bv = flag_value( bitvector_type[typ].table, bvector ) ) == NO_FLAG )
    {
    	send_to_char( "Invalid bitvector type.\n\r", ch );
	send_to_char( "Valid bitvector types are:\n\r", ch );
	show_help( ch, bitvector_type[typ].help );
    	return FALSE;
    }

    if ( mod[0] == '\0' || !is_number( mod ) )
    {
	send_to_char( "Syntax:  addapply [type] [location] [#xmod] [bitvector]\n\r", ch );
	return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   value;
    pAf->modifier   =   atoi( mod );
    pAf->where	    =   apply_flags[typ].bit;
    pAf->type	    =	-1;
    pAf->duration   =   -1;
    pAf->bitvector  =   bv;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Apply added.\n\r", ch);
    return TRUE;
}

ED_FUN_DEC( ed_value )
{
	return oedit_values( ch, argument, (int) par );
}

ED_FUN_DEC( ed_new_obj )
{
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    int  value;
    int  iHash;

    value = atoi( argument );

    if ( argument[0] == '\0' || value == 0 )
    {
	send_to_char( "Syntax:  oedit create [vnum]\n\r", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );

    if ( !pArea )
    {
	send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "OEdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    if ( get_obj_index( value ) )
    {
	send_to_char( "OEdit:  Object vnum already exists.\n\r", ch );
	return FALSE;
    }
        
    pObj			= new_obj_index();
    pObj->vnum			= value;
    pObj->area			= pArea;
    pObj->extra_flags		= 0;
        
    if ( value > top_vnum_obj )
	top_vnum_obj = value;

    iHash			= value % MAX_KEY_HASH;
    pObj->next			= obj_index_hash[iHash];
    obj_index_hash[iHash]	= pObj;

    set_editor(ch->desc, ED_OBJECT, pObj);
/*    ch->desc->pEdit		= (void *)pObj;
    ch->desc->editor		= ED_OBJECT; */

    send_to_char( "Object Created.\n\r", ch );

    return TRUE;
}

ED_FUN_DEC( ed_race )
{
    MOB_INDEX_DATA *pMob = (MOB_INDEX_DATA *) arg;
    int race;

    if ( argument[0] != '\0'
    && ( race = race_lookup( argument ) ) != 0 )
    {
	pMob->race		 = race;
	pMob->act		|= race_table[race].act;
	pMob->affected_by	|= race_table[race].aff;
	pMob->off_flags		|= race_table[race].off;
	pMob->imm_flags		|= race_table[race].imm;
	pMob->res_flags		|= race_table[race].res;
	pMob->vuln_flags	|= race_table[race].vuln;
	pMob->form		|= race_table[race].form;
	pMob->parts		|= race_table[race].parts;

	send_to_char( "Race set.\n\r", ch );
	return TRUE;
    }

    if ( argument[0] == '?' )
    {
	char buf[MIL];

	send_to_char( "Available races are:", ch );

	for ( race = 0; race_table[race].name != NULL; race++ )
	{
	    if ( ( race % 3 ) == 0 )
		send_to_char( "\n\r", ch );
	    sprintf( buf, " %-15s", race_table[race].name );
	    send_to_char( buf, ch );
	}

	send_to_char( "\n\r", ch );
	return FALSE;
    }

    send_to_char( "Syntax:  race [race]\n\r"
		  "Type 'race ?' for a list of races.\n\r", ch );
    return FALSE;
}

ED_FUN_DEC( ed_olded )
{
	return (* (OLC_FUN *) par) ( ch, argument );
}

ED_FUN_DEC( ed_docomm )
{
	(* (DO_FUN *) par) (ch, argument);
	return FALSE;
}

ED_FUN_DEC( ed_direccion )
{
    return change_exit( ch, argument, (int) par );
}

ED_FUN_DEC( ed_olist )
{
    OBJ_INDEX_DATA	*pObjIndex;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH   ];
    BUFFER		*buf1;
    char		blarg  [ MAX_INPUT_LENGTH    ];
    bool fAll, found;
    int vnum;
    int  col = 0;

    one_argument( argument, blarg );

    if ( blarg[0] == '\0' )
    {
	send_to_char( "Sintaxis : olist <all/name/item_type>\n\r", ch );
	return FALSE;
    }

    pArea	= *(AREA_DATA **) arg;
    buf1	= new_buf();
    fAll	= !str_cmp( blarg, "all" );
    found	= FALSE;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) )
	{
	    if ( fAll || is_name( blarg, pObjIndex->name )
	    || flag_value( type_flags, blarg ) == pObjIndex->item_type )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %-17.16s",
		    pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
		add_buf( buf1, buf );
		if ( ++col % 3 == 0 )
		    add_buf( buf1, "\n\r" );
	    }
	}
    }

    if ( !found )
    {
	send_to_char( "Object(s) not found in this area.\n\r", ch);
	return FALSE;
    }

    if ( col % 3 != 0 )
	add_buf( buf1, "\n\r" );

    page_to_char( buf_string(buf1), ch );
    free_buf(buf1);

    return FALSE;
}

REDIT( redit_limpiar )
{
	ROOM_INDEX_DATA * pRoom;
	int i;

	if ( !IS_NULLSTR( argument ) )
	{
		send_to_char( "Sintaxis : limpiar\n\r", ch );
		return FALSE;
	}

	EDIT_ROOM(ch, pRoom);

	pRoom->sector_type = SECT_INSIDE;
	pRoom->heal_rate	= 100;
	pRoom->mana_rate	= 100;
	pRoom->clan		= 0;
	pRoom->room_flags	= 0;
	free_string( pRoom->owner );
	free_string( pRoom->name );
	free_string( pRoom->description );
	pRoom->owner		= str_dup( "" );
	pRoom->name		= str_dup( "" );
	pRoom->description	= str_dup( "" );

	for ( i = 0; i < MAX_DIR; i++ )
	{
		free_exit(pRoom->exit[i]);
		pRoom->exit[i] = NULL;
	}

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}
