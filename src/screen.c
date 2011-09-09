#include "include.h"
#include "tables.h"
#include "screen.h"
#include "olc.h"
#include "vt.h"

char * areaname( void * point )
{
	AREA_DATA * area = *(AREA_DATA **) point;

	return area->name;
}

char * clan2str( void * point )
{
	sh_int clan = *(sh_int *) point;

	return clan_table[clan].name;
}

char * extradescr2str( void * point )
{
static	char buf[MIL];
	EXTRA_DESCR_DATA *ed = *(EXTRA_DESCR_DATA **) point;

	buf[0] = '\0';
	for ( ; ed; ed = ed->next )
	{
	    strcat( buf, ed->keyword );
	    if ( ed->next )
		strcat( buf, " " );
	}
	buf[67] = '\0'; // para lineas de 80 carac
	return buf;
}

char * progs( void * point )
{
static	char buf[MSL];
	char tmpbuf[MIL];
	int cnt;
	MPROG_LIST * list = *(MPROG_LIST **) point;

	buf[0] = '\0';
	strcat(buf, "Programas:\n\r");

	for (cnt=0; list; list=list->next)
        {
              if (cnt ==0)
                      strcat ( buf, "#UN  Vnum  Trigger Frase     #u\n\r" );

              sprintf(tmpbuf, "%2d %5d %7.7s %s\n\r", cnt,
                     list->vnum, mprog_type_to_name(list->trig_type),
                     list->trig_phrase);
              strcat( buf, tmpbuf );
              cnt++;
        }

	return buf;
}

char * sex2str( void * point )
{
	sh_int sex = *(sh_int *) point;
static	char tmpch[2];

	if ( sex == SEX_FEMALE )
		tmpch[0] = 'F';
	else
	if ( sex == SEX_MALE )
		tmpch[0] = 'M';
	else
		tmpch[0] = 'O';

	return tmpch;
}

char * damtype2str( void * point )
{
	sh_int dtype = *(sh_int *) point;

	return attack_table[dtype].name;
}

char * size2str( void * point )
{
	sh_int siz = *(sh_int *) point;

	return size_table[siz].name;
}

char * ac2str( void * point )
{
	sh_int * ac = (sh_int *) point;
static	char buf[40];

	sprintf(buf,"Pi:%d Ba:%d Sl:%d Ma:%d",
		ac[AC_PIERCE], ac[AC_BASH],
		ac[AC_SLASH], ac[AC_EXOTIC]);

	return buf;
}

char * dice2str( void * point )
{
	sh_int * dado = (sh_int *) point;
static	char buf[30];

	sprintf(buf, "%dd%d+%d", dado[DICE_NUMBER], dado[DICE_TYPE], dado[DICE_BONUS] );

	return buf;
}

char * race2str( void * point )
{
	sh_int raza = *(sh_int *) point;

	return race_table[raza].name;
}

char * pos2str( void * point )
{
	sh_int posic = *(sh_int *) point;

	return position_table[posic].short_name;
}

char * exits2str( void * point )
{
    EXIT_DATA **pexitarray = (EXIT_DATA **) point;
    EXIT_DATA *pexit;
static char buf[MSL];
    char word[MIL], reset_state[MIL], tmpbuf[MIL];
    char *state;
    int i, length, j;

    buf[0] = '\0';

    for ( j = 0; j < MAX_DIR; j++ )
    {
	    if ((pexit = pexitarray[j]) == NULL)
	    	continue;

	    sprintf( tmpbuf, "-%-5.5s hacia [%5d] ",
		capitalize(dir_name[j]),
		pexit->u1.to_room ? pexit->u1.to_room->vnum : 0 );
	    strcat( buf, tmpbuf );

	    if (pexit->key > 0)
	    {
	    	sprintf(tmpbuf, "Llave:%d ", pexit->key);
	    	strcat(buf, tmpbuf);
	    }

	    /*
	     * Format up the exit info.
	     * Capitalize all flags that are not part of the reset info.
	     */
	    strcpy( reset_state, flag_string( exit_flags, pexit->rs_flags ) );
	    state = flag_string( exit_flags, pexit->exit_info );
	    strcat( buf, "Flags: [" );
	    for (; ;)
	    {
		state = one_argument( state, word );

		if ( word[0] == '\0' )
		{
		    int end;

		    end = strlen(buf) - 1;
		    buf[end] = ']';
		    if (pexit->keyword)
		    	strcat(buf, "(K)");
		    if (pexit->description)
		    	strcat(buf, "(D)");
		    strcat( buf, "\n\r" );
		    break;
		}

		if ( str_infix( word, reset_state ) )
		{
		    length = strlen(word);
		    for (i = 0; i < length; i++)
			word[i] = UPPER(word[i]);
		}

		strcat( buf, word );
		strcat( buf, " " );
	    }

/*	    if ( pexit->keyword && pexit->keyword[0] != '\0' )
	    {
		sprintf( tmpbuf, "Kwds: [%s]\n\r", pexit->keyword );
		strcat( buf, tmpbuf );
	    }
	    if ( pexit->description && pexit->description[0] != '\0' )
	    {
		sprintf( tmpbuf, "%s", pexit->description );
		strcat( buf, tmpbuf );
	    } */
    }

    return buf;
}

char * spec2str( void * point )
{
	SPEC_FUN * spec = *(SPEC_FUN **) point;

	return spec_name(spec);
}

char * shop2str( void * point )
{
	SHOP_DATA *pShop = *(SHOP_DATA **) point;
	int iTrade;
static	char buf[MSL];
	char tmpbuf[MIL];

	sprintf( buf,
	  "Ganancia al vender : %d%%\n\r"
	  "Ganancia al comprar: %d%%\n\r"
	  "Horas : %d hasta %d\n\r",
		pShop->profit_buy, pShop->profit_sell,
		pShop->open_hour, pShop->close_hour );

	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	{
	    if ( pShop->buy_type[iTrade] != 0 )
	    {
		if ( iTrade == 0 )
		    strcat( buf, "N Tipo\n\r" );
		sprintf( tmpbuf, "%d %s\n\r", iTrade,
			flag_string( type_flags, pShop->buy_type[iTrade] ) );
		strcat(buf,tmpbuf);
	    }
	}

	return buf;
}

const struct olc_show_table_type redit_olc_show_table [] =
{
	{
		"name",	&xRoom.name, "Nombre:", OLCS_STRING,
		1, 1, 40, 1, 1, NULL
	},
	{
		"vnum",	&xRoom.vnum, "Vnum:", OLCS_SHINT,
		49, 1, 5, 1, 1, NULL
	},
	{
		"area", &xRoom.area, "Area:", OLCS_STRFUNC,
		60, 1, 15, 1, 1, areaname
	},
	{
		"heal", &xRoom.heal_rate, "Heal:", OLCS_SHINT,
		1, 2, 3, 1, 1, NULL
	},
	{
		"mana", &xRoom.mana_rate, "Mana:", OLCS_SHINT,
		10, 2, 3, 1, 1, NULL
	},
	{
		"clan", &xRoom.clan, "Clan:", OLCS_STRFUNC,
		19, 2, 10, 1, 1, clan2str
	},
	{
		"sector", &xRoom.sector_type, "Sector:", OLCS_FLAGSTR_SHINT,
		35, 2, 10, 1, 1, sector_flags
	},
	{
		"roomflags", &xRoom.room_flags, "Flags:", OLCS_FLAGSTR_INT,
		1, 3, 73, 1, 1, room_flags
	},
	{
		"extradesc", &xRoom.extra_descr, "Extra desc:", OLCS_STRFUNC,
		1, 4, 67, 1, 1, extradescr2str
	},
	{
		"desctag", NULL, "Descripcion:", OLCS_TAG,
		1, 5, -1, -1, 1, NULL
	},
	{
		"desc", &xRoom.description, "", OLCS_STRING,
		1, 6, -1, 6, 1, NULL
	},
	{
		"exitstag", NULL, "Salidas:", OLCS_TAG,
		1, 12, -1, -1, 1, NULL
	},
	{
		"exits", &xRoom.exit, "", OLCS_STRFUNC,
		1, 13, -1, -1, 1, exits2str
	},
	// pagina 2
	{
		"owner", &xRoom.owner, "Due~no:", OLCS_STRING,
		1, 1, 10, 1, 2, NULL
	},
	{
		NULL, NULL, NULL, 0, 0, 0, 0, 0
	}
};

const struct olc_show_table_type medit_olc_show_table [] =
{
	{
		"name", &xMob.player_name, "Nombre:", OLCS_STRING,
		1, 1, 31, 1, 1, NULL
	},
	{
		"area", &xMob.area, "Area:", OLCS_STRFUNC,
		40, 1, 12, 1, 1, areaname
	},
	{
		"vnum",	&xMob.vnum, "Vnum :", OLCS_SHINT,
		70, 1, 5, 1, 1, NULL
	},
	{
		"level", &xMob.level, "Nivel :", OLCS_SHINT,
		1, 2, 2, 1, 1, NULL
	},
	{
		"align", &xMob.alignment, "Align:", OLCS_SHINT,
		11, 2, 5, 1, 1, NULL
	},
	{
		"damtype", &xMob.dam_type, "Damtype:", OLCS_STRFUNC,
		37, 2, 12, 1, 1, damtype2str
	},
	{
		"sex", &xMob.sex, "Sexo   :", OLCS_STRFUNC,
		58, 2, 1, 1, 1, sex2str
	},
	{
		"group", &xMob.group, "Grupo:", OLCS_SHINT,
		70, 2, 5, 1, 1, NULL
	},
	{
		"race", &xMob.race, "Raza:", OLCS_STRFUNC,
		1, 3, 10, 1, 1, race2str
	},
	{
		"size", &xMob.size, "Tam:", OLCS_STRFUNC,
		17, 3, 5, 1, 1, size2str
	},
	{
		"material", &xMob.material, "Mat:", OLCS_STRING,
		27, 3, 10, 1, 1, NULL
	},
	{
		"wealth", &xMob.wealth, "$:", OLCS_INT,
		43, 3, 6, 1, 1, NULL
	},
	{
		"start_pos", &xMob.start_pos, "Pos ini:", OLCS_STRFUNC,
		58, 3, 4, 1, 1, pos2str
	},
	{
		"default_pos", &xMob.default_pos, "def:", OLCS_STRFUNC,
		72, 3, 4, 1, 1, pos2str
	},
	{
		"hit", &xMob.hit, "HitD:", OLCS_STRFUNC,
		1, 4, 12, 1, 1, dice2str
	},
	{
		"damage", &xMob.damage, "DamD:", OLCS_STRFUNC,
		22, 4, 12, 1, 1, dice2str
	},
	{
		"mana", &xMob.mana, "ManaD:", OLCS_STRFUNC,
		39, 4, 12, 1, 1, dice2str
	},
	{
		"hitroll", &xMob.hitroll, "Hitr:", OLCS_SHINT,
		71, 4, 2, 1, 1, NULL
	},
	{
		"spec_fun", &xMob.spec_fun, "Spec:", OLCS_STRFUNC,
		1, 5, 15, 1, 1, spec2str
	},
	{
		"ac", &xMob.ac, "AC:", OLCS_STRFUNC,
		39, 5, 38, 1, 1, ac2str
	},
	{
		"act", &xMob.act, "Act :", OLCS_FLAGSTR_INT,
		1, 6, 74, 1, 1, act_flags
	},
	{
		"aff", &xMob.affected_by, "Aff :", OLCS_FLAGSTR_INT,
		1, 7, 74, 1, 1, affect_flags
	},
	{
		"form", &xMob.form, "Form:", OLCS_FLAGSTR_INT,
		1, 9, 74, 1, 1, form_flags
	},
	{
		"parts", &xMob.parts, "Part:", OLCS_FLAGSTR_INT,
		1, 10, 74, 1, 1, part_flags
	},
	{
		"imm", &xMob.imm_flags, "Imm :", OLCS_FLAGSTR_INT,
		1, 11, 74, 1, 1, imm_flags
	},
	{
		"res", &xMob.res_flags, "Res :", OLCS_FLAGSTR_INT,
		1, 12, 74, 1, 1, res_flags
	},
	{
		"vuln", &xMob.vuln_flags, "Vuln:", OLCS_FLAGSTR_INT,
		1, 13, 74, 1, 1, vuln_flags
	},
	{
		"off", &xMob.off_flags, "Off :", OLCS_FLAGSTR_INT,
		1, 14, 74, 1, 1, off_flags
	},
	{
		"short_descr", &xMob.short_descr, "ShDe:", OLCS_STRING,
		1, 15, 74, 1, 1, NULL
	},
	{
		"long_descr", &xMob.long_descr, "LoDe:", OLCS_STRING,
		1, 16, 74, 1, 1, NULL
	},
	// pagina 2
	{
		"shop", &xMob.pShop, "", OLCS_STRFUNC,
		1, 2, 25, 4 + MAX_TRADE, 2, shop2str
	},
	{
		NULL, NULL, NULL, 0,
		0, 0, 0, 0, 0, NULL
	}
};

const struct olc_show_table_type oedit_olc_show_table [] =
{
	{
		"name", &xObj.name, "Nombre:", OLCS_STRING,
		1, 1, 32, 1, 1, NULL
	},
	{
		"area", &xObj.area, "Area:", OLCS_STRFUNC,
		40, 1, 14, 1, 1, areaname
	},
	{
		"vnum", &xObj.vnum, "Vnum :", OLCS_SHINT,
		68, 1, 5, 1, 1, NULL
	},
	{
		"type", &xObj.item_type, "Tipo  :", OLCS_FLAGSTR_SHINT,
		1, 2, 11, 1, 1, type_flags
	},
	{
		"level", &xObj.level, "Nivel:", OLCS_SHINT,
		20, 2, 2, 1, 1, NULL
	},
	{
		"condition", &xObj.condition, "Cond:", OLCS_SHINT,
		29, 2, 3, 1, 1, NULL
	},
	{
		"weight", &xObj.weight, "Peso:", OLCS_SHINT,
		56, 2, 5, 1, 1, NULL
	},
	{
		"cost",	&xObj.cost, "Costo:", OLCS_INT,
		68, 2, -1, 1, 1, NULL
	},
	{
		"wear", &xObj.wear_flags, "Wear F:", OLCS_FLAGSTR_INT,
		1, 3, 72, 1, 1, wear_flags
	},
	{
		"extra", &xObj.extra_flags, "Extra :", OLCS_FLAGSTR_INT,
		1, 4, 72, 1, 1, extra_flags
	},
	{
		"short_descr", &xObj.short_descr, "ShDesc:", OLCS_STRING,
		1, 5, 72, 1, 1, NULL
	},
	{
		"extra_descr", &xObj.extra_descr, "ExDesc:", OLCS_STRFUNC,
		1, 6, 72, 1, 1, extradescr2str
	},
	{
		"description", &xObj.description, "Desc  :", OLCS_STRING,
		1, 7, 72, 6, 1, NULL
	},
	{
		NULL, NULL, NULL, 0,
		0, 0, 0, 0, 0, NULL
	}
};

void InitScreen(DESCRIPTOR_DATA *d)
{
	char buf[MIL];
	int size;
	CHAR_DATA * ch = d->character;

	if (!IS_SET(ch->comm, COMM_OLCX))
		return;

	size = IS_NPC(ch) ? PAGELEN : (ch->lines + 3);

	send_to_char(VT_HOMECLR, ch);

	if ( d->editor != ED_NONE && IS_SET(ch->comm, COMM_OLCX) )
	{
		InitScreenMap(d);
		sprintf(buf, VT_MARGSET, size - 4, size);
		send_to_char(buf, ch);
		sprintf(buf, VT_CURSPOS "OK!\n\r", size - 4, 0);
		send_to_char(buf, ch);
	}
}

void InitScreenMap(DESCRIPTOR_DATA *d)
{
	int i;

	if (d->screenmap == NULL)
		d->screenmap = calloc(80*(d->character->lines - 3) + 1, sizeof(char));
	if (d->oldscreenmap == NULL)
		d->oldscreenmap = calloc(80*(d->character->lines - 3) + 1, sizeof(char));
	for (i = 0; i < 80*(d->character->lines - 3); i++)
		d->screenmap[i] = d->oldscreenmap[i] = ' ';
}

void UpdateOLCScreen(DESCRIPTOR_DATA *d)
{
	char buf[MSL*2], buf2[MSL*2];
	void * point;
const	struct olc_show_table_type * tabla;
	int blah, i, largo, x, y, j;
extern	ROOM_INDEX_DATA xRoom;
	STRFUNC * func;
	char * tmpstr;
const	struct flag_type * flagt;

	if (d->screenmap == NULL || d->oldscreenmap == NULL)
		InitScreenMap(d);

	switch(d->editor)
	{
		default:	return;
		case ED_ROOM:	tabla = redit_olc_show_table;
				blah = (int) &xRoom;
				break;
		case ED_MOBILE:	tabla = medit_olc_show_table;
				blah = (int) &xMob;
				break;
		case ED_OBJECT:	tabla = oedit_olc_show_table;
				blah = (int) &xObj;
				break;
	}

	write_to_buffer( d, VT_CURSAVE, 0 );

	tmpstr		= d->oldscreenmap;
	d->oldscreenmap	= d->screenmap;
	d->screenmap	= tmpstr;

	for ( i = 0; tabla[i].nombre != NULL; i++ )
	{
		point = (void *) ((int) tabla[i].point - (int) blah + (int) d->pEdit);

		if (tabla[i].pagina != d->pagina)
			continue;

		switch(tabla[i].tipo)
		{
			default:break;
			case OLCS_STRING:
			strcpy(buf, *(char **) point);
			break;

			case OLCS_INT:
			strcpy(buf, itos(*(int *)point));
			break;

			case OLCS_SHINT:
			strcpy(buf, itos(*(sh_int *)point));
			break;

			case OLCS_STRFUNC:
			func = (STRFUNC *) tabla[i].func;
			tmpstr = (*func) (point);
			strcpy(buf, tmpstr ? tmpstr : "" );
			break;

			case OLCS_FLAGSTR_INT:
			flagt = (const struct flag_type *) tabla[i].func;
			strcpy(buf, flag_string(flagt, *(int *)point));
			break;

			case OLCS_FLAGSTR_SHINT:
			flagt = (const struct flag_type *) tabla[i].func;
			strcpy(buf, flag_string(flagt, *(sh_int *) point));
			break;

			case OLCS_BOOL:
			strcpy(buf, *(bool *) point == TRUE ? "S" : "N");
			break;

			case OLCS_TAG:
			buf[0] = '\0';
			break;
		}

		strcpy(buf2, tabla[i].desc);
		strcat(buf2, buf);
		largo = strlen(buf2);
		x = tabla[i].x;
		y = tabla[i].y;
		for ( j = 0; j < largo; j++ )
		{
			if (buf2[j] == '\r')
			{
				x = tabla[i].x;
				continue;
			}
			if (buf2[j] == '\n')
			{
				y++;
				continue;
			}
			if ((tabla[i].largox < 1 && x > 79)
			||  (tabla[i].largox > 0 && x >= tabla[i].x + tabla[i].largox + strlen(tabla[i].desc)))
			{
				y++;
				x = tabla[i].x;
			}
			if ((tabla[i].largoy < 1 && y > d->character->lines-3)
			||  (tabla[i].largoy > 0 && y >= tabla[i].y + tabla[i].largoy))
				break;
			if (((y-1)*80+(x-1)) >= 80*(d->character->lines-3))
				break;
			d->screenmap[(y-1)*80+(x++-1)] = buf2[j];
		}
	}

	// mandamos solo las diferencias
	largo = strlen(d->screenmap);
	i = j = 0;
	buf[0] = '\0';
	while(i<largo)
	{
		if ( d->screenmap[i] == d->oldscreenmap[i] )
		{
			i++;
			continue;
		}
		sprintf(buf2, VT_CURSPOS "%c", i/80+1, i%80, d->screenmap[i++] );
		strcat(buf, buf2);
		j += strlen(buf2);
		while ( d->screenmap && d->screenmap[i] != d->oldscreenmap[i] )
			buf[j++] = d->screenmap[i++];
		buf[j] = '\0';
	}

	write_to_buffer( d, buf, j );
	write_to_buffer( d, VT_CURREST, 0 );
}
