
#include "include.h"
#include "tables.h"
#include "magic.h"
#include "lookup.h"
#include "olc.h"
#include "recycle.h"

extern bool		fBootDb;

char *			gsn_name( sh_int *pgsn );
char *			spell_name( SPELL_FUN *spell );
SPELL_FUN *		spell_function( char *argument );
sh_int *		gsn_lookup( char *argument );
#define SKEDIT( fun )	bool fun( CHAR_DATA *ch, char *argument )

#if !defined(FIRST_BOOT)
struct	skill_type	*skill_table;
int			MAX_SKILL;
#endif
struct	skhash		*skill_hash_table[26];

void			crear_tabla_hash_skills		( void );
void			borrar_tabla_hash_skills	( void );

#define	SKILL_FILE	DATA_DIR "skills"

struct	gsn_type
{
	char *		name;
	sh_int *	pgsn;
};

struct	spell_type
{
	char *		name;
	SPELL_FUN *	spell;
};

#if defined(SPELL)
#undef SPELL
#endif
#define SPELL(spell)	{	#spell,		spell		},

const	struct	spell_type	spell_table	[]	=
{
#include "magic.h"
	{	NULL,	NULL	}
};

#undef SPELL
#define SPELL(spell) DECLARE_SPELL_FUN(spell);

#define GSN(x)	{	#x,	&x	},

const	struct	gsn_type	gsn_table	[]	=
{
#include "gsn.h"
	{	NULL,			NULL			} 
};

extern struct skill_type xSkill;

const	struct	olc_comm_type	skill_olc_comm_table	[]	=
{
#if !defined(FIRST_BOOT)
 { "name",	NULL,				ed_olded,		skedit_name	},
 { "beats",	(void *) &xSkill.beats,		ed_numero_s_pos,	NULL		},
 { "position",	(void *) &xSkill.minimum_position,	ed_shintlookup,		position_lookup	},
 { "slot",	NULL,				ed_olded,		skedit_slot	},
 { "target",	(void *) &xSkill.target,	ed_flag_set_sh,		target_table	},
 { "mana",	(void *) &xSkill.min_mana,	ed_numero_s_pos,	NULL		},
 { "nivel",	NULL,				ed_olded,		skedit_nivel 	},
 { "rating",	NULL,				ed_olded,		skedit_rating   },
 { "gsn",	NULL,				ed_olded,		skedit_gsn	},
 { "spell",	NULL,				ed_olded,		skedit_spell	},
 { "noun",	(void *) &xSkill.noun_damage,	ed_line_string,		NULL		},
 { "off",	(void *) &xSkill.msg_off,	ed_line_string,		NULL 		},
 { "obj",	(void *) &xSkill.msg_obj,	ed_line_string,		NULL		},
 { "new",	NULL,				ed_olded,		skedit_new	},
#endif
 { "list",	NULL,				ed_olded,		skedit_list	},
 { "show",	NULL,				ed_olded,		skedit_show	},
 { "commands",	NULL,				ed_olded,		show_commands	},
 { "?",		NULL,				ed_olded,		show_help	},
 { "version",	NULL,				ed_olded,		show_version	},
 { NULL,	NULL,				NULL,			NULL		}
};

char *spell_name( SPELL_FUN *spell )
{
	int i = 0;
	
	if ( spell == NULL )
		return "";

	while ( spell_table[i].name )
		if ( spell_table[i].spell == spell )
			return spell_table[i].name;
		else
			i++;

	if ( fBootDb )
		bug( "spell_name : spell_fun inexistente", 0 );

	return "";
}

char *gsn_name( sh_int *pgsn )
{
	int i = 0;
	
	if ( pgsn == NULL )
		return "";

	while ( gsn_table[i].name )
		if ( gsn_table[i].pgsn == pgsn )
			return gsn_table[i].name;
		else
			i++;
	
	if ( fBootDb )
		bug( "gsn_name : pgsn %d inexistente", *pgsn );

	return "";
}

SPELL_FUN *spell_function( char *argument )
{
	int i;
	char buf[MSL];
	
	if ( IS_NULLSTR(argument) )
		return NULL;

	for ( i = 0; spell_table[i].name; ++i )
		if ( !str_cmp(spell_table[i].name, argument) )
			return spell_table[i].spell;
	
	if ( fBootDb )
	{
		sprintf( buf, "spell_function : spell %s inexistente", argument );
		bug( buf, 0 );
	}

	return spell_null;
}

void	check_gsns( void )
{
	int i;

	for ( i = 0; gsn_table[i].name; ++i )
		if ( *gsn_table[i].pgsn == 0 )
			bugf( "check_gsns : gsn %d(%s) no asignado!", i,
				gsn_table[i].name );

	return;
}

sh_int *gsn_lookup( char *argument )
{
	int i;
	char buf[MSL];

	if ( IS_NULLSTR(argument) )
		return NULL;

	for ( i = 0; gsn_table[i].name; ++i )
		if ( !str_cmp(gsn_table[i].name, argument) )
			return gsn_table[i].pgsn;
	
	if ( fBootDb == TRUE )
	{
		sprintf( buf, "gsn_lookup : gsn %s inexistente", argument );
		bug( buf, 0 );
	}

	return NULL;
}

void    skedit (CHAR_DATA * ch, char *argument)
{
	if (ch->pcdata->security < MIN_SKEDIT_SECURITY)
	{
		send_to_char ("SKEdit : Seguridad insuficiente para editar skills.\n\r", ch);
		edit_done (ch);
		return;
	}

	if (!str_cmp (argument, "done"))
	{
		edit_done (ch);
		return;
	}

	if (!str_cmp (argument, "save"))
	{
		grabar_skills();
		return;
	}

	if ( emptystring(argument) )
	{
		skedit_show (ch, argument);
		return;
	}

	/* Search Table and Dispatch Command. */
	if ( !procesar_comando_olc(ch, argument, skill_olc_comm_table) )
		interpret(ch, argument);

	return;
}

void do_skedit(CHAR_DATA *ch, char *argument)
{
    struct skill_type *pSkill;
    char command[MSL];
    int skill;

    if ( IS_NPC(ch) )
    	return;

    if ( IS_NULLSTR(argument) )
    {
    	send_to_char( "Sintaxis : SKEdit [skill]\n\r", ch );
    	return;
    }

    if (ch->pcdata->security < MIN_SKEDIT_SECURITY)
    {
    	send_to_char( "SKEdit : Insuficiente seguridad para editar skills.\n\r", ch );
    	return;
    }

    one_argument( argument, command );

#if !defined(FIRST_BOOT)
    if ( !str_cmp( command, "new" ) )
    {
	argument = one_argument( argument, command );
	if ( skedit_new(ch, argument) )
		grabar_skills();
	return;
    }
#endif

    if ( (skill = skill_lookup(argument)) == -1 )
    {
    	send_to_char( "SKEdit : Skill no existe.\n\r", ch );
    	return;
    }

    pSkill = &skill_table[skill];

    ch->desc->pEdit=(void *)pSkill;
    ch->desc->editor= ED_SKILL;

    return;
}

void skill_list( BUFFER *pBuf )
{
	char buf[MSL];
	int i;

	sprintf( buf, "Niv   %-20.20s Niv   %-20.20s Niv   %-20.20s\n\r",
			"Nombre", "Nombre", "Nombre" );
	add_buf( pBuf, buf );

	for ( i = 0; i < MAX_SKILL; ++i )
	{
		sprintf( buf, "#B%3d#b %c %-20.20s", i,
		skill_table[i].spell_fun == spell_null ? '-' : '+',
		skill_table[i].name );
		if ( i % 3 == 2 )
			strcat( buf, "\n\r" );
		else
			strcat( buf, " " );
		add_buf( pBuf, buf );
	}

	if ( i % 3 != 0 )
		add_buf( pBuf, "\n\r" );
}

void spell_list( BUFFER *pBuf )
{
	char buf[MSL];
	int i;
	
	sprintf( buf, "Num %-35.35s Num %-35.35s\n\r",
			"Nombre", "Nombre" );
	add_buf( pBuf, buf );

	for ( i = 0; spell_table[i].name; ++i )
	{
		sprintf( buf, "#B%3d#b %-35.35s", i,
		spell_table[i].name );
		if ( i % 2 == 1 )
			strcat( buf, "\n\r" );
		else
			strcat( buf, " " );
		add_buf( pBuf, buf );
	}

	if ( i % 2 != 0 )
		add_buf( pBuf, "\n\r" );
}

void gsn_list( BUFFER *pBuf )
{
	char buf[MSL];
	int i;
	
	sprintf( buf, "Num %-22.22s Num %-22.2s Num %-22.22s\n\r",
			"Nombre", "Nombre", "Nombre" );
	add_buf( pBuf, buf );

	for ( i = 0; gsn_table[i].name; ++i )
	{
		sprintf( buf, "#B%3d#b %-22.22s", i,
		gsn_table[i].name );
		if ( i % 3 == 2 )
			strcat( buf, "\n\r" );
		else
			strcat( buf, " " );
		add_buf( pBuf, buf );
	}

	if ( i % 3 != 0 )
		add_buf( pBuf, "\n\r" );
}

void slot_list( BUFFER *pBuf )
{
	char buf[MSL];
	int i, cnt;
	
	sprintf( buf, "Num %-22.22s Num %-22.2s Num %-22.22s\n\r",
			"Nombre", "Nombre", "Nombre" );
	add_buf( pBuf, buf );

	cnt = 0;
	for ( i = 0; i < MAX_SKILL; ++i )
	{
		if ( skill_table[i].slot )
		{
			sprintf( buf, "#B%3d#b %-22.22s",
				skill_table[i].slot,
				skill_table[i].name );
			if ( cnt % 3 == 2 )
				strcat( buf, "\n\r" );
			else
				strcat( buf, " " );
			add_buf( pBuf, buf );
			cnt++;
		}
	}

	if ( cnt % 3 != 0 )
		add_buf( pBuf, "\n\r" );
}

SKEDIT( skedit_list )
{
	BUFFER *pBuf;
	
	if ( IS_NULLSTR(argument) || !is_name(argument, "gsns skills spells slots") )
	{
		send_to_char( "Sintaxis : list [gsns/skills/spells/slots]\n\r", ch );
		return FALSE;
	}

	pBuf = new_buf();

	if ( !str_prefix( argument, "skills" ) )
		skill_list(pBuf);
	else if ( !str_prefix( argument, "spells" ) )
		spell_list(pBuf);
	else if ( !str_prefix( argument, "slots" ) )
		slot_list(pBuf);
	else if ( !str_prefix( argument, "gsns" ) )
		gsn_list(pBuf);
	else
		add_buf( pBuf, "Idiota.\n\r" );

	page_to_char( buf_string(pBuf), ch );

	free_buf(pBuf);

	return FALSE;
}

SKEDIT( skedit_show )
{
	struct skill_type *pSkill;
	char buf[MAX_STRING_LENGTH];
	char buf2[MSL];
	int i, sn = 0;

	EDIT_SKILL( ch, pSkill );

	sprintf( buf,	"Name     : [%s]\n\r",
		pSkill->name );
	send_to_char( buf, ch );

	while ( (sn < MAX_SKILL) && (pSkill != &skill_table[sn]) )
		sn++;

	if ( sn != MAX_SKILL )
	{
		sprintf( buf, "Sn       : [%3d/%3d]\n\r", sn, MAX_SKILL );
		send_to_char( buf, ch );
	}

	sprintf( buf, "Clase    + " );
	
	for ( i = 0; i < MAX_CLASS; ++i )
	{
		strcat( buf, class_table[i].who_name );
		strcat( buf, " " );
	}
	
	strcat( buf, "\n\r" );
	send_to_char( buf, ch );
	
	sprintf( buf, "Nivel    | " );

	for ( i = 0; i < MAX_CLASS; ++i )
	{
		sprintf( buf2, "%3d ", pSkill->skill_level[i] );
		strcat( buf, buf2 );
	}
	
	strcat( buf, "\n\r" );
	send_to_char( buf, ch );
	
	sprintf( buf, "Rating   | " );
	
	for ( i = 0; i < MAX_CLASS; ++i )
	{
		sprintf( buf2, "%3d ", pSkill->rating[i] );
		strcat( buf, buf2 );
	}
	
	strcat( buf, "\n\r" );
	send_to_char( buf, ch );
	
	sprintf( buf, "Spell    : [%s]\n\r", spell_name(pSkill->spell_fun) );
	send_to_char( buf, ch );
	
	sprintf( buf, "Target   : [%s]\n\r", flag_string(target_table,pSkill->target) );
	send_to_char( buf, ch );
	
	sprintf( buf, "Min Pos  : [%s]\n\r", position_table[pSkill->minimum_position].name );
	send_to_char( buf, ch );
	
	sprintf( buf, "pGsn     : [%s]\n\r", gsn_name(pSkill->pgsn) );
	send_to_char( buf, ch );
	
	sprintf( buf, "Slot     : [%d]\n\r", pSkill->slot );
	send_to_char( buf, ch );
	
	sprintf( buf, "Min Mana : [%d]\n\r", pSkill->min_mana );
	send_to_char( buf, ch );
	
	sprintf( buf, "Beats    : [%d], #B%.2f#b segundo(s).\n\r",
		pSkill->beats,
		pSkill->beats / (float) PULSE_PER_SECOND );
	send_to_char( buf, ch );

	sprintf( buf, "Noun Dmg : [%s]\n\r", pSkill->noun_damage );
	send_to_char( buf, ch );
	
	sprintf( buf, "Msg Off  : [%s]\n\r", pSkill->msg_off );
	send_to_char( buf, ch );
	
	sprintf( buf, "Msg Obj  : [%s]\n\r", pSkill->msg_obj );
	send_to_char( buf, ch );
	
	return FALSE;
}

void crear_tabla_hash_skills( void )
{
	int sn, valor;
	struct skhash *dato, *temp;

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
		if ( IS_NULLSTR(skill_table[sn].name) )
			continue;

		valor = (int) (LOWER(skill_table[sn].name[0]) - 'a');

		if ( valor < 0 || valor > 25 )
		{
			bug( "Crear_tabla_hash_skills : valor %d invalido", valor );
			exit(1);
		}

		dato				= new_skhash();
		dato->sn			= sn;

		/* Ahora linkear a la tabla */
		if ( skill_hash_table[valor] && (skill_table[sn].spell_fun == spell_null) ) /* skill */
		{
			/* Skills van al final! */
			for ( temp = skill_hash_table[valor]; temp; temp = temp->next )
				if ( temp->next == NULL )
					break;

			if ( !temp || temp->next )
			{
				bug( "Skill_hash_table : ???", 0 );
				exit(1);
			}

			temp->next		= dato;
			dato->next		= NULL;
		}
		else
		{
			dato->next			= skill_hash_table[valor];
			skill_hash_table[valor]		= dato;
		}
	}
}

void borrar_tabla_hash_skills( void )
{
	struct skhash *temp, *temp_next;
	int i;

	for ( i = 0; i < 26; ++i )
	{
		for ( temp = skill_hash_table[i]; temp; temp = temp_next )
		{
			temp_next = temp->next;
			free_skhash( temp );
		}
		skill_hash_table[i] = NULL;
	}
}

#if !defined(FIRST_BOOT)
SKEDIT( skedit_name )
{
	struct skill_type *pSkill;
	
	EDIT_SKILL( ch, pSkill );
	
	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : name [nombre]\n\r", ch );
		return FALSE;
	}
	
	if ( skill_lookup(argument) != -1 )
	{
		send_to_char( "Un skill/spell con ese nombre ya existe.\n\r", ch );
		return FALSE;
	}

	free_string(pSkill->name);
	pSkill->name = str_dup(argument);
	
	borrar_tabla_hash_skills();
	crear_tabla_hash_skills();

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

SKEDIT( skedit_slot )
{
	struct skill_type *pSkill;
	
	EDIT_SKILL( ch, pSkill );
	
	if ( IS_NULLSTR(argument) || !is_number(argument) || atoi(argument) < 0 )
	{
		send_to_char( "Sintaxis : slot [numero]\n\r", ch );
		return FALSE;
	}

	if ( slot_lookup(atoi(argument)) != -1 )
	{
		send_to_char( "Ese slot ya esta ocupado.\n\r", ch );
		return FALSE;
	}
	
	pSkill->slot = atoi(argument);
	
	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

SKEDIT( skedit_nivel )
{
	struct skill_type *pSkill;
	char arg[MIL];
	int clase, nivel;
	
	EDIT_SKILL(ch,pSkill);
	
	argument = one_argument( argument, arg );

	if ( IS_NULLSTR(argument) || IS_NULLSTR(arg) || !is_number(argument) )
	{
		send_to_char( "Sintaxis : nivel [clase] [nivel]\n\r", ch );
		return FALSE;
	}

	if ( (clase = class_lookup(arg)) == -1 )
	{
		send_to_char( "SKEdit : Clase inexistente.\n\r", ch );
		return FALSE;
	}

	nivel = atoi(argument);

	if ( nivel < 0 || nivel > MAX_LEVEL )
	{
		send_to_char( "SKEdit : Nivel invalido.\n\r", ch );
		return FALSE;
	}

	pSkill->skill_level[clase] = nivel;
	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

SKEDIT( skedit_rating )
{
	struct skill_type *pSkill;
	char arg[MIL];
	int rating, clase;

	EDIT_SKILL(ch,pSkill);

	argument = one_argument( argument, arg );

	if ( IS_NULLSTR(argument) || IS_NULLSTR(arg) || !is_number(argument) )
	{
		send_to_char( "Sintaxis : rating [clase] [nivel]\n\r", ch );
		return FALSE;
	}

	if ( (clase = class_lookup(arg)) == -1 )
	{
		send_to_char( "SKEdit : Clase inexistente.\n\r", ch );
		return FALSE;
	}

	rating = atoi(argument);

	if ( rating < 0 )
	{
		send_to_char( "SKEdit : Rating invalido.\n\r", ch );
		return FALSE;
	}

	pSkill->rating[clase] = rating;
	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

SKEDIT( skedit_spell )
{
	struct skill_type *pSkill;
	SPELL_FUN *spell;
	
	EDIT_SKILL( ch, pSkill );
	
	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : spell [nombre-spell]\n\r", ch );
		send_to_char( "           spell spell_null\n\r", ch );
		return FALSE;
	}
	
	if ( ( spell = spell_function(argument) ) == spell_null
	    && str_cmp(argument,"spell_null") )
	{
		send_to_char( "SKEdit : Spell inexistente.\n\r", ch );
		return FALSE;
	}
	
	pSkill->spell_fun = spell;
	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

SKEDIT( skedit_gsn )
{
	struct skill_type *pSkill;
	sh_int *gsn;
	int sn;
	
	EDIT_SKILL( ch, pSkill );
	
	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : gsn [nombre-gsn]\n\r", ch );
		send_to_char( "           gsn null\n\r", ch );
		return FALSE;
	}
	
	gsn = NULL;

	if ( str_cmp(argument,"null") && ( gsn = gsn_lookup(argument) ) == NULL )
	{
		send_to_char( "SKEdit : Gsn inexistente.\n\r", ch );
		return FALSE;
	}
	
	pSkill->pgsn = gsn;
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].pgsn != NULL )
		*skill_table[sn].pgsn = sn;
	}

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

SKEDIT( skedit_new )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *tch;
	struct skill_type *new_table;
	bool *tempgendata;
	sh_int *templearned;
	int i;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : new [nombre-de-nuevo-skill]\n\r", ch );
		return FALSE;
	}

	if (skill_lookup(argument) != -1)
	{
		send_to_char ("Un skill con ese nombre ya existe!\n\r",ch);
		return FALSE;
	}

	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->connected != CON_PLAYING || (tch = CH(d)) == NULL || tch->desc == NULL )
			continue;

		if ( tch->desc->editor == ED_SKILL )
			edit_done(tch);
	}

	/* reallocate the table */
	MAX_SKILL++;
	new_table = realloc (skill_table, sizeof(struct skill_type) * (MAX_SKILL + 1));

	if (!new_table) /* realloc failed */
	{
		send_to_char ("Falla en realloc. Preparate para el impacto.\n\r",ch);
		return FALSE;
	}

	skill_table					= new_table;

	skill_table[MAX_SKILL-1].name			= str_dup (argument);
	for ( i = 0; i < MAX_CLASS; ++i )
	{
		skill_table[MAX_SKILL-1].skill_level[i]	= 53;
		skill_table[MAX_SKILL-1].rating[i]	= 0;
	}
	skill_table[MAX_SKILL-1].spell_fun		= spell_null;
	skill_table[MAX_SKILL-1].target			= TAR_IGNORE;
	skill_table[MAX_SKILL-1].minimum_position	= POS_STANDING;
	skill_table[MAX_SKILL-1].pgsn			= NULL;
	skill_table[MAX_SKILL-1].slot			= 0;
	skill_table[MAX_SKILL-1].min_mana		= 0;
	skill_table[MAX_SKILL-1].beats			= 0;
	skill_table[MAX_SKILL-1].noun_damage		= str_dup( "" );
	skill_table[MAX_SKILL-1].msg_off		= str_dup( "" );
	skill_table[MAX_SKILL-1].msg_obj		= str_dup( "" );

	skill_table[MAX_SKILL].name			= NULL;

	for ( d = descriptor_list; d; d = d->next )
	{
		if ( (d->connected == CON_PLAYING)
		||  ((tch = CH(d)) == NULL)
		||  (tch->gen_data == NULL) )
			continue;

		tempgendata = realloc( tch->gen_data->skill_chosen, sizeof( bool ) * MAX_SKILL );
		tch->gen_data->skill_chosen			= tempgendata;
		tch->gen_data->skill_chosen[MAX_SKILL-1]	= 0;
	}

	for ( tch = char_list; tch; tch = tch->next )
		if ( !IS_NPC(tch) )
		{
			templearned = new_learned();

			/* copiamos los valores */
			for ( i = 0; i < MAX_SKILL - 1; ++i )
				templearned[i] = tch->pcdata->learned[i];

			free_learned(tch->pcdata->learned);
			tch->pcdata->learned = templearned;
		}

	borrar_tabla_hash_skills();
	crear_tabla_hash_skills();
	ch->desc->editor	= ED_SKILL;
	ch->desc->pEdit		= (void *) &skill_table[MAX_SKILL-1];

	send_to_char ("Nuevo skill creado.\n\r",ch);
	return TRUE;
}
#endif
