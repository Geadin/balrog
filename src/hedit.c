#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merc.h"
#include "tables.h"
#include "olc.h"
#include "lookup.h"
#include "recycle.h"

#define HEDIT( fun )           bool fun(CHAR_DATA *ch, char*argument)

extern HELP_AREA * had_list;

const struct olc_cmd_type hedit_table[] =
{
/*	{	command		function	}, */

	{	"keyword",	hedit_keyword	},
	{	"texto",	hedit_text	},
	{	"new",		hedit_new	},
	{	"nivel",	hedit_level	},
	{	"commands",	show_commands	},
	{	"delete",	hedit_delete	},
	{	"list",		hedit_list	},
	{	"show",		hedit_show	},
	{	"?",		show_help	},

	{	NULL,		0		}
};

HELP_AREA * get_help_area( HELP_DATA *help )
{
	HELP_AREA * temp;
	HELP_DATA * thelp;

	for ( temp = had_list; temp; temp = temp->next )
		for ( thelp = temp->first; thelp; thelp = thelp->next_area )
			if ( thelp == help )
				return temp;

	return NULL;
}

HEDIT(hedit_show)
{
	HELP_DATA * help;
	char buf[MSL*2];

	EDIT_HELP(ch, help);

	sprintf( buf, "Keyword : [%s]\n\r"
		      "Nivel   : [%d]\n\r"
		      "Texto   :\n\r"
		      "%s-FIN-\n\r",
		      help->keyword,
		      help->level,
		      help->text );
	send_to_char( buf, ch );

	return FALSE;
}

HEDIT(hedit_level)
{
	HELP_DATA *help;
	int lev;

	EDIT_HELP(ch, help);

	if ( IS_NULLSTR(argument) || !is_number(argument) )
	{
		send_to_char( "Sintaxis : nivel [-1..MAX_LEVEL]\n\r", ch );
		return FALSE;
	}

	lev = atoi(argument);

	if ( lev < -1 || lev > MAX_LEVEL )
	{
		printf_to_char( ch, "HEdit : niveles entre -1 y %d solamente.\n\r", MAX_LEVEL );
		return FALSE;
	}

	help->level = lev;
	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

HEDIT(hedit_keyword)
{
	HELP_DATA *help;

	EDIT_HELP(ch, help);

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : keyword [keywords]\n\r", ch );
		return FALSE;
	}

	free_string(help->keyword);
	help->keyword = str_dup(argument);

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

HEDIT(hedit_new)
{
	char arg[MIL], fullarg[MIL];
	HELP_AREA *had;
	HELP_DATA *help;
	extern HELP_DATA *help_last;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : new [nombre]\n\r", ch );
		send_to_char( "           new [area] [nombre]\n\r", ch );
		return FALSE;
	}

	strcpy( fullarg, argument );
	argument = one_argument( argument, arg );

	if ( !(had = had_lookup(arg)) )
	{
		had = ch->in_room->area->helps;
		argument = fullarg;
	}

	if ( help_lookup(argument) )
	{
		send_to_char( "HEdit : help ya existe.\n\r", ch );
		return FALSE;
	}

	if (!had) /* el area no tiene helps */
	{
		had		= new_had();
		had->filename	= str_dup(ch->in_room->area->file_name);
		had->area	= ch->in_room->area;
		had->first	= NULL;
		had->last	= NULL;
		had->changed	= TRUE;
		had->next	= had_list;
		had_list	= had;
		ch->in_room->area->helps = had;
		SET_BIT(ch->in_room->area->area_flags, AREA_CHANGED);
	}

	help		= new_help();
	help->level	= 0;
	help->keyword	= str_dup(argument);
	help->text	= str_dup( "" );

	if (help_last)
		help_last->next	= help;

	if (help_first == NULL)
		help_first = help;

	help_last	= help;
	help->next	= NULL;

	if ( !had->first )
		had->first	= help;
	if ( !had->last )
		had->last	= help;

	had->last->next_area	= help;
	had->last		= help;
	help->next_area		= NULL;

	ch->desc->pEdit		= (HELP_DATA *) help;
	ch->desc->editor	= ED_HELP;

	send_to_char( "Ok.\n\r", ch );
	return FALSE;
}

HEDIT( hedit_text )
{
	HELP_DATA *help;

	EDIT_HELP(ch, help);

	if ( !IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : texto\n\r", ch );
		return FALSE;
	}

	string_append( ch, &help->text );

	return TRUE;
}

void hedit( CHAR_DATA *ch, char *argument)
{
    HELP_DATA * pHelp;
    HELP_AREA *had;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde(argument);
    strcpy(arg, argument);
    argument = one_argument( argument, command);

    EDIT_HELP(ch, pHelp);

    had = get_help_area(pHelp);

    if (had == NULL)
    {
    	bugf( "hedit : had para help %s NULL", pHelp->keyword );
    	edit_done(ch);
    	return;
    }

    if (ch->pcdata->security < 9)
    {
        send_to_char("HEdit: Insuficiente seguridad para editar help.\n\r",ch);
	edit_done(ch);
	return;
    }

    if (command[0] == '\0')
    {
	hedit_show(ch, argument);
        return;
    }

    if (!str_cmp(command, "done") )
    {
        edit_done(ch);
        return;
    }

    for (cmd = 0; hedit_table[cmd].name != NULL; cmd++)
    {
	if (!str_prefix(command, hedit_table[cmd].name) )
	{
		if ((*hedit_table[cmd].olc_fun) (ch, argument))
			had->changed = TRUE;
		return;
	}
    }

    interpret(ch, arg);
    return;
}

void do_hedit(CHAR_DATA *ch, char *argument)
{
	HELP_DATA * pHelp;

	if ( IS_NPC(ch) )
		return;

	if ( (pHelp = help_lookup( argument )) == NULL )
	{
		send_to_char( "HEdit : Help inexistente.\n\r", ch );
		return;
	}

	ch->desc->pEdit		= (void *) pHelp;
	ch->desc->editor	= ED_HELP;

	return;
}

HEDIT(hedit_delete)
{
	HELP_DATA * pHelp, * temp;
	HELP_AREA * had;
	DESCRIPTOR_DATA *d;
	bool found = FALSE;

	EDIT_HELP(ch, pHelp);

	for ( d = descriptor_list; d; d = d->next )
		if ( d->editor == ED_HELP && pHelp == (HELP_DATA *) d->pEdit )
			edit_done(d->character);

	if (help_first == pHelp)
		help_first = help_first->next;
	else
	{
		for ( temp = help_first; temp; temp = temp->next )
			if ( temp->next == pHelp )
				break;

		if ( !temp )
		{
			bugf( "hedit_delete : help %s no encontrado en help_first", pHelp->keyword );
			return FALSE;
		}

		temp->next = pHelp->next;
	}

	for ( had = had_list; had; had = had->next )
		if ( pHelp == had->first )
		{
			found = TRUE;
			had->first = had->first->next_area;
		}
		else
		{
			for ( temp = had->first; temp; temp = temp->next_area )
				if ( temp->next_area == pHelp )
					break;

			if ( temp )
			{
				temp->next_area = pHelp->next_area;
				found = TRUE;
				break;
			}
		}

	if ( !found )
	{
		bugf( "hedit_delete : help %s no encontrado en had_list", pHelp->keyword );
		return FALSE;
	}

	free_help(pHelp);

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

HEDIT(hedit_list)
{
	char buf[MIL];
	int cnt = 0;
	HELP_DATA *pHelp;
	BUFFER *buffer;

	EDIT_HELP(ch, pHelp);

	if ( !str_cmp( argument, "all" ) )
	{
		buffer = new_buf();

		for ( pHelp = help_first; pHelp; pHelp = pHelp->next )
		{
			sprintf( buf, "%3d. %-14.14s%s", cnt, pHelp->keyword,
				cnt % 4 == 3 ? "\n\r" : " " );
			add_buf( buffer, buf );
			cnt++;
		}

		if ( cnt % 4 )
			add_buf( buffer, "\n\r" );

		page_to_char( buf_string(buffer), ch );
		return FALSE;
	}

	if ( !str_cmp( argument, "area" ) )
	{
		if ( ch->in_room->area->helps == NULL )
		{
			send_to_char( "No hay helps en esta area.\n\r", ch );
			return FALSE;
		}

		buffer = new_buf();

		for ( pHelp = ch->in_room->area->helps->first; pHelp; pHelp = pHelp->next_area )
		{
			sprintf( buf, "%3d. %-14.14s%s", cnt, pHelp->keyword,
				cnt % 4 == 3 ? "\n\r" : " " );
			add_buf( buffer, buf );
			cnt++;
		}

		if ( cnt % 4 )
			add_buf( buffer, "\n\r" );

		page_to_char( buf_string(buffer), ch );
		return FALSE;
	}

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : list all\n\r", ch );
		send_to_char( "           list area\n\r", ch );
		return FALSE;
	}

	return FALSE;
}
