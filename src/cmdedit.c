#include "include.h"
#include "interp.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include "recycle.h"

#define CMD_FILE DATA_DIR "comandos"
#define CMDEDIT( fun )	bool fun( CHAR_DATA *ch, char *argument )

void	grabar_tabla_comandos( void );

struct cmd_list_type
{
	char *name;
	DO_FUN *funcion;
};

extern	bool		fBootDb;
int			MAX_CMD;
#if !defined(FIRST_BOOT)
struct	cmd_type	*cmd_table;
#endif
void			crear_tabla_com( void );

#if defined(COMMAND)
#undef COMMAND
#endif

#define COMMAND(cmd)	{	#cmd,	cmd	},

const	struct	cmd_list_type	cmd_list	[]	=
{
#include "command.h"
	{	NULL,		NULL		}
};

#undef COMMAND
#define COMMAND(cmd)	DECLARE_DO_FUN(cmd);

extern struct cmd_type xCmd;

const	struct	olc_comm_type	cmd_olc_comm_table[]	=
{
#if !defined(FIRST_BOOT)
	{ "name",	NULL,		ed_olded,	cmdedit_name	},
	{ "funcion",	NULL,		ed_olded,	cmdedit_funcion	},
	{ "nivel",	NULL,		ed_olded,	cmdedit_nivel	},
	{ "posicion",	&xCmd.position,	ed_shintlookup,	position_lookup	},
	{ "log",	&xCmd.log,	ed_flag_set_long,log_flags	},
	{ "tipo",	&xCmd.show,	ed_flag_set_long,show_flags	},
	{ "new",	NULL,		ed_olded,	cmdedit_new	},
	{ "delete",	NULL,		ed_olded,	cmdedit_delete	},
#endif
	{ "list",	NULL,		ed_olded,	cmdedit_list	},
	{ "show",	NULL,		ed_olded,	cmdedit_show	},
	{ "commands",	NULL,		ed_olded,	show_commands	},
	{ "?",		NULL,		ed_olded,	show_help	},
	{ "version",	NULL,		ed_olded,	show_version	},
	{ NULL,		NULL,		NULL,		NULL		}
};

char *	cmd_func_name( DO_FUN *command )
{
	int cmd;

	for ( cmd = 0; cmd_list[cmd].name; cmd++ )
		if ( cmd_list[cmd].funcion == command )
			return cmd_list[cmd].name;

	return "";
}

DO_FUN *	cmd_func_lookup( char *arg )
{
	int cmd;

	for ( cmd = 0; cmd_list[cmd].name; cmd++ )
		if ( !str_cmp(cmd_list[cmd].name, arg) )
			return cmd_list[cmd].funcion;

	if ( fBootDb )
	{
		bugf( "Cmd_func_lookup : funcion %s inexistente.", arg );
		return do_nothing;
	}

	return NULL;
}

int cmd_lookup( char *arg )
{
	int cmd;
	
	for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
		if ( LOWER(arg[0]) == LOWER(cmd_table[cmd].name[0])
		&&   !str_prefix( arg, cmd_table[cmd].name ) )
			return cmd;

	return -1;
}

void cmdedit( CHAR_DATA *ch, char *argument)
{
    if (ch->pcdata->security < MIN_CMDEDIT_SECURITY)
    {
        send_to_char("CMDEdit: Insuficiente seguridad para modificar comando.\n\r",ch);
        edit_done(ch);
        return;
    }

    if (emptystring(argument))
    {
        cmdedit_show(ch, argument);
        return;
    }

    if (!str_cmp(argument, "done") )
    {
        edit_done(ch);
        return;
    }

    if (!str_cmp(argument, "save") )
    {
	send_to_char("Grabando tabla comandos...", ch);
    	grabar_tabla_comandos();
    	send_to_char("hecho.\n\r", ch);
    	return;
    }

    /* Search Table and Dispatch Command. */
    if ( !procesar_comando_olc(ch, argument, cmd_olc_comm_table) )
	interpret(ch, argument);

    return;
}

void do_cmdedit(CHAR_DATA *ch, char *argument)
{
    struct cmd_type *pCmd;
    char command[MSL];
    int comando;

    if ( IS_NPC(ch) )
    	return;

    if ( IS_NULLSTR(argument) )
    {
    	send_to_char( "Sintaxis : CMDEdit [comando]\n\r", ch );
    	return;
    }

    if (ch->pcdata->security < MIN_CMDEDIT_SECURITY)
    {
    	send_to_char( "CMDEdit : Insuficiente seguridad para editar comandos.\n\r", ch );
    	return;
    }

    argument = one_argument( argument, command );

#if !defined(FIRST_BOOT)
    if ( !str_cmp( command, "new" ) )
    {
	if ( cmdedit_new(ch, argument) )
		grabar_tabla_comandos();
	return;
    }

    if ( !str_cmp( command, "delete" ) )
    {
    	if ( cmdedit_delete(ch, argument) )
    		grabar_tabla_comandos();
    	return;
    }
#endif

    if ( (comando = cmd_lookup(command)) == -1 )
    {
    	send_to_char( "CMDEdit : Comando no existe.\n\r", ch );
    	return;
    }

    pCmd = &cmd_table[comando];

    ch->desc->pEdit=(void *)pCmd;
    ch->desc->editor= ED_CMD;

    return;
}

CMDEDIT( cmdedit_show )
{
	struct	cmd_type *pCmd;
	char buf[MIL];
	
	EDIT_CMD( ch, pCmd );
	
	sprintf( buf, "Name     : [%s]\n\r", pCmd->name );
	send_to_char( buf, ch );
	
	sprintf( buf, "Funcion  : [%s]\n\r", cmd_func_name(pCmd->do_fun) );
	send_to_char( buf, ch );
	
	sprintf( buf, "Nivel    : [%d]\n\r", pCmd->level );
	send_to_char( buf, ch );
	
	sprintf( buf, "Posicion : [%s]\n\r", position_table[pCmd->position].name );
	send_to_char( buf, ch );
	
	sprintf( buf, "Log      : [%s]\n\r", flag_string( log_flags, pCmd->log ) );
	send_to_char( buf, ch );
	
	sprintf( buf, "Tipo     : [%s]\n\r", flag_string( show_flags, pCmd->show ) );
	send_to_char( buf, ch );
	
	return FALSE;
}

void list_funciones(BUFFER *pBuf)
{
	int i;
	char buf[MSL];
	
	sprintf( buf, "#UNum %-13.13s Num %-13.13s Num %-13.13s Num %-13.13s#u\n\r",
		"Nombre", "Nombre", "Nombre", "Nombre" );
	add_buf( pBuf, buf );

	for ( i = 0; cmd_list[i].name; i++ )
	{
		sprintf( buf, "#B%3d#b %-13.13s", i, cmd_list[i].name );
		if ( i % 4 == 3 )
			strcat( buf, "\n\r" );
		else
			strcat( buf, " " );
		add_buf(pBuf, buf);
	}

	if ( i % 4 != 0 )
		add_buf(pBuf, "\n\r" );
}

void list_commands(BUFFER *pBuf, int minlev, int maxlev)
{
	char buf[MSL];
	int i, cnt = 0;

	sprintf( buf, "#UNv %-12.12s %-13.13s Pos Log Nv %-12.12s %-13.13s Pos Log#u\n\r",
		"Nombre",	"Funcion",	"Nombre",	"Funcion" );
	add_buf( pBuf, buf );

	for ( i = 0; i < MAX_CMD; ++i )
	{
		if ( cmd_table[i].level < minlev || cmd_table[i].level > maxlev )
			continue;

		sprintf( buf, "%2d #B%-12.12s#b %-13.13s %-3.3s %-3.3s",
			cmd_table[i].level,
			cmd_table[i].name,
			cmd_func_name(cmd_table[i].do_fun),
			position_table[cmd_table[i].position].name,
			&(flag_string(log_flags,cmd_table[i].log))[4] );
		if ( cnt % 2 == 1 )
			strcat(buf,"\n\r");
		else
			strcat(buf," ");
		add_buf(pBuf,buf);
		cnt++;
	}

	if ( cnt % 2 != 0 )
		add_buf(pBuf,"\n\r");
}

CMDEDIT( cmdedit_list )
{
	BUFFER *pBuf;
	char arg[MIL], arg2[MIL];
	int minlev, maxlev;

	argument = one_argument( argument, arg );

	if ( IS_NULLSTR(arg) || !is_name(arg, "comandos funciones") )
	{
		send_to_char( "Sintaxis : list [comandos/funciones] [nivel min] [nivel max]\n\r", ch );
		return FALSE;
	}

	minlev = 0;
	maxlev = MAX_LEVEL;

	if ( !IS_NULLSTR(argument) )
	{
		argument = one_argument( argument, arg2 );

		if ( !is_number( arg2 ) )
		{
			send_to_char( "CMDEdit : debes poner un nivel como argumento.\n\r", ch );
			return FALSE;
		}

		minlev = atoi( arg2 );

		if ( !IS_NULLSTR(argument) )
			maxlev = atoi( argument );
		else
			maxlev = 0;

		if ( maxlev < 1 || maxlev > MAX_LEVEL )
			maxlev = minlev;
	}

	pBuf = new_buf();

	if ( !str_prefix(arg,"comandos") )
		list_commands(pBuf,minlev,maxlev);
	else if ( !str_prefix(arg,"funciones") )
		list_funciones(pBuf);
	else
		add_buf(pBuf, "Idiota.\n\r");

	page_to_char(buf_string(pBuf),ch);
	free_buf(pBuf);
	return FALSE;
}

void do_nothing( CHAR_DATA *ch, char *argument )
{
	return;
}

#if !defined(FIRST_BOOT)
CMDEDIT( cmdedit_name )
{
	struct cmd_type *pCmd;
	int cmd;
	
	EDIT_CMD(ch,pCmd);
	
	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : name [nuevo-nombre]\n\r", ch );
		return FALSE;
	}
	
	if ( cmd_lookup(argument) != -1 )
	{
		send_to_char( "CMDEdit : Un comando con ese nombre ya existe.\n\r", ch );
		return FALSE;
	}
	
	free_string(pCmd->name);
	pCmd->name = str_dup( argument );
	
	crear_tabla_com();

	cmd = cmd_lookup(argument);
	if ( cmd != -1 )
		ch->desc->pEdit = &cmd_table[cmd];
	else
		bugf( "Cmdedit_name : cmd_lookup retorno -1 a %s!", argument );

	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

CMDEDIT( cmdedit_new )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *tch;
	struct cmd_type *new_table;
	int cmd;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : new [nombre-de-nuevo-comando]\n\r", ch );
		return FALSE;
	}

	cmd = cmd_lookup( argument );

	if (cmd != -1)
	{
		send_to_char ("CMDEdit : Un comando con ese nombre ya existe.\n\r",ch);
		return FALSE;
	}

	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->connected != CON_PLAYING || (tch = CH(d)) == NULL || tch->desc == NULL )
			continue;

		if ( tch->desc->editor == ED_CMD )
		  	edit_done(tch);
	}

	/* reallocate the table */

	MAX_CMD++;
	new_table = realloc (cmd_table, sizeof(struct cmd_type) * (MAX_CMD + 1));

	if (!new_table) /* realloc failed */
	{
		send_to_char ("Falla en realloc. Preparate para el impacto.\n\r",ch);
		return FALSE;
	}

	cmd_table = new_table;

	cmd_table[MAX_CMD-1].name		= str_dup (argument);
	cmd_table[MAX_CMD-1].do_fun		= do_nothing;
	cmd_table[MAX_CMD-1].position		= position_lookup( "standing" );
	cmd_table[MAX_CMD-1].level		= MAX_LEVEL;
	cmd_table[MAX_CMD-1].log		= LOG_ALWAYS;
	cmd_table[MAX_CMD-1].show		= TYP_NUL;

	cmd_table[MAX_CMD].name			= str_dup( "" );

	crear_tabla_com();

	cmd = cmd_lookup(argument);
	if ( cmd != -1 )
		ch->desc->pEdit = &cmd_table[cmd];
	else
		bugf( "Cmdedit_new : cmd_lookup retorno -1 a %s!", argument );

	ch->desc->editor	= ED_CMD;

	send_to_char ("Nuevo comando creado.\n\r",ch);
	return TRUE;
}

CMDEDIT( cmdedit_delete )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *tch;
	int i,j,iCmd;
	struct cmd_type *new_table;

	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : delete [nombre-del-comando-a-borrar]\n\r", ch );
		return FALSE;
	}

	iCmd = cmd_lookup(argument);

	if (iCmd == -1)
	{
		send_to_char( "CMDEdit : ese comando no existe.\n\r", ch );
		return FALSE;
	}

	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->connected != CON_PLAYING || (tch = CH(d)) == NULL || tch->desc == NULL )
			continue;

		if ( tch->desc->editor == ED_CMD )
		  	edit_done(tch);
	}

	new_table = malloc (sizeof(struct cmd_type) * MAX_CMD);

	if (!new_table)
	{
		send_to_char ("Memory allocation failed. Brace for impact...\n\r",ch);
		return FALSE;
	}

	for (i = 0, j = 0; i < MAX_CMD+1; i++)
		if (i != iCmd) /* copy, increase only if copied */
		{
			new_table[j] = cmd_table[i];
			j++;
		}

	free (cmd_table);
	cmd_table = new_table;

	MAX_CMD--; /* Important :() */

	crear_tabla_com();
	send_to_char ("Ahora ese comando es historia.\n\r",ch);
	return TRUE;
}

CMDEDIT( cmdedit_funcion )
{
	struct cmd_type *pCmd;
	DO_FUN *funcion;
	
	EDIT_CMD(ch,pCmd);
	
	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : funcion [nombre-funcion]\n\r", ch );
		return FALSE;
	}
	
	if ( (funcion = cmd_func_lookup(argument)) == NULL )
	{
		send_to_char( "CMDEdit : Funcion inexistente.\n\r", ch );
		return FALSE;
	}
	
	pCmd->do_fun = funcion;
	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

CMDEDIT( cmdedit_nivel )
{
	struct cmd_type *pCmd;
	int niv;
	
	EDIT_CMD(ch,pCmd);

	if ( IS_NULLSTR(argument) || !is_number(argument) )
	{
		send_to_char( "Sintaxis : nivel [0 <= nivel <= MAX_LEVEL]\n\r", ch );
		return FALSE;
	}
	
	niv = atoi(argument);
	if ( niv < 0 || niv > MAX_LEVEL )
	{
		send_to_char( "CMDEdit : Nivel fuera de rango.\n\r", ch );
		return FALSE;
	}

	if ( pCmd->level > ch->level )
	{
		send_to_char( "CMDEdit : No puedes hacer eso.\n\r", ch );
		return FALSE;
	}

	pCmd->level = niv;
	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}
#endif
