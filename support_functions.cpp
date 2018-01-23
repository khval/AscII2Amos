#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <proto/exec.h>

int last_token = 0;

#define my_va_ptr( list, type ) \
	*(( type * ) buffer) = (type) va_arg( list, int ); \
	buffer+=sizeof( type ); 

char *tokenWriter(char *buffer,unsigned short token , const char *fmt,   ... )
{
	int _len = 0;

	char *sptr;
	const char *c;
	va_list l;
	va_start( l,fmt);

	// write token to token buffer

	*(( unsigned short * ) buffer ) = token;
	buffer +=sizeof( unsigned short );

	// write token args to token buffer

	for (c=fmt;*c;c++)
	{
		switch (*c)
		{
			case '4':
			case 'l':	my_va_ptr( l, int ); break;

			case '2':
			case 'w':	my_va_ptr( l, short );  break;

			case '1':
			case 'b':	my_va_ptr( l, char ); break;

			case 's':
					_len = 0;
					for (sptr = (char *) va_arg(l, int ); *sptr; sptr++)
					{
						*buffer++=*sptr; _len++;
					}
					if (_len & 1) *buffer++= 0;	// 16 bit aligned text strings.
			}
	}
	va_end(l);

	last_token = token;

	return buffer;
}

char *tokenArgWriter(char *buffer, const char *fmt,   ... )
{
	int _len = 0;

	char *sptr;
	const char *c;
	va_list l;
	va_start( l,fmt);

	// write token args to token buffer

	for (c=fmt;*c;c++)
	{
		switch (*c)
		{
			case '4':
			case 'l':	my_va_ptr( l, int ); break;

			case '2':
			case 'w':	my_va_ptr( l, short );  break;

			case '1':
			case 'b':	my_va_ptr( l, char ); break;

			case 's':
					_len = 0;
					for (sptr = (char *) va_arg(l, int ); *sptr; sptr++)
					{
						*buffer++=*sptr; _len++;
					}
					if (_len & 1) *buffer++= 0;	// 16 bit aligned text strings.
			}
	}
	va_end(l);

	return buffer;
}

void Capitalize(char *str)
{
	int n;
	char c,lc;
	int upper = 'A'-'a';

	for (n=0;str[n];n++)
	{
		c = str[n];

		if (n==0)
		{
			str[n] = ((c>='a')  && (c <= 'z')) ? c+upper : c;
		}
		else if (n>0)
		{
			lc = str[n-1];

			if ((lc==' ')||(lc=='!'))
			{
				str[n] = ((c>='a')  && (c <= 'z')) ? c+upper : c;
			}
		}
	}
}

//	- First character:
//		The first character defines the TYPE on instruction:
//			I--> instruction
//			0--> function that returns a integer
//			1--> function that returns a float
//			2--> function that returns a string
//-- Not documented--
//			3--> is string used on AMAL command, 3 is also used other commands, the diff between 2 and 3 I don't know.
//-- End of not documented
//			V--> reserved variable. In that case, you must
//				state the type int-float-string
//	- If your instruction does not need parameters, then you stop
//	- Your instruction needs parameters, now comes the param list
//			Type,TypetType,Type...
//		Type of the parameter (0 1 2)
//		Comma or "t" for TO

BOOL return_value(const char *args)
{
	char rc;
	rc = args ? args[0] : 0;
	return ((rc>='0') && (rc<='3')) ? TRUE : FALSE;
}

int number_of_args(const char *aptr)
{
	char rc;
	int args = 0;

	if (aptr == NULL) return 0;
	if (aptr[0] == 0) return 0;

	if (*aptr=='V')	aptr++;	// any function that start with V is bull shit, I think.

	aptr++; // skip return type.
	rc = *aptr;	

	while (rc != 0)
	{
		args += ((rc>='0') && (rc<='3')) ? 1 : 0;
		aptr++;	// next
		rc = *aptr;
	}

	return args;
}
