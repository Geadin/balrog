/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/
 
/* Online Social Editting Module, 
 * (c) 1996 Erwin S. Andreasen <erwin@pip.dknet.dk>
 * See the file "License" for important licensing information
 */ 

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

#define SEDIT( fun )	bool fun( CHAR_DATA *ch, char *argument )

extern	struct	social_type	xSoc;

	int			maxSocial;	/* max number of socials */
#if !defined(FIRST_BOOT)
	struct	social_type *	social_table;	/* and social table */
#endif

const	struct	olc_comm_type	social_olc_comm_table [] =
{
#if !defined(FIRST_BOOT)
	{ "cnoarg",	&xSoc.char_no_arg,	ed_line_string,	NULL		},
	{ "onoarg",	&xSoc.others_no_arg,	ed_line_string,	NULL		},
	{ "cfound",	&xSoc.char_found,	ed_line_string,	NULL		},
	{ "vfound",	&xSoc.vict_found,	ed_line_string,	NULL		},
	{ "ofound",	&xSoc.others_found,	ed_line_string,	NULL		},
	{ "cself",	&xSoc.char_auto,	ed_line_string,	NULL		},
	{ "oself",	&xSoc.others_auto,	ed_line_string,	NULL		},
	{ "new",	NULL,			ed_olded,	sedit_new	},
	{ "delete",	NULL,			ed_olded,	sedit_delete	},
#endif
	{ "show",	NULL,			ed_olded,	sedit_show	},
	{ "commands",	NULL,			ed_olded,	show_commands	},
	{ "version",	NULL,			ed_olded,	show_version	},
	{ "?",		NULL,			ed_olded,	show_help	},
	{ NULL,		NULL,			NULL,		NULL		}
};

/* Find a social based on name */ 
int social_lookup (const char *name)
{
	int i;
	
	for (i = 0; i < maxSocial ; i++)
		if (!str_cmp(name, social_table[i].name))
			return i;
			
	return -1;
}

void sedit( CHAR_DATA *ch, char *argument)
{
    if (ch->pcdata->security < MIN_SEDIT_SECURITY)
    {
        send_to_char("SEdit: Insuficiente seguridad para modificar social.\n\r",ch);
        edit_done(ch);
        return;
    }

    if (emptystring(argument))
    {
        sedit_show(ch, argument);
        return;
    }

    if (!str_cmp(argument, "done") )
    {
        edit_done(ch);
        return;
    }

    if ( !procesar_comando_olc(ch, argument, social_olc_comm_table) )
    	interpret(ch, argument);

    return;
}

void do_sedit(CHAR_DATA *ch, char *argument)
{
    struct social_type *pSocial;
    char command[MIL];
    int social;

    if ( IS_NPC(ch) || ch->desc == NULL )
    	return;

    if ( IS_NULLSTR(argument) )
    {
    	send_to_char( "Sintaxis : SEdit [social]\n\r", ch );
    	send_to_char( "           SEdit new [social]\n\r", ch );
    	send_to_char( "           SEdit delete [social]\n\r", ch );
    	return;
    }

    if (ch->pcdata->security < MIN_SEDIT_SECURITY)
    {
    	send_to_char( "SEdit : Insuficiente seguridad para editar socials.\n\r", ch );
    	return;
    }

    argument = one_argument( argument, command );

#if !defined(FIRST_BOOT)
    if ( !str_cmp( command, "new" ) )
    {
	if ( sedit_new(ch, argument) )
		grabar_socials();
	return;
    }

    if ( !str_cmp( command, "delete" ) )
    {
    	if ( sedit_delete(ch, argument) )
    		grabar_socials();
    	return;
    }
#endif

    if ( (social = social_lookup(command)) == -1 )
    {
    	send_to_char( "SEdit : Social no existe.\n\r", ch );
    	return;
    }

    pSocial = &social_table[social];

    ch->desc->pEdit=(void *)pSocial;
    ch->desc->editor= ED_SOCIAL;

    return;
}

SEDIT( sedit_show )
{
	struct social_type *pSocial;
	char buf[MSL];
	
	EDIT_SOCIAL(ch,pSocial);
	
	sprintf (buf, "Social: %s\n\r"
	              "(cnoarg) Sin argumento, jugador ve :\n\r"
	              "%s\n\r\n\r"
	              "(onoarg) Sin argumento, otros ven :\n\r"
	              "%s\n\r\n\r"
	              "(cfound) Con objetivo, jugador ve :\n\r"
	              "%s\n\r\n\r"
	              "(ofound) Con objetivo, otros ven :\n\r"
	              "%s\n\r\n\r"
	              "(vfound) Con objetivo, victima ve :\n\r"
	              "%s\n\r\n\r"
	              "(cself) Objetivo es el mismo jugador :\n\r"
	              "%s\n\r\n\r"
	              "(oself) Objetivo es el mismo jugador, otros ven :\n\r"
	              "%s\n\r",

	              pSocial->name,
	              pSocial->char_no_arg,
	              pSocial->others_no_arg,
	              pSocial->char_found,
	              pSocial->others_found,
	              pSocial->vict_found,
	              pSocial->char_auto,
	              pSocial->others_auto);

	send_to_char (buf,ch);		          
	return FALSE;
}

#if !defined(FIRST_BOOT)
SEDIT( sedit_new )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *tch;
	struct social_type *new_table;
	int iSocial;
	
	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : new [nombre-del-nuevo-social]\n\r", ch );
		return FALSE;
	}
	
	iSocial = social_lookup( argument );

	if (iSocial != -1)
	{
		send_to_char ("Un social con ese nombre ya existe!\n\r",ch);
		return FALSE;
	}

	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->connected != CON_PLAYING || (tch = CH(d)) == NULL || tch->desc == NULL )
			continue;

		if ( tch->desc->editor == ED_SOCIAL )
		  	edit_done(ch);
	}

	/* reallocate the table */
	/* Note that the table contains maxSocial socials PLUS one empty spot! */

	maxSocial++;
	new_table = realloc (social_table, sizeof(struct social_type) * (maxSocial + 1));

	if (!new_table) /* realloc failed */
	{
		send_to_char ("Falla en realloc. Preparate para el impacto.\n\r",ch);
		return FALSE;
	}

	social_table = new_table;

	social_table[maxSocial-1].name		= str_dup (argument);
	social_table[maxSocial-1].char_no_arg	= str_dup ("");
	social_table[maxSocial-1].others_no_arg	= str_dup ("");
	social_table[maxSocial-1].char_found	= str_dup ("");
	social_table[maxSocial-1].others_found	= str_dup ("");
	social_table[maxSocial-1].vict_found	= str_dup ("");
	social_table[maxSocial-1].char_auto	= str_dup ("");
	social_table[maxSocial-1].others_auto	= str_dup ("");
	social_table[maxSocial].name		= str_dup (""); /* 'terminating' empty string */

	ch->desc->editor	= ED_SOCIAL;
	ch->desc->pEdit		= (void *) &social_table[maxSocial-1];

	send_to_char ("Nuevo social creado.\n\r",ch);
	return TRUE;
}

SEDIT( sedit_delete )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *tch;
	int i,j,iSocial;
	struct social_type *new_table;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : delete [nombre-del-social-a-borrar]\n\r", ch );
		return FALSE;
	}

	iSocial = social_lookup(argument);
	
	if (iSocial == -1)
	{
		send_to_char( "SEdit : ese social no existe.\n\r", ch );
		return FALSE;
	}

	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->connected != CON_PLAYING || (tch = CH(d)) == NULL || tch->desc == NULL )
			continue;

		if ( tch->desc->editor == ED_SOCIAL )
		  	edit_done(ch);
	}

	new_table = malloc (sizeof(struct social_type) * maxSocial);

	if (!new_table)
	{
		send_to_char ("Memory allocation failed. Brace for impact...\n\r",ch);
		return FALSE;
	}

	/* Copy all elements of old table into new table, except the deleted social */
	for (i = 0, j = 0; i < maxSocial+1; i++)
		if (i != iSocial) /* copy, increase only if copied */
		{
			new_table[j] = social_table[i];
			j++;
		}

	free (social_table);
	social_table = new_table;

	maxSocial--; /* Important :() */

	send_to_char ("Ahora ese social es historia.\n\r",ch);
	return TRUE;
}
#endif
