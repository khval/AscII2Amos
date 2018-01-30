
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <math.h>
#include <vector>
#include <proto/exec.h>
#include <proto/amosextension.h>

#include "support_functions.h"
#include "load_interpreter_config.h"
#include "argflags.h"
#include "dynamicCommands.h"

using namespace std;

extern ULONG flags;
extern vector<DynamicCommand *> DCommands;

#define cmpID( a, b ) (*((int *) a) == *((int *) ((void *) b)))
#define Leek( adr )	*((int *) (adr))
#define Peek( adr ) *((char *) (adr))
#define Peek_str( adr, n ) strndup( adr , n )

const char *config_name = "AMOSPro_Interpreter_Config";

char *ST_str[STMX];

void process_load( char *mem )
{
	char *BNAME_str;
	char *F_str;
	char *ADAR;
	char *ADAT;
	char *STAD;
	char *A;
	int ST;
	int L;			// string length

	if (cmpID(mem,"PId1"))
	{
	         STAD=mem+Leek(mem+4)+8;
		if(cmpID(STAD,"PIt1"))
		{
			// Strings
			A=STAD+8;

			for (ST=1 ; ST<=STMX; ST++)
			{
				L=Peek(A+1) ;
				 if (L==0xFF) break;
				ST_str[ST-1]=Peek_str(A+2,L);
				A+=L+2;
			} 
		} 
	} 
}

BOOL load_config( const char *name )
{
	FILE *fd;
	size_t filesize;
	char *bank;
	BOOL ret = FALSE;

	fd = fopen(name,"r");
	if (fd)
	{	
		fseek(fd,0,SEEK_END);
		filesize = ftell(fd);
		fseek(fd,0,SEEK_SET);

		bank = (char *) malloc( filesize );

		if (bank)
		{
			if (fread( bank, filesize, 1, fd )==1)
			{
				process_load( bank );
			}

			ret = TRUE;
			free(bank);
		}

		fclose(fd);
	}
	return ret;
}

char *get_path(char *name)
{
	char *path;
	int nlen = name ? strlen(name) : 0;
	int n;

	for (n=nlen-1; n>0; n--)
	{
		if ((name[n]==':')||(name[n]=='/'))
		{
			return strndup(name,n);
		}
	}

	return NULL;
}

char *safe_addpart(char *path, char *name)
{
	if ( (path) && (name) )
	{
		int size;
		int pl = strlen(path);
		char *newname;
		BOOL has_divider = TRUE;		// if path is "" then we don't add a divider

		if (pl>0) has_divider = ((path[pl-1] == '/') || (path[pl-1] == ':')) ? TRUE : FALSE;

		size = pl + strlen(name) + 2;		// one extra for divider, one extra for \0 = two extra
		newname = (char *) malloc( size );
		if (newname)
		{
			sprintf(newname,"%s%s%s", path,has_divider ? "" : "/", name);
			return newname;
		}		
	}

	if ( (path == NULL) && (name) )	// we don't have path, so we do normal strdup().
	{
		return strdup(name);
	}

	return NULL;
}

BOOL try_config( const char *path, char *config_name)
{
	BOOL config_loaded = FALSE;

	char *config_full_name = NULL;

	config_full_name = safe_addpart( (char *) path, config_name );

	if (flags & flag_verbose) printf("try: '%s'\n", config_full_name);

	if (config_full_name)
	{
		config_loaded = load_config(config_full_name);
		free(config_full_name);
	}

	return config_loaded;
}

BOOL load_config_try_paths( const char *filename)
{
	BOOL config_loaded = FALSE;
	const char **path;
	char *_path;	// tmp path
	const char *paths[] =
		{
			"amospro:s",
			"s:",
			"progdir:",
			NULL,
		};

	_path = get_path( (char *) filename );
	if (_path)
	{
		config_loaded = try_config( _path,  (char *) config_name);
		free(_path);
	}

	path = paths;
	while ((config_loaded == FALSE)&&(*path))
	{
		config_loaded = try_config( *path,  (char *) config_name);
		path++;
	}

	return config_loaded;
}


void load_extensions( const char *filename )
{
	char buffer[1000];
	BOOL config_loaded = load_config_try_paths( filename );
	int n;

	for (n=14;n<14+extensions_max;n++)
	{
		if (ST_str[n]) if (ST_str[n][0]) 
		{
			sprintf(buffer,"AmosPro:APSystem/%s",ST_str[n]);
			extensions[n-14] = OpenExtension(buffer);

			if ((flags & flag_ShowExtensions) || (flags & flag_verbose))
			{
				printf("%02d: %s is%s loaded.\n",n -14, ST_str[n], extensions[n-14] ? "" : " NOT" );
			}
		}
	}
}

void free_extensions()
{
	int n;

	for (n=0;n<STMX;n++) { if (ST_str[n]) free(ST_str[n]); ST_str[n] = NULL;}
	for (n=0;n<extensions_max;n++) if (extensions[n]) CloseExtension(extensions[n]);
}

void init_extensions()
{
	int n;
	for (n=0;n<STMX;n++) ST_str[n]=NULL;
	for (n=0;n<extensions_max;n++) extensions[n]=NULL;
}



void extensions_to_commands()
{
	char name_str[200];
	struct ExtensionDescriptor *ed;
	int nn,n;
	DynamicCommand *_new;
	
	for (n=0;n<extensions_max-14;n++)
	{
		if (extensions[n])
		{
			for ( ed = FirstExtensionItem( extensions[n] ); ed ; ed = NextExtensionItem( ed ))
			{

				if (ed -> tokenInfo.command)
				{
					sprintf(name_str,"%s", 
						(char *) (ed -> tokenInfo.command[0] == '!' ?  ed -> tokenInfo.command + 1 : ed -> tokenInfo.command ) );

					Capitalize(name_str);

					_new = new DynamicCommand( ed -> tokenInfo.token, n,  name_str, 
							number_of_args( ed -> tokenInfo.args ),
							return_value(ed -> tokenInfo.args));

					if (_new)	DCommands.push_back(_new);
				}
			}
		}
	}
}

