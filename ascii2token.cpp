#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <vector>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/asl.h>
#include <proto/amosextension.h>
#include "init.h"
#include "what_is.h"
#include "support_functions.h"
#include "special.h"
#include "datatypes.h"
#include "dynamicCommands.h"
#include "load_interpreter_config.h"
#include "argflags.h"
#include "native.h"
#include "saveAmos.h"

#include <iostream>


using namespace std;

// For this test we only have one command..

ULONG flags = 0;
char *filename = NULL;

char src_token_buffer[1000];
struct extension *extensions[extensions_max];

extern char *symbolToken(char *token_buffer, const char **ptr);

char *specialToken(char *token_buffer, const char **ptr)
{
	struct special *itm;
	const char *s;
	char c;
	int l;

	for (itm=Special;itm->name;itm++)
	{
		l = strlen(itm->name);

//		printf("*%s*\n", itm -> name);

		if ( strncasecmp( *ptr, itm->name, l) == 0 )
		{
			c = ((char *) (*ptr)) [ strlen(itm ->name) ];

			if ((c==' ')||(c=='(')||(c==0))		// command needs to terminated correct to be vaild.
			{
				token_buffer = itm->fn( token_buffer, ptr );
				*ptr = ((char *) (*ptr)+l);	// next;
				return token_buffer;
			}
		}
	}
	return NULL;
}

char *extensionToken(char *token_buffer, int token, int table )
{
	printf("[%04X,%02X,%02X,%04X] ",0x004E , table, 0, token );
	token_buffer = tokenWriter( token_buffer, 0x004E, "1,1,2", table,0, token );
	return token_buffer;
}

char *_start_of_line_( char *token_buffer, char length, char level )
{
	printf("[%01X,%01X] ", length, level);

	token_buffer = tokenArgWriter( token_buffer, "1,1",  length, level );
	return token_buffer;
}

char *_end_of_line_( char *token_buffer )
{
	printf("[%04X]", 0);

	token_buffer = tokenWriter( token_buffer, 0x0000, ""  );
	return token_buffer;
}

char *_variable_( char *token_buffer, const char **ptr)
{
	char buffer[1000];
	const char *s;
	char *d;
	BOOL _break = FALSE;

	unsigned short token = 0x0006;
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
			case '[':
			case ']':
			case '(':
			case ')':
			case '*':
					s--;	// this are not part of the variable name
					_break = TRUE;
					break;

			case '#':	flags = 1;	
					break;
			case '$':	flags = 2; 
					break;
			case ':':	if (last_token == 0)
					{
						token = 0x000C;
					}
					else s--;

					_break = TRUE;
					break;

			default: 
				*d++=*s; length++;
		}
	}

	*d = 0;

	if (*ptr == s) return NULL;	// string did not change so it can't be variale

	*ptr = s;

	printf("[%04X,%04X,%02X,%02X,%s%s] ", token, unknown, length+(length&1), flags, buffer, length &1 ? ",00" : "");

	token_buffer = tokenWriter( token_buffer, token, "2, 1, 1, s" , unknown, length+(length&1), flags, buffer );

	return token_buffer;
};



vector<DynamicCommand *> DCommands;

void init_cmd_list()
{
	DynamicCommand *_new;
	BOOL return_value = FALSE;
	int args = 0;

	for (struct native *item=nativeList; item -> name; item++)
	{
//		printf("[%s]\n", item -> name);

		_new = new DynamicCommand( item -> token, 0, (char *) item -> name, args, return_value);
		if (_new)	DCommands.push_back(_new);
	}
}


void order_by_cmd_length()
{
	int i;
	DynamicCommand *tmp;
	BOOL sorted;

	// sort until nothing more to sort, not unlike bobble sort but not so stupid.
	do
	{
		sorted = FALSE;

		for (i = 0 ; i<DCommands.size() - 1; i++ )
		{
			if (DCommands[i]->len < DCommands[i+1]->len)
			{
				tmp = DCommands[i];
				DCommands[i] = DCommands[i+1];
				DCommands[i+1] = tmp;
			}
		}
	} while (sorted);
}

struct find_token_return 
{
	unsigned short token;
	unsigned short extension;
};

struct find_token_return find_token(const char **input )
{
	struct find_token_return ret;
	int i;
	char c;

	for (i = 0 ; i<DCommands.size() - 1; i++ )
	{
		if (strncasecmp( *input, DCommands[i]->name, DCommands[i]->len ) == 0 )
		{
			c = ((char *) *input ) [ DCommands[i] -> len ];

			if ((c==0)||(c=='(')||(c==' '))		// the correct terminated command name in a prompt.
			{
				*input += DCommands[i] -> len;

				ret.token = DCommands[i] -> token;
				ret.extension = DCommands[i] -> extension;

				return ret;
			}
		}
	}	

	ret.token = 0;
	ret.extension = 0;
	return ret;
}

void list_commands()
{
	int i;

	printf("-- start of command list --\n\n");

	for (i = 0 ; i<DCommands.size() - 1; i++ )
	{
		printf("[%s]\n",DCommands[i] -> name);
	}	

	printf("-- end of command list --\n\n");
}


// return break point, to next command.


int reformat_string(char *str)
{
	BOOL is_str = FALSE;
	char *dest;
	char *ptr;
	char lc = 0;
	int level = 0;

	dest = str;
	while ((*str==' ')||(*str=='\t')) { str++; level+= *str==' ' ? 1 : 3 ; }	// if line starts with tabs or spaces, AMOS don't use tabs

	for (ptr=str;*ptr;ptr++)
	{
		if (*ptr == '"') is_str = TRUE;

		if (is_str)
		{
			*dest++=*ptr;
		}
		else
		{
			if (*ptr == '\t') *ptr = ' ';		// tabs are not legal outside of strings
			if ((lc != ' ')||(*ptr != ' ')) *dest++=*ptr;
		}
		lc = *ptr;
	}
	*dest = 0;
	return level;
}


char *encode_line(char *reformated_str, char	*ptr_token_buffer);


void asciiAmosFile( const char *name, const char *outputfile )
{
	char *reformated_str;
	char *ptr_token_buffer;
	string line;
	ifstream myfile( name );
	FILE *fd;
	unsigned int bufferSize = 0;
	unsigned int tokenBlockSize = 0;

	if (myfile.is_open())
	{
		fd = writeAMOSFileStart( outputfile );

		while ( getline( myfile, line) )
		{
			reformated_str = strdup( line.c_str() );

			if (reformated_str) if (reformated_str[0] != 0)
			{
				printf("%s\n",reformated_str);

				ptr_token_buffer = encode_line( reformated_str, src_token_buffer );

				free(reformated_str);
				reformated_str = NULL;

				// length should be writen to start of line as a char, length is in x * short

				bufferSize = ((int) ptr_token_buffer - (int) src_token_buffer);

				src_token_buffer[0] = (char) (bufferSize / 2) ;

				printf("\n");

				tokenBlockSize +=  (int) ptr_token_buffer - (int) src_token_buffer;
					
				if (fd) writeAMOSFileBuffer( fd, src_token_buffer, bufferSize, tokenBlockSize );
			}
			else 
			{

				ptr_token_buffer = _start_of_line_( src_token_buffer, 0, 1 );
				ptr_token_buffer = _end_of_line_( ptr_token_buffer );

				bufferSize = ((int) ptr_token_buffer - (int) src_token_buffer);
				src_token_buffer[0] = (char) (bufferSize / 2) ;

				printf("\n");

				tokenBlockSize +=  (int) ptr_token_buffer - (int) src_token_buffer;

				if (fd) writeAMOSFileBuffer( fd, src_token_buffer, bufferSize, tokenBlockSize );
			}
		}

		if (fd) writeAMOSFileEnd(fd);

		myfile.close();
	}
	else cout << "unable to open file";
}

void  interactiveAmosCommandLine()
{
	char *reformated_str;
	char *ptr_token_buffer;
	string line;

	do
	{
		printf("\nEnter AMOS command line:\n");
		getline(cin, line);

		reformated_str = strdup( line.c_str() );

		if (reformated_str) if (reformated_str[0] != 0)
		{
			ptr_token_buffer = encode_line( reformated_str, src_token_buffer );

			free(reformated_str);
			reformated_str = NULL;

			// length should be writen to start of line as a char, length is in x * short
			*src_token_buffer = ((int) ptr_token_buffer - (int) src_token_buffer) / 2 ;	

			printf("\n\nsize: %d\n", ptr_token_buffer - src_token_buffer);

			printf("%08x - %08x\n", src_token_buffer, ptr_token_buffer);

			writeAMOSLineAsFile( "amostest/ascii2amos.amos", src_token_buffer, (int) ptr_token_buffer - (int) src_token_buffer );
		}

	} while ( line.length() != 0 );
}


char	* encode_line(char *reformated_str, char *ptr_token_buffer)
{
	int level;
	const char *ptr;
	unsigned short token;
	char *ret = NULL;

	level = reformat_string(reformated_str);
				
	ptr_token_buffer = _start_of_line_( ptr_token_buffer, 0, level + 1 );		// we don't know the length yet.

	ptr = reformated_str;
	do
	{
		ret = NULL;
	
		if ((*ptr =='"')||(*ptr =='\'')) 	// is string.
		{
			ret = _string_(ptr_token_buffer, &ptr );

			if (ret == NULL)
			{
				printf("**break - string not terminated\n");
				break;
			}

			ptr_token_buffer = ret;
		}
		else	if (is_bin(ptr))
		{
			ptr_token_buffer = _bin_(ptr_token_buffer, &ptr );
			ret = (char *) 1;
		}
		else if (is_hex(ptr))
		{
			ptr_token_buffer = _hex_(ptr_token_buffer, &ptr );
			ret = (char *) 1;
		}
		else if (is_float(ptr))
		{
			ptr_token_buffer = _float_(ptr_token_buffer, &ptr );
			ret = (char *) 1;
		}
		else if (is_number(ptr))
		{
			ret = _number_(ptr_token_buffer, &ptr );
			if (ret) ptr_token_buffer = ret;
		}
					
		if (!ret)
		{
			if (!ret)
			{
				ret = symbolToken(ptr_token_buffer , &ptr);
				if (ret) ptr_token_buffer = ret;
			}

			if (!ret)
			{
				ret = specialToken(ptr_token_buffer , &ptr);
				if (ret) ptr_token_buffer = ret;	
			}

			if (!ret)	//  its not a special command, and its not symbolToken.
			{
				struct find_token_return info = find_token( &ptr );
				if (info.token)
				{
					if (info.extension == 0)
					{
						printf("[%04X] ", info.token);
						ret = ptr_token_buffer = tokenWriter( ptr_token_buffer, info.token, "" );
					}
					else
					{
						ret = ptr_token_buffer = extensionToken( ptr_token_buffer,  info.token, info.extension );
					}
				}
			}

			if (!ret)
			{
				ret = _variable_(ptr_token_buffer, &ptr );
				if (ret) ptr_token_buffer = ret;	
			}

			if (!ret)
			{
				printf("**break - can't decode\n");
				break;
			}
		}

		if (*ptr==' ') ptr++;
	} while ( *ptr );

	ptr_token_buffer = _end_of_line_(ptr_token_buffer );

	return ptr_token_buffer;
}


void free_commands()
{
	DynamicCommand *_item;

	while (!DCommands.empty())
	{
		_item = DCommands.front();
		if (_item) delete _item;
		DCommands.erase(DCommands.begin());
	}
}

int main(int args, char **arg)
{

	int read;
	const char *lptr;
	const char *ptr, *next_ptr;
	char *reformated_str;
	char	*ptr_token_buffer;

	flags = flag_verbose;

	if (init())
	{
		init_cmd_list();

		if (args>1) filename = arg[1];

		init_extensions();
		load_extensions( filename);
		extensions_to_commands();
		free_extensions();
		order_by_cmd_length();

		if (args==1)
		{
			interactiveAmosCommandLine();
		}
		else
		{
			asciiAmosFile( arg[1] , "amostest/ascii2amos.amos" );
		}

		free_commands();

		closedown();
	}

	return 0;
}
