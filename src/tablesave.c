#include "include.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include "recycle.h"
#include "interp.h"

#define SOCIAL_FILE	DATA_DIR "socials"
#define SKILL_FILE DATA_DIR "skills"

struct savetable_type
{
	char *	campo;
	sh_int	tipo_campo;
	void *	puntero_campo;
	const void *	argumento;
	const void *	argumento2;
};

struct cmd_type		cmd;
struct race_type	race;
#if defined(FIRST_BOOT)
struct pc_race_type	pcrace;
#endif
struct social_type	soc;
struct skill_type	sk;
MPROG_CODE		pcode;

char * cmd_func_name( DO_FUN * );
DO_FUN * cmd_func_lookup( char * );

char * gsn_name( sh_int * );
sh_int * gsn_lookup( char * );

char * spell_name( SPELL_FUN * );
SPELL_FUN * spell_function( char * );

typedef char * STR_FUNC ( void * );
typedef bool   STR_READ_FUNC ( void *, char * );

char * do_fun_str( void * temp )
{
	DO_FUN ** fun = (DO_FUN **) temp;

	return cmd_func_name( *fun );
}

char * position_str( void * temp )
{
	sh_int *flags = (sh_int *) temp;

	return position_table[*flags].name;
}

char * size_str( void * temp )
{
	sh_int *size = (sh_int *) temp;

	return size_table[UMAX(0, *size)].name;
}

char * race_str( void * temp )
{
	sh_int *raza = (sh_int *) temp;

	return race_table[*raza].name;
}

char * clan_str( void * temp )
{
	sh_int *klan = (sh_int *) temp;

	return clan_table[*klan].name;
}

char * class_str( void * temp )
{
	sh_int *class = (sh_int *) temp;

	return class_table[*class].name;
}

char * pgsn_str( void * temp )
{
	sh_int **pgsn = (sh_int **) temp;

	return gsn_name(*pgsn);
}

char * spell_fun_str( void * temp )
{
	SPELL_FUN ** spfun = (SPELL_FUN **) temp;

	return spell_name(*spfun);
}

bool race_read( void * temp, char * arg )
{
	sh_int * raza = (sh_int *) temp;

	*raza = race_lookup(arg);

	return (*raza == 0) ? FALSE : TRUE;
}

bool clan_read( void * temp, char * arg )
{
	sh_int * klan = (sh_int *) temp;

	*klan = clan_lookup(arg);

	return TRUE;
}

bool class_read( void * temp, char * arg )
{
	sh_int * class = (sh_int *) temp;

	*class = class_lookup(arg);

	if ( *class == -1 )
	{
		*class = 0;
		return FALSE;
	}

	return TRUE;
}

bool do_fun_read( void * temp, char * arg )
{
	DO_FUN ** fun = (DO_FUN **) temp;

	*fun = cmd_func_lookup(arg);

	return TRUE;
}

bool position_read( void * temp, char * arg )
{
	sh_int * posic = (sh_int *) temp;
	sh_int ffg = position_lookup(arg);

	*posic = UMAX(0, ffg);

	if ( ffg == -1 )
		return FALSE;
	else
		return TRUE;
}

bool size_read( void * temp, char * arg )
{
	sh_int * size = (sh_int *) temp;
	int ffg = size_lookup(arg);

	*size = UMAX(0, ffg);

	if ( ffg == -1 )
		return FALSE;
	else
		return TRUE;
}

bool pgsn_read( void * temp, char * arg )
{
	sh_int ** pgsn = (sh_int **) temp;
	sh_int * blah = gsn_lookup(arg);

	*pgsn = blah;

	return !str_cmp(arg, "") || blah != NULL;
}

bool spell_fun_read( void * temp, char * arg )
{
	SPELL_FUN ** spfun = (SPELL_FUN **) temp;
	SPELL_FUN * blah = spell_function(arg);

	*spfun = blah;

	return !str_cmp(arg, "") || blah != NULL;
}

#define CAMPO_STRING			0
#define CAMPO_FUNCION_INT_TO_STR	1
#define CAMPO_SHINT			2
#define CAMPO_FLAGSTRING		3
#define CAMPO_INT			4
#define CAMPO_FLAGVECTOR		5
#define CAMPO_BOOL			6
#define CAMPO_SHINT_ARRAY		7
#define CAMPO_STRING_ARRAY		8
#define CAMPO_FUNCION_SHINT_TO_STR	9
#define CAMPO_SHINT_FLAGSTRING		10
#define CAMPO_BOOL_ARRAY		11
#define CAMPO_INUTIL			12

const struct savetable_type progcodesavetable [] =
{
	{	"vnum",		CAMPO_SHINT,			(void *) &pcode.vnum,	NULL,	NULL	},
	{	"code",		CAMPO_STRING,			(void *) &pcode.code,	NULL,	NULL	},
	{	NULL,		0,				NULL,			NULL,	NULL	}
};

const struct savetable_type skillsavetable [] =
{
	{	"name",		CAMPO_STRING,			(void *) &sk.name,		NULL,			NULL	},
	{	"skill_level",	CAMPO_SHINT_ARRAY,		(void *) &sk.skill_level,	(void *) MAX_CLASS,	NULL	},
	{	"rating",	CAMPO_SHINT_ARRAY,		(void *) &sk.rating,		(void *) MAX_CLASS,	NULL	},
	{	"spell_fun",	CAMPO_FUNCION_INT_TO_STR,	(void *) &sk.spell_fun,		spell_fun_str,		spell_fun_read	},
	{	"target",	CAMPO_SHINT_FLAGSTRING,		(void *) &sk.target,		target_table,		NULL	},
	{	"minimum_position", CAMPO_FUNCION_SHINT_TO_STR,	(void *) &sk.minimum_position,	position_str,		position_read	},
	{	"pgsn",		CAMPO_FUNCION_INT_TO_STR,	(void *) &sk.pgsn,		pgsn_str,		pgsn_read	},
	{	"slot",		CAMPO_SHINT,			(void *) &sk.slot,		NULL,			NULL	},
	{	"min_mana",	CAMPO_SHINT,			(void *) &sk.min_mana,		NULL,			NULL	},
	{	"beats",	CAMPO_SHINT,			(void *) &sk.beats,		NULL,			NULL	},
	{	"noun_damage",	CAMPO_STRING,			(void *) &sk.noun_damage,	NULL,			NULL	},
	{	"msg_off",	CAMPO_STRING,			(void *) &sk.msg_off,		NULL,			NULL	},
	{	"msg_obj",	CAMPO_STRING,			(void *) &sk.msg_obj,		NULL,			NULL	},
	{	NULL,		0,				NULL,				NULL,			NULL	}
};

const struct savetable_type socialsavetable [] =
{
	{	"name",		CAMPO_STRING,	(void *) &soc.name,		NULL,	NULL	},
	{	"char_no_arg",	CAMPO_STRING,	(void *) &soc.char_no_arg, 	NULL,	NULL	},
	{	"others_no_arg",CAMPO_STRING,	(void *) &soc.others_no_arg,	NULL,	NULL	},
	{	"char_found",	CAMPO_STRING,	(void *) &soc.char_found,	NULL,	NULL	},
	{	"others_found",	CAMPO_STRING,	(void *) &soc.others_found,	NULL,	NULL	},
	{	"vict_found",	CAMPO_STRING,	(void *) &soc.vict_found,	NULL,	NULL	},
	{	"char_auto",	CAMPO_STRING,	(void *) &soc.char_auto,	NULL,	NULL	},
	{	"others_auto",	CAMPO_STRING,	(void *) &soc.others_auto,	NULL,	NULL	},
	{	NULL,		0,		NULL,				NULL,	NULL	}
};

const struct savetable_type racesavetable [] =
{
	{	"name",		CAMPO_STRING,			(void *) &race.name,	NULL,		NULL		},
	{	"pc",		CAMPO_BOOL,			(void *) &race.pc_race,	NULL,		NULL		},
	{	"act",		CAMPO_FLAGVECTOR,		(void *) &race.act,	NULL,		NULL		},
	{	"aff",		CAMPO_FLAGVECTOR,		(void *) &race.aff,	NULL,		NULL		},
	{	"off",		CAMPO_FLAGVECTOR,		(void *) &race.off,	NULL,		NULL		},
	{	"imm",		CAMPO_FLAGVECTOR,		(void *) &race.imm,	NULL,		NULL		},
	{	"res",		CAMPO_FLAGVECTOR,		(void *) &race.res,	NULL,		NULL		},
	{	"vuln",		CAMPO_FLAGVECTOR,		(void *) &race.vuln,	NULL,		NULL		},
	{	"form",		CAMPO_FLAGVECTOR,		(void *) &race.form,	NULL,		NULL		},
	{	"parts",	CAMPO_FLAGVECTOR,		(void *) &race.parts,	NULL,		NULL		},
#if !defined(FIRST_BOOT)
	{	"points",	CAMPO_SHINT,		(void *) &race.points,		NULL,			NULL	},
	{	"class_mult",	CAMPO_SHINT_ARRAY,	(void *) &race.class_mult,	(void *) MAX_CLASS,	NULL	},
	{	"who_name",	CAMPO_STRING,		(void *) &race.who_name,	NULL,			NULL	},
	{	"skills",	CAMPO_STRING_ARRAY,	(void *) &race.skills,		(void *) 5,		NULL	},
	{	"stats",	CAMPO_SHINT_ARRAY,	(void *) &race.stats,		(void *) MAX_STATS,	NULL	},
	{	"max_stats",	CAMPO_SHINT_ARRAY,	(void *) &race.max_stats,	(void *) MAX_STATS,	NULL	},
	{	"size",		CAMPO_FUNCION_SHINT_TO_STR,(void *) &race.size,		size_str,		size_read},
#endif
	{	NULL,		0,				NULL,			NULL,		NULL		}
};

#if defined(FIRST_BOOT)
const struct savetable_type pcracesavetable [] =
{
	{	"points",	CAMPO_SHINT,		(void *) &pcrace.points,	NULL,			NULL	},
	{	"class_mult",	CAMPO_SHINT_ARRAY,	(void *) &pcrace.class_mult,	(void *) MAX_CLASS,	NULL	},
	{	"who_name",	CAMPO_STRING,		(void *) &pcrace.who_name,	NULL,			NULL	},
	{	"skills",	CAMPO_STRING_ARRAY,	(void *) &pcrace.skills,	(void *) 5,		NULL	},
	{	"stats",	CAMPO_SHINT_ARRAY,	(void *) &pcrace.stats,		(void *) MAX_STATS,	NULL	},
	{	"max_stats",	CAMPO_SHINT_ARRAY,	(void *) &pcrace.max_stats,	(void *) MAX_STATS,	NULL	},
	{	"size",		CAMPO_FUNCION_SHINT_TO_STR,(void *) &pcrace.size,	size_str,		size_read},
	{	NULL,		0,			NULL,				NULL,			NULL	}
};
#endif

const struct savetable_type cmdsavetable [] =
{
	{	"name",		CAMPO_STRING,			(void *) &cmd.name,	NULL,		NULL		},
	{	"do_fun",	CAMPO_FUNCION_INT_TO_STR,	(void *) &cmd.do_fun,	do_fun_str,	do_fun_read	},
	{	"position",	CAMPO_FUNCION_SHINT_TO_STR,	(void *) &cmd.position,	position_str,	position_read	},
	{	"level",	CAMPO_SHINT,			(void *) &cmd.level,	NULL,		NULL		},
	{	"log",		CAMPO_SHINT_FLAGSTRING,		(void *) &cmd.log,	log_flags,	NULL		},
	{	"show",		CAMPO_SHINT_FLAGSTRING,		(void *) &cmd.show,	show_flags,	NULL		},
	{	NULL,		0,				NULL,			NULL,		NULL		}
};

void load_struct( FILE *fp, void * tipobase, const struct savetable_type * tabla, void * puntero )
{
	char * word;
	const struct savetable_type * temp;
	sh_int * pshint;
	int * pentero;
	char ** pcadena;
	char * cadena;
	int * pint;
	STR_READ_FUNC * funcion;
	struct flag_type * flagtable;
	bool found = FALSE;
	bool * pbool;
	int cnt = 0, i;

	while ( str_cmp((word = fread_word(fp)), "#END") )
	{
		for ( temp = tabla; !IS_NULLSTR(temp->campo); temp++ )
		{
			if ( !str_cmp( word, temp->campo ) )
			{
				// lo encontramos!
				switch(temp->tipo_campo)
				{
					case CAMPO_STRING:
					pcadena = (char **) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					*pcadena = fread_string(fp);
					found = TRUE, cnt++;
					break;

					case CAMPO_SHINT:
					pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					*pshint = (sh_int) fread_number(fp);
					found = TRUE, cnt++;
					break;

					case CAMPO_INT:
					pint = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					*pint = fread_number(fp);
					found = TRUE, cnt++;
					break;

					case CAMPO_FUNCION_INT_TO_STR:
					funcion = temp->argumento2;
					cadena = fread_string(fp);
					pint = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					if ((*funcion) (pint, cadena) == FALSE)
						bugf( "load_struct : campo %s invalido, cadena %s",
							temp->campo, cadena );
					free_string(cadena);
					found = TRUE, cnt++;
					break;

					case CAMPO_FUNCION_SHINT_TO_STR:
					funcion = temp->argumento2;
					cadena = fread_string(fp);
					pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					if ((*funcion) (pshint, cadena) == FALSE)
						bugf( "load_struct : campo %s invalido, cadena %s",
							temp->campo, cadena );
					free_string(cadena);
					found = TRUE, cnt++;
					break;

					case CAMPO_FLAGSTRING:
					flagtable = (struct flag_type *) temp->argumento;
					pentero = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					cadena = fread_string(fp);
					if ( (*pentero = flag_value(flagtable, cadena)) == NO_FLAG )
						*pentero = 0;
					free_string(cadena);
					found = TRUE, cnt++;
					break;

					case CAMPO_SHINT_FLAGSTRING:
					flagtable = (struct flag_type *) temp->argumento;
					pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					cadena = fread_string(fp);
					if ( (*pshint = flag_value(flagtable, cadena)) == NO_FLAG )
						*pshint = 0;
					free_string(cadena);
					found = TRUE, cnt++;
					break;

					case CAMPO_FLAGVECTOR:
					pentero = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					*pentero = fread_flag(fp);
					found = TRUE, cnt++;
					break;

					case CAMPO_BOOL:
					pbool = (bool *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					cadena = fread_word(fp);
					*pbool = str_cmp( cadena, "false" ) ? TRUE : FALSE;
					found = TRUE, cnt++;
					break;

					case CAMPO_SHINT_ARRAY:
					pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					i = 0;
					while( str_cmp((cadena = fread_word(fp)), "@") )
					{
						if ( i == (int) temp->argumento )
							bugf( "load_struct : campo_shint_array %s con exceso de elementos",
								temp->campo );
						else
							pshint[i++] = (sh_int) atoi(cadena);
					}
					found = TRUE, cnt++;
					break;

					case CAMPO_STRING_ARRAY:
					pcadena = (char **) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					i = 0;
					while( str_cmp((cadena = fread_string(fp)), "@") )
					{
						if ( i == (int) temp->argumento )
							bugf( "load_struct : campo_string_array %s con exceso de elementos",
								temp->campo);
						else
							pcadena[i++] = cadena;
					}
					found = TRUE, cnt++;
					break;

					case CAMPO_INUTIL:
					fread_to_eol(fp);
					found = TRUE, cnt++;
					break;

					case CAMPO_BOOL_ARRAY:
					pbool = (bool *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
					i = 0;
					while( str_cmp((cadena = fread_word(fp)), "@") )
					{
						if ( (temp->argumento != NULL
						  && i == (int) temp->argumento)
						||   (temp->argumento == NULL
						  && temp->argumento2 != NULL
						  && i == *( (int *) temp->argumento2)) )
							bugf( "load_struct : campo_bool_array %s con exceso de elementos",
								temp->campo );
						else
							pbool[i++] = (bool) atoi(cadena);
					}
					found = TRUE, cnt++;
					break;
				} // switch
				if (found == TRUE)
					break;
			} // if
		} // for

		if (found == FALSE)
		{
			bugf( "load_struct : clave %s no encontrada", word );
			fread_to_eol(fp);
		}
		else
			found = FALSE;
	} // while
}

void save_struct( FILE *fp, void * tipobase, const struct savetable_type * tabla, void * puntero )
{
	const struct savetable_type * temp;
	char ** pcadena;
	sh_int * pshint;
	STR_FUNC * funcion;
	char * cadena;
	int * pentero;
	bool * pbool;
	const struct flag_type * flagtable;
	int cnt = 0, i;

	for ( temp = tabla; !IS_NULLSTR(temp->campo); temp++ )
	{
		switch(temp->tipo_campo)
		{
			default:
			bugf( "save_struct : tipo_campo %d invalido, campo %s",
				temp->tipo_campo, temp->campo );
			break;

			case CAMPO_STRING:
			pcadena = (char **) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %s~\n", temp->campo, !IS_NULLSTR(*pcadena) ? *pcadena : "" );
			break;

			case CAMPO_SHINT:
			pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %d\n", temp->campo, *pshint );
			break;

			case CAMPO_INT:
			pentero = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %d\n", temp->campo, *pentero );
			break;

			case CAMPO_FUNCION_INT_TO_STR:
			funcion = temp->argumento;
			pentero = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			cadena = (*funcion) ((void *) pentero);
			fprintf( fp, "%s %s~\n", temp->campo, cadena );
			break;

			case CAMPO_FUNCION_SHINT_TO_STR:
			funcion = temp->argumento;
			pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			cadena = (*funcion) ((void *) pshint);
			fprintf( fp, "%s %s~\n", temp->campo, cadena );
			break;

			case CAMPO_FLAGSTRING:
			flagtable = (struct flag_type *) temp->argumento;
			pentero = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %s~\n", temp->campo, flag_string(flagtable, *pentero) );
			break;

			case CAMPO_SHINT_FLAGSTRING:
			flagtable = (struct flag_type *) temp->argumento;
			pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %s~\n", temp->campo, flag_string(flagtable, *pshint) );
			break;

			case CAMPO_FLAGVECTOR:
			pentero = (int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %s\n", temp->campo, print_flags(*pentero) );
			break;

			case CAMPO_BOOL:
			pbool = (bool *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s %s\n", temp->campo,
						(*pbool == TRUE) ? "true" : "false" );
			break;

			case CAMPO_SHINT_ARRAY:
			pshint = (sh_int *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s ", temp->campo );
			for ( i = 0; i < (int) temp->argumento; i++ )
				fprintf( fp, "%d ", pshint[i] );
			fprintf( fp, "@\n" );
			break;

			case CAMPO_STRING_ARRAY:
			pcadena = (char **) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s ", temp->campo );
			for ( i = 0; i < (int) temp->argumento; i++ )
				fprintf( fp, "%s~ ", !IS_NULLSTR(pcadena[i]) ? pcadena[i] : "" );
			fprintf( fp, "@~\n" );
			break;

			case CAMPO_BOOL_ARRAY:
			pbool = (bool *) ((int) temp->puntero_campo - (int) tipobase + (int) puntero);
			fprintf( fp, "%s ", temp->campo );
			for ( i = 0; i < (temp->argumento ? (int) temp->argumento : *(int *) temp->argumento2); i++ )
				fprintf( fp, "%d ", pbool[i] == TRUE ? 1 : 0 );
			fprintf( fp, "@\n" );
			break;

			case CAMPO_INUTIL:
			break;
		}

		cnt++;
	}
};

void grabar_tabla_comandos( void )
{
	FILE * fp;
	struct cmd_type * temp;
#if !defined(FIRST_BOOT)
extern	struct cmd_type * cmd_table;
#endif
	int cnt = 0;

	fp = fopen( DATA_DIR "comandos", "w" );

	if ( !fp )
	{
		perror( "grabar_tabla_comandos" );
		return;
	}

	for ( temp = cmd_table; !IS_NULLSTR(temp->name); temp = temp++ )
		cnt++;

	fprintf( fp, "%d\n\n", cnt );

	for ( temp = cmd_table; !IS_NULLSTR(temp->name); temp = temp++ )
	{
		fprintf( fp, "#COMANDO\n" );
		save_struct( fp, &cmd, cmdsavetable, temp );
		fprintf( fp, "#END\n\n" );
	}

	fclose(fp);
}

#if !defined(FIRST_BOOT)
void cargar_comandos( void )
{
	FILE * fp;
extern	struct cmd_type *	cmd_table;
extern	int			MAX_CMD;
static	struct cmd_type		emptycmd;
	int i = 0, largo;
	char * word;

	fp = fopen( DATA_DIR "comandos", "r" );

	if ( fp == NULL )
	{
		perror( "load_tabla_comandos ");
		return;
	}

	largo = fread_number(fp);

	MAX_CMD = largo;

	flog( "Creando cmd_table de largo %d, tama~no %d", largo + 1,
		sizeof(struct cmd_type) * (largo + 1) );
	cmd_table = calloc( sizeof(struct cmd_type), largo + 1);

	for ( i = 0; i <= largo; i++ )
		cmd_table[i] = emptycmd; // limpiar

	i = 0;

	while(TRUE)
	{
		word = fread_word(fp);

		if ( str_cmp(word, "#COMANDO") )
		{
			bugf( "load_tabla_comandos : word %s", word );
			fclose(fp);
			return;
		}

		load_struct( fp, &cmd, cmdsavetable, &cmd_table[i++] );

		if ( i == largo )
		{
			flog( "Tabla de comandos cargada." );
			fclose(fp);
			cmd_table[i].name = str_dup( "" );
			return;
		}
	}
}

void cargar_razas( void )
{
	FILE * fp;
	extern int maxrace;
static	struct race_type cRace;
	char * word;
	int i;

	fp = fopen( DATA_DIR "razas", "r" );

	if ( !fp )
	{
		perror( "cargar_tabla_razas" );
		return;
	}

	maxrace = fread_number(fp);

	flog( "Creando race_table de largo %d, tama~no %d", maxrace + 1,
		sizeof(struct race_type) * (maxrace + 1) );

	race_table = calloc( sizeof(struct race_type), maxrace + 1 );

	// limpiar razas
	for ( i = 0; i <= maxrace; i++ )
	{
		race_table[i] = cRace;
		race_table[i].race_id = i;
	}

	i = 0;

	while(TRUE)
	{
		word = fread_word(fp);

		if ( str_cmp(word, "#RAZA") )
		{
			bugf( "cargar_tabla_razas : word %s", word );
			fclose(fp);
			return;
		}

		load_struct( fp, &race, racesavetable, &race_table[i++] );

		if ( i == maxrace )
		{
			flog( "Tabla de razas cargada." );
			fclose(fp);
			race_table[i].name = NULL;
			return;
		}
	}
}

void cargar_socials (void)
{
	FILE *fp;
	int i;
	extern int maxSocial;
	char * clave;
	
	fp = fopen (SOCIAL_FILE, "r");

	if (!fp)
	{
		perror(SOCIAL_FILE);
		exit(1);
	}

	fscanf (fp, "%d\n", &maxSocial);

	flog( "Creando social_table de largo %d, tama~no %d", maxSocial + 1,
		sizeof(struct social_type) * (maxSocial + 1) );
	/* IMPORTANT to use malloc so we can realloc later on */
	social_table = malloc (sizeof(struct social_type) * (maxSocial+1));
	
	for (i = 0; i < maxSocial; i++)
	{
		if ( str_cmp((clave = fread_word(fp)), "#SOCIAL") )
		{
			bugf( "cargar_socials : clave %s inexistente",
				clave );
			exit(1);
		}
		load_struct (fp, &soc, socialsavetable, &social_table[i]);
	}

	/* For backwards compatibility */
	social_table[maxSocial].name = str_dup(""); /* empty! */		

	fclose (fp);

	flog( "Tabla de socials cargada." );
}

void	cargar_skills( void )
{
	FILE *fp;
static	struct skill_type skzero;
	int i = 0;
	char *word;

	fp = fopen( SKILL_FILE, "r" );

	if ( !fp )
	{
		bug( "No se pudo leer " SKILL_FILE " para cargar skills.", 0 );
		exit(1);
	}

	fscanf( fp, "%d\n", &MAX_SKILL );

	flog( "Creando tabla de skills de largo %d, tama~no %d",
		MAX_SKILL + 1, sizeof(struct skill_type) * (MAX_SKILL + 1) );
	skill_table = malloc( sizeof(struct skill_type) * (MAX_SKILL + 1) );

	if ( !skill_table )
	{
		bug( "Error! Skill_table == NULL, MAX_SKILL : %d", MAX_SKILL );
		exit(1);
	}

	for ( ; ; )
	{
		word = fread_word( fp );

		if ( !str_cmp( word, "#!" ) )
			break;

		if ( str_cmp( word, "#SKILL" ) )
		{
			bugf( "Cargar_skills : clave inexistente (%s)", word );
			exit(1);
		}

		if ( i >= MAX_SKILL )
		{
			bug( "Cargar_skills : numero de skills mayor que MAX_SKILL", 0 );
			exit(1);
		}

		skill_table[i] = skzero;
		load_struct( fp, &sk, skillsavetable, &skill_table[i++] );
	}

	skill_table[MAX_SKILL].name = NULL;

	fclose(fp);
}
#endif

void grabar_razas( void )
{
	FILE * fp;
	struct race_type * temp;
#if !defined(FIRST_BOOT)
extern	struct race_type * race_table;
#endif
	int cnt = 0;

	fp = fopen( DATA_DIR "temprazas", "w" );

	if ( !fp )
	{
		perror( "grabar_razas : fopen" );
		return;
	}

	for ( temp = race_table; !IS_NULLSTR(temp->name); temp = temp++ )
		cnt++;

	fprintf( fp, "%d\n\n", cnt );

	for ( temp = race_table, cnt = 0; !IS_NULLSTR(temp->name); temp = temp++ )
	{
		fprintf( fp, "#RAZA\n" );
		save_struct( fp, &race, racesavetable, temp );
#if defined(FIRST_BOOT)
		if (cnt < MAX_PC_RACE)
			save_struct( fp, &pcrace, pcracesavetable, &pc_race_table[cnt]);
		cnt++;
#endif
		fprintf( fp, "#END\n\n" );
	}

	fclose(fp);

	if (rename(DATA_DIR "temprazas", DATA_DIR "razas") == -1)
		perror("grabar_razas : rename");
}

void grabar_socials(void)
{
	FILE *fp;
	int i;
	extern int maxSocial;
	
	fp = fopen (SOCIAL_FILE, "w");
	
	if (!fp)
	{
		perror( SOCIAL_FILE );
		return;
	}

	fprintf (fp, "%d\n", maxSocial);

	for ( i = 0 ; i < maxSocial ; i++)
	{
		fprintf( fp, "#SOCIAL\n" );
		save_struct( fp, &soc, socialsavetable, &social_table[i] );
		fprintf( fp, "#END\n\n" );
	}

	fclose (fp);
}

void grabar_skills( void )
{
	FILE *fpn;
	int i;

	fpn = fopen( SKILL_FILE, "w" );

	if ( !fpn )
	{
		bug( "Save_skills : NULL fpn", 0 );
		fclose( fpn );
		return;
	}

	fprintf( fpn, "%d\n", MAX_SKILL );

	for ( i = 0; i < MAX_SKILL; ++i )
	{
		fprintf( fpn, "#SKILL\n" );
		save_struct( fpn, &sk, skillsavetable, &skill_table[i] );
		fprintf( fpn, "#END\n\n" );
	}

	fprintf( fpn, "#!\n" );

	fclose( fpn );
}

void grabar_progs( int minvnum, int maxvnum )
{
	FILE * fp;
	MPROG_CODE * pMprog;
	char buf[64];

	for ( pMprog = mprog_list; pMprog; pMprog = pMprog->next )
		if ( pMprog->changed == TRUE
		&&   ENTRE_I(minvnum, pMprog->vnum, maxvnum) )
		{
			sprintf(buf, PROG_DIR "%d.prg", pMprog->vnum );
			fp = fopen( buf, "w" );

			if ( !fp )
			{
				perror(buf);
				return;
			}

			fprintf( fp, "#PROG\n" );
			save_struct( fp, &pcode, progcodesavetable, pMprog );
			fprintf( fp, "#END\n\n" );
			fclose(fp);

			pMprog->changed = FALSE;
		}
}

void cargar_prog( FILE * fp, MPROG_CODE ** prog )
{
extern	MPROG_CODE * mprog_list;
static	MPROG_CODE mprog_zero;
	char * word = fread_word(fp);

	if (str_cmp(word, "#PROG"))
	{
		bugf("cargar_prog : clave %s invalida", word);
		*prog = NULL;
		return;
	}

	*prog = alloc_perm(sizeof(MPROG_CODE));

	// blanquear
	**prog = mprog_zero;

	load_struct( fp, &pcode, progcodesavetable, *prog );

	// a la lista
	if (mprog_list == NULL)
		mprog_list = *prog;
	else
	{
		// al comienzo o al final? VNUM decide
		if ((*prog)->vnum < mprog_list->vnum)
		{
			(*prog)->next	= mprog_list;
			mprog_list	= *prog;
		}
		else
		{
			MPROG_CODE * temp, * prev = mprog_list;

			for ( temp = mprog_list->next; temp; temp = temp->next )
			{
				if ( temp->vnum > (*prog)->vnum )
					break;
				prev = temp;
			}
			prev->next = *prog;
			(*prog)->next = temp;
		}
	}
}

MPROG_CODE * pedir_prog( int vnum )
{
	FILE * fp;
	MPROG_CODE * prog;
	char buf[128];
	extern bool fBootDb;

	prog = get_mprog_index(vnum);

	if (prog != NULL)
		return prog;

	sprintf(buf, PROG_DIR "%d.prg", vnum );

	fp = fopen(buf,"r");

	if ( !fp )
	{
		if ( fBootDb == TRUE )
			perror("pedir_prog");
		return NULL;
	}

	cargar_prog(fp, &prog);

	fclose(fp);

	return prog;
}

