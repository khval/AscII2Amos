#include <stdlib.h>
#include <stdio.h>
#include <proto/exec.h>
#include "support_functions.h"
#include "special.h"
#include "string.h"
#include "argflags.h"

extern ULONG flags;

struct special Special[]= 
{
	{"Rem",cmdRem},
	{"On",cmdOn},
	{"Data",cmdData},
	{"Else If",cmdElseIf},
	{"Else",cmdElse},
	{"Exit If",cmdExitIf},
	{"Exit",cmdExit},
	{"Then",cmdThen},
	{"Equ",cmdEqu},
	{"Lvo",cmdLvo},
//	{"Struc",cmdStruc},
	{"Procedure",cmdProcedure},
//	{"Proc",cmdProc},
	{"For",cmdFor},
	{"Repeat",cmdRepeat},
	{"While",cmdWhile},
	{"Until",cmdUntil},
	{"If",cmdIf},
	{"Do",cmdDo},
	{NULL, NULL}
};

char *cmdRem( char *token_buffer, const char **ptr)
{
	unsigned short token = 0x064A;
	const char *_start;
	const char *p;
	unsigned short length = 0;
	int remLen = 0;

	_start = *ptr;

	// rem\0

	if ( strcasecmp(_start,"rem") ==0 )
	{
		*ptr = ((char *) *ptr) + 3;
		return token_buffer;		
	}

	p =_start;

	if (strncasecmp(_start,"rem ",4) == 0 ) { remLen=4; p+=remLen; }

	for ( ;*p;p++) length++;

	if (flags & flag_verbose) printf("[%04X,%04X,%s%s] ", token, length + (length&1), _start+remLen, length &1 ? ",00" : "");
	token_buffer = tokenWriter( token_buffer, token, "2,s",  length + (length&1) , _start+remLen );

	*ptr = ((char *) *ptr) + length + 1;

	return token_buffer;
}


char *cmdExit( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %08X] ", 0x029E, 0 );
	token_buffer = tokenWriter( token_buffer,0x029E,"4", 0);
	return token_buffer;
}

char *cmdFor( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %04X] ", 0x023C, 0 );
	token_buffer = tokenWriter( token_buffer,0x023C,"2",0);
	return token_buffer;
}

char *cmdRepeat( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %04X] ", 0x0250, 0 );
	token_buffer = tokenWriter( token_buffer,0x0250, "2",0);
	return token_buffer;
}

char *cmdProcedure( char *token_buffer, const char **ptr)
{
	int flags = 0;

	if (flags & flag_verbose) printf("[%04X, %04X] ", 0x0376, 0 );
	token_buffer = tokenWriter( token_buffer,0x0376,"4,2,1,1",0,0,flags,0);
	return token_buffer;
}

char *cmdWhile( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %04X] ", 0x0268, 0 );
	token_buffer = tokenWriter( token_buffer,0x0268,"2",0);
	return token_buffer;
}

char *cmdDo( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %04X] ", 0x027E, 0 );
	token_buffer = tokenWriter( token_buffer,0x027E,"2",0);
	return token_buffer;
}

char *cmdIf( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %04X] ", 0x02BE, 0 );
	token_buffer = tokenWriter( token_buffer,0x02BE,"2",0);
	return token_buffer;
}

char *cmdOn( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %08X] ", 0x0316, 0 );
	token_buffer = tokenWriter( token_buffer, 0x0316,"4",0);
	return token_buffer;
}

char *cmdElse( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %04X] ", 0x02D0, 0 );
	token_buffer = tokenWriter( token_buffer, 0x02D0,"2",0);
	return token_buffer;
}

char *cmdData( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %04X] ", 0x0404, 0 );
	token_buffer = tokenWriter( token_buffer, 0x0404,"2",0);
	return token_buffer;
}

char *cmdElseIf( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %04X] ", 0x25A4, 0 );
	token_buffer = tokenWriter( token_buffer, 0x25A4, "2",0);
	return token_buffer;
}

char *cmdExitIf( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %08X] ", 0x0290, 0 );
	token_buffer = tokenWriter( token_buffer, 0x0290, "4", 0);
	return token_buffer;
}

char *cmdEqu( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %04X, %04X, %04X] ", 0x2A40, 0, 0, 0 );
	token_buffer = tokenWriter( token_buffer, 0x2A40, "2,2,2", 0, 0, 0);
	return token_buffer;
}

char *cmdLvo( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %04X, %04X, %04X] ", 0x2A4A, 0, 0, 0 );
	token_buffer = tokenWriter( token_buffer, 0x2A4A, " 2,2,2", 0, 0, 0 );
	return token_buffer;
}

char *cmdUntil( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X, %04X] ", 0x025C, 0 );
	token_buffer = tokenWriter( token_buffer, 0x025C, "2", 0);
	return token_buffer;
}

char *cmdThen( char *token_buffer, const char **ptr)
{
	if (flags & flag_verbose) printf("[%04X] ", 0x02C6 );
	token_buffer = tokenWriter( token_buffer, 0x02C6, "");
	return token_buffer;
}

