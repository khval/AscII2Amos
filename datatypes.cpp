#include <stdio.h>
#include <stdlib.h>
#include <proto/exec.h>
#include "support_functions.h"
#include "datatypes.h"
#include "what_is.h"
#include "ascii2token.h"
#include "argflags.h"

extern ULONG flags;
extern BOOL is_logical_operation( unsigned short token );		// part og symbol.cpp, don't have .h file so.

char *_bin_( char *token_buffer, const char **ptr)
{
	int number = 0;
	const char *p = *ptr;

	if (*p=='%') p++;
	for (;is_break_char(*p)==FALSE; p++)
	{
		number = (number << 1 ) + (*p-'0');
	}
	*ptr = p;

	if (flags & flag_verbose) printf("[%04X,%08X] ", 0x001E, number );
	token_buffer = tokenWriter( token_buffer, 0x001E, "4",  number );

	return token_buffer;
}

char *_hex_( char *token_buffer, const char **ptr)
{
	int number = 0;
	int n=0;
	const char *p = *ptr;

	if (*p=='$') p++;

	for (;is_break_char(*p)==FALSE; p++)
	{
		if ((*p>='0')&&(*p<='9')) n = *p - '0';
		if ((*p>='a')&&(*p<='f')) n = *p -'a' + 10;
		if ((*p>='A')&&(*p<='F')) n = *p -'A' + 10;
		number = (number << 4 ) + n;
	}
	*ptr = p;

	if (flags & flag_verbose) printf("[%04X,%08X] ", 0x0036, number );
	token_buffer = tokenWriter( token_buffer, 0x0036, "4",  number );

	return token_buffer;
}

char *_number_( char *token_buffer, char *start, const char **ptr)
{
	BOOL neg = FALSE;
	int number = 0;
	int n;
	const char *p = *ptr;

	if (*p=='-')		// this might not be a number, it might be substract
	{
		if (
			(last_token == 0x0006) ||			// If token bedore the "-" symbol is variable 
			is_logical_operation( last_token ) ||		// We just do what brain dead AMOSPro does here.
			(last_token == 0x007C) ||			// ) symbol
			(last_token == 0x005C) ||			// comma symbol, this is another stupid mistake AMOSPro do.
			(last_token_is == is_command_type)||		// typical: Screen Width - number
			(last_token_is == is_number_type)
		)
		{
			// this is a substract symbol, exit here.
			return NULL;
		}
		else
		{
			 neg = TRUE; p++; 
		}
	}

	for (;is_break_char(*p)==FALSE; p++)
	{
		number = (number *10) + (*p - '0');
	}

	if (*ptr == p) return NULL;	// nothing found 

	*ptr = p;

	if (neg) number = -number;

	if (flags & flag_verbose)  printf("[%04X,%08X] ", 0x003E, number );
	token_buffer = tokenWriter( token_buffer,  0x003E, "4", number );

	return token_buffer;
}

char *_float_( char *token_buffer, const char **ptr)
{
	int number = 0;
	double f;
	const char *s;

	sscanf(*ptr, "%lf", &f );
	number = toAmosFloat( f );

	if (flags & flag_verbose)  printf("[%04X,%08X] ", 0x0046, number );

	s = *ptr;

	if (*s=='-')
	{
		token_buffer = tokenWriter( token_buffer, 0xFFCA, "" );
		s++;
	}

	token_buffer = tokenWriter( token_buffer, 0x0046, "4", number );
	if (token_buffer == NULL) printf("Str: '%s'\n",*ptr);

	while ( (is_break_char(*s) == FALSE) && (*s) ) s++;

	*ptr = s;

	return token_buffer;
}


char *_string_( char *token_buffer, const char **ptr)
{
	const char *s;
	char *dest;
	char *d;
	int token;
	int numStartEndSymbols = 2;
	unsigned short length = 0;
	char firstSymbol;

	s=*ptr;

	token = 0;

	firstSymbol = *s;

	switch(firstSymbol)
	{
		case '"' : token = 0x0026; break;
		case '\'': token = 0x002E; break;
	}

	// check if string it terminated correct.

	length = 0;
	for (s=(char *) (*ptr) + 1; ((*s!=firstSymbol) || (last_token == 0)) && (*s) ;s++) length++;

	if (*s==0)	// string was unexpected terminated.
	{
		if ((last_token == 0) && (firstSymbol=='\''))	// if we are on new line, and ' symbol is used, then its not a string but a rem.
		{
			token = 0x0652;
			numStartEndSymbols = 1;

			// skip space after rem symbol.
			if (*((char *) (*ptr) + 1) == ' ') 
			{
				*ptr = (char *) (*ptr) + 1;
				length --;
			}
		}
		else 	 return NULL;
	}

	dest = (char *) malloc(length + 1);

	if (dest)
	{
		int n = 0;

		d = dest;
		s=(char *) (*ptr) + 1;
		for (n = 0; n < length ;n++) 	*d++=s[n];
		*d = 0;

		*ptr = (((char *) *ptr) + length + numStartEndSymbols);

		if (flags & flag_verbose)  printf("[%04X,%04X,%s%s] ", token, length, dest, length &1 ? ",00" : "");
		token_buffer = tokenWriter( token_buffer, token, "2,s",  length , dest );

		free(dest);
	}

	return token_buffer;
};

double getDouble(int data)
{
	int n = 0;
	double u = 0.0f;

	for (n=23;n>-1;n--)
	{
		if (data & (1<<n) )
		{
			u += 1.0f / (double) (1<<(23-n) );
		}
	}

	return u;
}

int toAmosFloat(double v)
{
	int n;
	int number1 = 0;
	int data = 0;
	int e =0, E = 1;
	bool s = (v<0.0f) ;
	double u;
	double vv,uu;
	bool success = false;

	if (s) v=-v;

	while (v>=2.0f) {e++; v/=2.0f; };
	while (v<1.0f) {e--; v*=2.0f; };

	uu = 0;
	vv = v;
	for (n=23;n>-1;n--)
	{
		u = 1.0f / (double) (1<<(23-n));
	
		if (v>=u)
		{
			number1 |= (1<<n);
			v -= u;
			uu += u;
		}
	}

	// correct for error

	if ( uu<vv )
	{
		for (n = number1; n<((1<<24)-1);n++)
		{
			if (getDouble( n ) >= vv)
			{
				number1 = n;
				break;
			}
		}
	}


	if (e>0) 
	{
		E = e+1;
	}
	else if (e<0)
	{
		E=(e+65);
	}

	data = number1 << 8 | (s ? 0x80 : 0x00) | (e>-1 ? 0x40 : 0x00) | ( E & 0x3F);

	return data;
}
