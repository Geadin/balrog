#include "include.h"
#include "olc.h"

#if !defined(FIRST_BOOT)
int MAX_GROUP;
struct group_type *group_table;
#endif
#define GROUP_FILE	DATA_DIR "grupos"
#define GEDIT( fun )	bool fun( CHAR_DATA *ch, char *argument )

#if !defined(FIRST_BOOT)
void load_group( FILE *fp, struct group_type *group )
{
	int i;
	char *temp;

	group->name	= fread_string(fp);

	for ( i = 0; i < MAX_CLASS; ++i )
		group->rating[i]	= fread_number( fp );

	i = 0;

	while(TRUE)
	{
		temp = fread_string(fp);
		if ( !str_cmp( temp, "End" ) || i >= MAX_IN_GROUP )
      {
         while( i < MAX_IN_GROUP )
                group->spells[i++] = str_dup( "" );
			break;
      }
	   else
			group->spells[i++]	= temp;
	}
}

void	load_groups( void )
{
	FILE *fp;
	int i;

	fp = fopen( GROUP_FILE, "r" );

	if ( !fp )
	{
		bug( "No se pudo leer " GROUP_FILE " para cargar grupos.", 0 );
		exit(1);
	}

	fscanf( fp, "%d\n", &MAX_GROUP );

	group_table = malloc( sizeof(struct group_type) * (MAX_GROUP + 1) );

	if ( !group_table )
	{
		bug( "Error! Group_table == NULL, MAX_GROUP : %d", MAX_GROUP );
		exit(1);
	}

	for ( i = 0; i < MAX_GROUP; ++i )
		load_group (fp, &group_table[i] );

	group_table[MAX_GROUP].name = NULL;

	fclose(fp);
}
#endif

void save_group( FILE *fp, struct group_type *group )
{
	int i;

	fprintf( fp, "%s~\n", CHECKNULLSTR(group->name) );

	for ( i = 0; i < MAX_CLASS; ++i )
		fprintf( fp, "%d ", group->rating[i] );
	fprintf( fp, "\n" );
	
	for ( i = 0; i < MAX_IN_GROUP && !IS_NULLSTR(group->spells[i]); ++i )
		fprintf( fp, "%s~\n", CHECKNULLSTR(group->spells[i]) );

	fprintf( fp, "End~\n\n" );
}

void save_groups( void )
{
	int i;
	FILE *fp;
	char buf[MIL];
	
	fclose( fpReserve );

	sprintf( buf, "%s/grupos", DATA_DIR );
	fp = fopen( buf, "w" );

	if ( !fp )
	{
		bug( "save_groups : fp null al grabar razas", 0 );
		fpReserve = fopen( NULL_FILE, "r" );
		return;
	}

	fprintf( fp, "%d\n", MAX_GROUP );

	for ( i = 0; i < MAX_GROUP; ++i )
		save_group( fp, &group_table[i] );

	fclose( fp );

	fpReserve = fopen( NULL_FILE, "r" );
}

const struct olc_cmd_type gedit_table[] =
{
#if !defined(FIRST_BOOT)
	{	"name",		gedit_name	},
	{	"rating",	gedit_rating	},
	{	"spell",	gedit_spell	},
	{	"list",		gedit_list	},
#endif
	{	"commands",	show_commands	},
	{	"show",		gedit_show	},
	{	NULL,		0		}
};

void gedit( CHAR_DATA *ch, char *argument)
{
    struct group_type *pGroup;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;

    strcpy(arg, argument);
    argument = one_argument( argument, command);

    EDIT_GROUP(ch, pGroup);
    if (ch->pcdata->security < 5)
    {
        send_to_char("SKEdit: Insuficiente seguridad para modificar grupo.\n\r",ch);
        edit_done(ch);
        return;
    }

    if (command[0] == '\0')
    {
        gedit_show(ch, argument);
        return;
    }

    if (!str_cmp(command, "done") )
    {
        edit_done(ch);
        return;
    }

    if (!str_cmp(command, "save") )
    {
    	save_groups();
    	return;
    }

    for (cmd = 0; gedit_table[cmd].name != NULL; cmd++)
    {
        if (!str_prefix(command, gedit_table[cmd].name) )
        {
           if ((*gedit_table[cmd].olc_fun) (ch, argument))
           	save_groups();
           return;
        }
    }

    interpret(ch, arg);
    return;
}

void do_gedit(CHAR_DATA *ch, char *argument)
{
    struct group_type *pGroup;
    char command[MSL];
    int group;

    if ( IS_NPC(ch) )
    	return;

    if ( IS_NULLSTR(argument) )
    {
    	send_to_char( "Sintaxis : GEdit [grupo]\n\r", ch );
    	return;
    }

    if (ch->pcdata->security < 5)
    {
    	send_to_char( "GEdit : Insuficiente seguridad para editar grupos.\n\r", ch );
    	return;
    }

    argument	= one_argument( argument, command );

/*  if ( !str_cmp( command, "new" ) )
    {
	if ( gedit_new(ch, argument) )
		save_groups();
	return;
    } */

    if ( (group = group_lookup(command)) == -1 )
    {
    	send_to_char( "GEdit : Grupo no existe.\n\r", ch );
    	return;
    }

    pGroup = &group_table[group];

    ch->desc->pEdit=(void *)pGroup;
    ch->desc->editor= ED_GROUP;

    return;
}

GEDIT( gedit_show )
{
	struct group_type *pGrp;
	char buf[MIL], buf2[MIL];
	int i;
	
	EDIT_GROUP(ch,pGrp);
	
	sprintf(buf, "Nombre    : [%s]\n\r", pGrp->name );
	send_to_char (buf, ch );

	sprintf( buf, "Clase    + " );

	for ( i = 0; i < MAX_CLASS; ++i )
	{
		strcat( buf, class_table[i].who_name );
		strcat( buf, " " );
	}

	strcat( buf, "\n\r" );
	send_to_char( buf, ch );

	sprintf( buf, "Rating   | " );

	for ( i = 0; i < MAX_CLASS; ++i )
	{
		sprintf( buf2, "%3d ", pGrp->rating[i] );
		strcat( buf, buf2 );
	}

	strcat( buf, "\n\r" );
	send_to_char( buf, ch );

	i = 0;

	while ( i < MAX_IN_GROUP && !IS_NULLSTR(pGrp->spells[i]) )
	{
		sprintf( buf, "%2d. #B%s#b\n\r", i, pGrp->spells[i] );
		send_to_char( buf, ch );
		i++;
	}
	
	return FALSE;
}

#if !defined(FIRST_BOOT)
GEDIT( gedit_name )
{
	struct group_type *pGrp;
	
	EDIT_GROUP(ch, pGrp);
	
	if (IS_NULLSTR(argument))
	{
		send_to_char( "Sintaxis : name [nombre-grupo]\n\r", ch );
		return FALSE;
	}
	
	if ( group_lookup(argument) != -1 )
	{
		send_to_char( "GEdit : Ese grupo ya existe.\n\r", ch );
		return FALSE;
	}
	
	free_string(pGrp->name);
	pGrp->name = str_dup(argument);
	
	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

GEDIT( gedit_rating )
{
	struct group_type *pGrp;
	char arg[MIL];
	int rating, clase;

	EDIT_GROUP(ch,pGrp);

	argument = one_argument( argument, arg );

	if ( IS_NULLSTR(argument) || IS_NULLSTR(arg) || !is_number(argument) )
	{
		send_to_char( "Sintaxis : rating [clase] [costo]\n\r", ch );
		return FALSE;
	}

	if ( (clase = class_lookup(arg)) == -1 )
	{
		send_to_char( "GEdit : Clase inexistente.\n\r", ch );
		return FALSE;
	}

	rating = atoi(argument);

	if ( rating < -1 )
	{
		send_to_char( "GEdit : Rating invalido.\n\r", ch );
		return FALSE;
	}

	pGrp->rating[clase] = rating;
	send_to_char( "Ok.\n\r", ch );
	return TRUE;
}

GEDIT( gedit_spell )
{
	struct group_type *pGrp;
	char arg[MSL];
	int i = 0, j = 0;
	
	EDIT_GROUP(ch,pGrp);
	
	if ( IS_NULLSTR(argument) )
	{
		send_to_char( "Sintaxis : spell new [nombre]\n\r", ch );
		send_to_char( "           spell delete [nombre]\n\r", ch );
		send_to_char( "           spell delete [numero]\n\r", ch );
		return FALSE;
	}
	
	argument = one_argument( argument, arg );

	if ( !str_cmp(arg, "new") )
	{
		for ( i = 0; !IS_NULLSTR(pGrp->spells[i]) && (i < MAX_IN_GROUP); ++i )
			;
		
		if ( i == MAX_IN_GROUP )
		{
			send_to_char( "GEdit : grupo esta lleno.\n\r", ch );
			return FALSE;
		}
		
		if ( skill_lookup(argument) == -1 && group_lookup(argument) == -1 )
		{
			send_to_char( "GEdit : skill/spell/grupo no existe.\n\r", ch );
			return FALSE;
		}
		
		free_string(pGrp->spells[i]);
		pGrp->spells[i] = str_dup(argument);
		send_to_char( "Ok.\n\r", ch );
		return TRUE;
	}
	
	if ( !str_cmp( arg, "delete" ) )
	{
		int num = is_number(argument) ? atoi(argument) : -1;

		if ( is_number(argument) && (num < 0 || num >= MAX_IN_GROUP) )
		{
			send_to_char( "GEdit : Argumento invalido.\n\r", ch );
			return FALSE;
		}
		
		while ( i < MAX_IN_GROUP && !IS_NULLSTR(pGrp->spells[i]) )
		{
			if ( i == num || !str_cmp(pGrp->spells[i], argument) )
			{
				for ( j = i; j < MAX_IN_GROUP - 1; ++j )
				{
					free_string(pGrp->spells[j]);
					pGrp->spells[j] = str_dup(pGrp->spells[j+1]);
				}
				free_string(pGrp->spells[MAX_IN_GROUP - 1]);
				pGrp->spells[MAX_IN_GROUP - 1] = str_dup( "" );
				send_to_char( "Ok.\n\r", ch );
				return TRUE;
			}
			++i;
		}
						
		send_to_char( "GEdit : Skills/spell/grupo inexistente.\n\r", ch );
		return FALSE;
	}

	gedit_spell( ch, "" );
	return FALSE;
}
#endif

GEDIT(gedit_list)
{
	struct group_type *pGrp;
	int i, cnt = 0;
	char buf[MIL];
	
	for ( i = 0; i < MAX_GROUP; ++i )
	{
		if ((pGrp = &group_table[i]) == NULL)
			break;

		sprintf( buf, "%2d. #B%20s#b ", i, group_table[i].name );

		if ( cnt++ % 2 )
			strcat( buf, "\n\r" );
		else
			strcat( buf, " - " );

		send_to_char( buf, ch );
	}

	if ( cnt % 2 )
		send_to_char( "\n\r", ch );

	return FALSE;
}
