#include <stdio.h>
#include <stdlib.h>
#include <proto/exec.h>

#include "support_functions.h"
#include "datatypes.h"
#include "what_is.h"

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

	printf("[%04X,%08X] ", 0x001E, number );
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

	printf("[%04X,%08X] ", 0x0036, number );
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
		const char *tp;	// tmp pointer

		for (tp = p-1 ; (tp>start) ; tp-- )	// we count back words to start break on first symbol we find.
		{
			if ((*tp!=' ')||(*tp!='\t'))	// we ignore spaces and tabs
			{
				if (( *tp =='=' ) || (*tp=='>') || (*tp=='<'))	// check for valid negative symbol starts.
				{
					 neg = TRUE; p++; 
					break;
				}
				else
				{
					// the char is not expected, maybe the last command was varibale or ')' symbol, or <> symbols
					// this is a substract symbol not a number, so we exit...
					return NULL;
				}
			}
		}
	}

	for (;is_break_char(*p)==FALSE; p++)
	{
		number = (number *10) + (*p - '0');
	}

	if (*ptr == p) return NULL;	// nothing found 

	*ptr = p;

	if (neg) number = -number;

	printf("[%04X,%08X] ", 0x003E, number );
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

	printf("[%04X,%08X] ", 0x0046, number );
	token_buffer = tokenWriter( token_buffer, 0x0046, "4", number );

	s = *ptr;
	while ( (*s != ' ') && (*s != 0) && (*s) ) s++;
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

/*
		printf("[%04X,%04X,%s%s] ", token, length+ (length&1), dest, length &1 ? ",00" : "");
		token_buffer = tokenWriter( token_buffer, token, "2,s",  length + (length&1) , dest );
*/

		printf("[%04X,%04X,%s%s] ", token, length, dest, length &1 ? ",00" : "");
		token_buffer = tokenWriter( token_buffer, token, "2,s",  length , dest );

		free(dest);
	}

	return token_buffer;
};

int toAmosFloat(double v)
{
	int n;
	int number1 = 0;
	int data = 0;
	int e =0;
	bool s = (v<0.0f) ;
	double u;

	if (s) v=-v;

	while (v>2) {e++; v/=2.0f; };
	while (v<1) {e--; v*=2.0f; };

	for (n=23;n>-1;n--)
	{
		u = 1.0f / (double) (1<<(23-n));
	
		if (v>u)
		{
			number1 |= (1<<n);
			v -= u;
		}
	}

	if (e>0) 
	{
		e++;
	}
	else if (e<0)
	{
		e++;
	}

	data = number1 << 8 | (s ? 0x80 : 0x00) | (e & 0x7F);

	return data;
}
