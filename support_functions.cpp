#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

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
