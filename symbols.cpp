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

char *symbolToken(char *token_buffer, const char **ptr)
{
	int n;
	int l;

	for (n=0;n<sizeof(Symbol)/sizeof(struct symbol );n++)
	{
		l = strlen(Symbol[n].symbol);

		if ( strncmp( *ptr, Symbol[n].symbol, l) == 0 )
		{
			printf("[%04X] ",  Symbol[n].token);

			*ptr = (((char *) *ptr) + l);
			token_buffer = tokenWriter( token_buffer, Symbol[n].token, "" );
			return token_buffer;
		}
	}
	return NULL;
}
