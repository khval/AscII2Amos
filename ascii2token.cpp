#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/asl.h>
#include <proto/amosextension.h>
#include "init.h"
#include "what_is.h"

#include <iostream>
#include <string>

using namespace std;

// For this test we only have one command..

#define my_va_ptr( list, type ) \
	*(( type * ) buffer) = (type) va_arg( list, int ); \
	buffer+=sizeof( type ); 


char src_token_buffer[1000];

struct symbol 
{
	unsigned short token;
	const char *symbol;
};

struct symbol Symbol[]=
{
	{0xFF8E,">="},	// numbers 
	{0xFF7A,"<="},	// numbers
	{0xFF98,">="},
	{0xFF84,"<="},
	{0xFF66,"<>"},
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
	{0xFFF6,"^"}
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

char *tokenWriter(char *buffer,const char *fmt,  ... )
{
	int _len = 0;

	char *sptr;
	const char *c;
	va_list l;
	va_start( l,fmt);

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

char *symbolToken(char *token_buffer, const char **ptr)
{
	int n;
	int l;

	for (n=0;n<sizeof(Symbol)/sizeof(struct symbol );n++)
	{
		l = strlen(Symbol[n].symbol);

		if ( strncmp( *ptr, Symbol[n].symbol, l) == 0 )
		{
			printf("[%08X] ",  Symbol[n].token);

			*ptr = (((char *) *ptr) + l);
			token_buffer = tokenWriter( token_buffer, "  2 ",  Symbol[n].token );
			return token_buffer;
		}
	}
	return NULL;
}


char *_start_of_line_( char *token_buffer, char length, char level )
{
	printf("[%01X,%01X] ", length, level);

	token_buffer = tokenWriter( token_buffer, "  1, 1 ",  length, level );
	return token_buffer;
}

char *_end_of_line_( char *token_buffer )
{
	printf("[%04X]", 0);

	token_buffer = tokenWriter( token_buffer, " 2 ",  0x0000 );
	return token_buffer;
}

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
	token_buffer = tokenWriter( token_buffer, " 2, 4 ", 0x001E ,  number );

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
	token_buffer = tokenWriter( token_buffer, " 2, 4 ", 0x0036 ,  number );

	return token_buffer;
}

char *_number_( char *token_buffer, const char **ptr)
{
	BOOL neg = FALSE;
	int number = 0;
	int n;
	const char *p = *ptr;

	if (*p=='-') { neg = TRUE; p++; }

	for (;is_break_char(*p)==FALSE; p++)
	{
		number = (number *10) + (*p - '0');
	}
	*ptr = p;

	if (neg) number = -number;

	printf("[%04X,%08X] ", 0x003E, number );
	token_buffer = tokenWriter( token_buffer, " 2, 4 ", 0x003E ,  number );

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
	token_buffer = tokenWriter( token_buffer, " 2, 4 ", 0x0046 ,  number );

	s = *ptr;
	while ( (*s != ' ') && (*s != 0) && (*s) ) s++;
	*ptr = s;

	return token_buffer;
}

char *_variable_( char *token_buffer, const char **ptr)
{
	char buffer[1000];
	const char *s;
	char *d;
	BOOL _break = FALSE;

	short unknown = 0;
	char	length = 0;
	char flags = 0;

	d = buffer;
	for (s=*ptr; (_break == FALSE) && (*s) ;s++) 
	{
		switch(*s)
		{
			case ' ':
			case '=':
			case '+':
			case '-':
			case '/':
			case '*':
					s--;	// this are not part of the variable name
					_break = TRUE;
					break;

			case '#':	flags = 1;	
					break;
			case '$':	flags = 2; 
					break;
			default: 
				*d++=*s; length++;
		}
	}

	*d = 0;
	*ptr = s;

	printf("[%04X,%04X,%02X,%02X,%s%s] ", 0x0006, unknown, length + (length &1), flags, buffer, length &1 ? ",00" : "");

	token_buffer = tokenWriter( token_buffer, " 2, 2, 1, 1, s ",  0x0006, unknown, length + (length &1), flags, buffer );

	return token_buffer;
};

struct cmd_line
{
	unsigned short token;
	const char *name;
	int len;
	int comma;
	BOOL return_value;
};

struct cmd_line cmds[] = {
	{0x0C6E,"Screen",0,1,FALSE},
	{0x09EA,"Screen Open",0,4,FALSE},
	{0x0054,":",0,0,FALSE}
};

void init_cmd_list()
{
	int n;
	int cmd_count = sizeof(cmds) / sizeof(struct cmd_line);

	struct cmd_line tmp;

	for (n=0; n<cmd_count; n++)
	{
		cmds[n].len = strlen(cmds[n].name);
	}
}


void order_by_cmd_length()
{
	int n;
	int cmd_count = sizeof(cmds) / sizeof(struct cmd_line);

	struct cmd_line tmp;

	for (n=0; n<cmd_count-1; n++)
	{
		if (cmds[n].len < cmds[n+1].len)	
		{
			tmp = cmds[n];
			cmds[n] = cmds[n+1];
			cmds[n+1] = tmp;
		}
	}
}

BOOL is_command(char *input, int args, BOOL has_return_value)
{
	int n;
	int cmd_count = sizeof(cmds) / sizeof(struct cmd_line);

	for (n=0; n<cmd_count; n++)
	{
		if (strncasecmp( input, cmds[n].name, cmds[n].len ) == 0 )
		{
			return TRUE;
		}
	}	

	return FALSE;
}

unsigned short find_token(const char **input )
{
	int n;
	int cmd_count = sizeof(cmds) / sizeof(struct cmd_line);

	for (n=0; n<cmd_count; n++)
	{
		if (strncasecmp( *input, cmds[n].name, cmds[n].len ) == 0 )
		{
			*input += cmds[n].len;

			return cmds[n].token;
		}
	}	
	return 0;
}

void list_commands()
{
	int n;
	int cmd_count = sizeof(cmds) / sizeof(struct cmd_line);

	for (n=0; n<cmd_count; n++)
	{
		printf("[%s]\n",cmds[n].name);
	}	
}


// return break point, to next command.

const char *get_info( const char *str, cmd_line &out )
{
	BOOL is_string = FALSE;
	BOOL has_ascii = FALSE;
	int equal_symbol = FALSE;
	int parentheses = 0;
	const char *ptr;

	out.return_value = FALSE;
	out.comma = 0;

	for (ptr = str; *ptr; ptr++)
	{
		if (is_string == FALSE)
		{
			switch (*ptr)
			{
				case '=':	if (has_ascii == TRUE)
						{
							printf("parentheses %d,comma %d\n", parentheses,out.comma);
							return ptr;
						}
						else
						{
							printf("command starts with '='");
							parentheses = -1;
						}
						break;

				case '(':	parentheses++;	break;
				case ')':	parentheses--;		break;
				case ':':	if (is_string == FALSE) 
				
					printf("parentheses %d,comma %d\n", parentheses,out.comma);

					return ptr+1;	
					break;

				case ',':	if (parentheses == 0) out.comma++; break;

				case ' ':	break; // ignore spaces.

				default:	has_ascii = TRUE;
						printf("*\n");
			}
		}

		if (*ptr=='"')	is_string = !is_string;	// swap is string or not
	}

	printf("parentheses %d,out.comma %d\n", parentheses,out.comma);
	return ptr;
}

int reformat_string(char *str)
{
	BOOL is_str = FALSE;
	char *dest;
	char *ptr;
	char lc = 0;
	int level = 0;

	dest = str;
	while (*str==' ') { str++; level++; }

	for (ptr=str;*ptr;ptr++)
	{
		if (*ptr == '"') is_str = TRUE;

		if (is_str)
		{
			*dest++=*ptr;
		}
		else
		{
			if ((lc != ' ')||(*ptr != ' ')) *dest++=*ptr;
		}
		lc = *ptr;
	}
	*dest = 0;
	return level;
}

void writeAMOS(char *buffer, int size)
{
	int _null = 0 ;
	FILE *fd;

	fd=fopen("amostest/ascii2amos.amos","w");
	if (fd)
	{
		char *id = (char *) "AMOS Basic v124 ";
		fwrite( id, strlen(id), 1 , fd );
		fwrite( &size, sizeof(int), 1,fd);
		fwrite( buffer, size,1,fd);
		fwrite( "AmBs", 4, 1, fd);
		fwrite( &_null, 4, 1, fd);
		fwrite( &_null, 4, 1, fd);
		fwrite( &_null, 4, 1, fd);
		fwrite( &_null, 4, 1, fd);
		fclose(fd);
	}

}

int main(int args, char **arg)
{
	string line;
	int read;
	int level;
	struct cmd_line command_info;
	const char *lptr;
	const char *ptr, *next_ptr;
	char *reformated_str;
	char	*ptr_token_buffer;


	if (init())
	{
		init_cmd_list();
		order_by_cmd_length();
		list_commands();
		unsigned short token;

		do
		{
			ptr_token_buffer = src_token_buffer;			// we calulate the size after line is converted.

			printf("\nEnter AMOS command line:\n");
			getline(cin, line);
			
			reformated_str = strdup( line.c_str() );
			if (reformated_str) if (reformated_str[0] != 0)
			{
				level = reformat_string(reformated_str);
				
				ptr_token_buffer = _start_of_line_( ptr_token_buffer, 0, level + 1 );		// we don't know the length yet.

				ptr = reformated_str;
				do
				{
					token = find_token( &ptr );

					if (token)
					{
						printf("[%04X] ", token);
						ptr_token_buffer = tokenWriter( ptr_token_buffer, " 2 ", token );
					}
					else
					{
						if (is_bin(ptr))
						{
							ptr_token_buffer = _bin_(ptr_token_buffer, &ptr );
						}
						else if (is_hex(ptr))
						{
							ptr_token_buffer = _hex_(ptr_token_buffer, &ptr );
						}
						else if (is_float(ptr))
						{
							ptr_token_buffer = _float_(ptr_token_buffer, &ptr );
						}
						else if (is_number(ptr))
						{
							ptr_token_buffer = _number_(ptr_token_buffer, &ptr );
						}
						else		// think this is variable
						{
							char *ret;

							ret =symbolToken(ptr_token_buffer , &ptr);
							if (ret)
							{


								ptr_token_buffer = ret;	// symbol token found.
							}
							else
							{


								ptr_token_buffer = _variable_(ptr_token_buffer, &ptr );
							}
						}
						Delay(10);
					}


					if (*ptr==' ') ptr++;
				} while ( *ptr );

				ptr_token_buffer = _end_of_line_(ptr_token_buffer );


				free(reformated_str);
				reformated_str = NULL;

				// length should be writen to start of line as a char, length is in x * short
				*src_token_buffer = ((int) ptr_token_buffer - (int) src_token_buffer) / 2 ;	

				printf("\n\nsize: %d\n", ptr_token_buffer - src_token_buffer);

				writeAMOS( src_token_buffer, (int) ptr_token_buffer - (int) src_token_buffer );
			}
	
		} while ( line.length() != 0 );

		closedown();
	}

	return 0;
}

