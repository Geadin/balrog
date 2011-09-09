void InitScreen(DESCRIPTOR_DATA *);
void InitScreenMap(DESCRIPTOR_DATA *);
void UpdateOLCScreen(DESCRIPTOR_DATA *);

#define OLCS_STRING		1
#define OLCS_INT		2
#define OLCS_SHINT		3
#define OLCS_STRFUNC		4
#define OLCS_FLAGSTR_INT	5
#define OLCS_FLAGSTR_SHINT	6
#define OLCS_BOOL		7
#define OLCS_TAG		8

struct olc_show_table_type
{
	char *		nombre;
	void *		point;
	char *		desc;
	sh_int		tipo;
	sh_int		x;
	sh_int		y;
	sh_int		largox;
	sh_int		largoy;
	sh_int		pagina;
	const void *	func;
};

extern const struct olc_show_table_type redit_olc_show_table[];
extern const struct olc_show_table_type medit_olc_show_table[];
extern const struct olc_show_table_type oedit_olc_show_table[];

typedef char * STRFUNC ( void * );

extern STRFUNC areaname;

