#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "include/support_functions.h"

struct symbol 
{
	unsigned short token;
	const char *symbol;
};

struct symbol logical_symbol[]=
{
	{0xFF8E,">="},	// numbers 
	{0xFF7A,"<="},	// numbers
	{0xFF84,"<="},
	{0xFF98,">="},
	{0xFF66,"<>"},
	{0xFFA2,"="},
	{0xFFB6,">"},
	{0xFFAC,"<"},
	{0xFF58,"and "},
	{0xFF4C,"or "}
};

struct symbol Symbol[]=
{
	{0xFF8E,">="},	// numbers 
	{0xFF7A,"<="},	// numbers
	{0xFF84,"<="},
	{0xFF98,">="},
	{0xFF66,"<>"},
	{0x0054,":"},
	{0x005C,","},
	{0x0064,";"},
	{0x0074,"("},
	{0x007C,")"},
	{0x0084,"["},
	{0x008C,"]"},
	{0xFFC0,"+"},
	{0xFFCA,"-"},
	{0xFFA2,"="},
	{0xFFE2,"*"},
	{0xFFEC,"/"},
	{0xFFB6,">"},
	{0xFFAC,"<"},
	{0xFFF6,"^"},
	{0xFF58,"and "},
	{0xFF4C,"or "}
};

BOOL is_logical_operation( unsigned short token )
{
	int n;

	for (n=0;n<sizeof(logical_symbol)/sizeof(struct symbol );n++)
	{
		if (token == logical_symbol[n].token) return TRUE;
	}
	return FALSE;
}

char *symbolToken(char *token_buffer, const char **ptr)
{
	int n;
	int l;
	const char *name,*data;
	char c,s;

	data = *ptr;

	for (n=0;n<sizeof(Symbol)/sizeof(struct symbol );n++)
	{
		name = Symbol[n].symbol;
		l = strlen(name);
		c = l>0? name[l-1] : 0;

		if (c != ' ')
		{
			if ( strncmp( data, name, l) == 0 )
			{
				printf("[%04X] ",  Symbol[n].token);

				*ptr = (data + l);
				token_buffer = tokenWriter( token_buffer, Symbol[n].token, "" );
				return token_buffer;
			}
		}
		else
		{
			if ( strncmp( *ptr, name, l-1) == 0 )
			{
				BOOL valid = FALSE;
				s = data[l-1];

				if (s == '(')
				{
					*ptr = (data+l-1);
					valid = TRUE;
				}
				else if (s == ' ')
				{
					*ptr = (data+l);
					valid = TRUE;
				}

				if (valid)
				{
					printf("[%04X] ",  Symbol[n].token);
					token_buffer = tokenWriter( token_buffer, Symbol[n].token, "" );
					return token_buffer;
				}
			}

		}
	}
	return NULL;
}
