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
#include "ascii2token.h"

#include <iostream>

#include "ascii2amos.exe_rev.h"
STRPTR USED ver = (STRPTR) VERSTAG;

using namespace std;

// For this test we only have one command..

struct cli_arg
{
	const char *arg;
	int flag;
	int type;
	void *data;
};

ULONG flags = 0;
char src_token_buffer[1000];
struct extension *extensions[extensions_max];
unsigned short last_token_is;
BOOL _error = FALSE;

char *asc_filename = NULL;
char *amos_filename = NULL;

struct cli_arg arg_list[]=
{
	// flag options

	{"--verbose",flag_verbose,0,NULL},
	{"-v",flag_verbose,0,NULL},

	{"--help",flag_help,0,NULL},
	{"-h",flag_help,0,NULL},

	{"--Interactive",flag_Interactive,0,NULL },
	{"-i",flag_Interactive,0,NULL },

	// file options

	{"FROM", 0, e_string, &asc_filename},
	{"-s",0, e_string, &asc_filename},

	{"TO", 0, e_string, &amos_filename},
	{"-o", 0, e_string, &amos_filename}
};

void print_help()
{
	printf("Ascii2Amos\n\n");
	printf("Example:\n");
	printf("\tAscii2Amos FROM source.asc TO dest.amos\n");
	printf("OR\n");
	printf("\tAscii2Amos -s source.asc -o dest.amos\n\n");
	printf("-v,--verbose\n");
	printf("\tShow tokens and other info\n\n");
	printf("-i,--Interactive\n");
	printf("\tInteractive / debug mode\n\n");
	printf("-h,--help\n");
	printf("\tShow help\n\n");
	printf("-s,FROM\n");
	printf("\tAsci file from convert\n\n");
	printf("-o,TO\n");
	printf("\tAMOS file to create\n\n");
}

struct cli_arg *find_arg( char *str)
{
	struct cli_arg *arg;
	int cnt=sizeof(arg_list) / sizeof(struct cli_arg);

	for (arg = arg_list; arg < arg_list+cnt; arg++ )
	{
		if (strcasecmp(arg->arg,str)==0) return arg;
	}
	return NULL;
}

BOOL read_args(int args, char **arg)
{
	struct cli_arg *f;
	int n;
	flags = 0;

	for (n=1; n<args;n++)
	{
		f = find_arg( arg[n] );

		if (f)
		{
			if (f->flag) flags |= f-> flag;
			if (f->data)
			{
				n++;
				if (n<args)
				{
					switch (f->type)
					{
						case e_string:	
							if (*((char **) f->data)) return FALSE;
							*((char **) f->data) = strdup(arg[n]);
					}
				}
			}
		}
		else return FALSE;
	}

	return TRUE;
}


extern char *symbolToken(char *token_buffer, const char **ptr);

void print_arg(char *arg)
{
	for ( ; ((*arg != ':') && (*arg != 0)) ; arg++)
	{
		printf("%c",*arg);
	}
	printf("\n");
}

char *extensionToken(char *token_buffer, int token, int table )
{
	if (flags & flag_verbose) printf("[%04X,%02X,%02X,%04X] ",0x004E , table, 0, token );
	token_buffer = tokenWriter( token_buffer, 0x004E, "1,1,2", table,0, token );
	return token_buffer;
}

char *_start_of_line_( char *token_buffer, char length, char level )
{
	if (flags & flag_verbose) printf("[%02X,%02X] ", length, level);

	token_buffer = tokenArgWriter( token_buffer, "1,1",  length, level );
	return token_buffer;
}

char *_end_of_line_( char *token_buffer )
{
	if (flags & flag_verbose) printf("[%04X]", 0);

	token_buffer = tokenWriter( token_buffer, 0x0000, ""  );
	return token_buffer;
}

BOOL chars_before( const char *start, const char *at )
{
	const char *p;

	for ( p = at; p>=start; p--)
	{
		if (*p==':') break;		// break on before command

		if ((*p!=' ')&&(*p!='\t'))
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL chars_after( const char *at )
{
	const char *p;

	for ( p = at; *p!=0; p++)
	{
		if (*p==':') break;		// break on after command

		if ((*p!=' ')&&(*p!='\t'))
		{
			return TRUE;
		}
	}

	return FALSE;
}

char *_variable_( char *token_buffer, const char *start, const char **ptr)
{
	char buffer[1000];
	const char *s;
	char *d;
	BOOL _break = FALSE;
	short unknown = 0;
	char	length = 0;
	char type = 0;

	// standard token is variable
	unsigned short token = 0x0006;

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
			case ']':
			case '<':
			case '>':
			case '(':
			case ')':
			case ',':
			case '*':
			case '^':
					s--;	// this are not part of the variable name
					_break = TRUE;
					break;

			case '#':	type = 1;	
					_break = TRUE;
					break;

			case '$':	type = 2;	
					_break = TRUE;
					break;
			case '[':		// this is a function with args
					token = 0x0012;
					_break = TRUE;
					s--;		// this not part of the name.
					break;

			case ':':	if (last_token == 0)
					{
						token = 0x000C;		// 0x000C is label
					}
					else s--;

					_break = TRUE;
					break;

			default: 
				*d++=*s; length++;
		}
	}

	// if last command was a Resume then this token should be a label.
	if ((last_token==0x0330)&&(type == 0)) token = 0x0018;

	// Prcedure name, and Goto, Gosub command uses this token
	if ((last_token == 0x02B2)||(last_token==0x0376)||(last_token == 0x02A8)) token = 0x0006;

	*d = 0;

	if (*ptr == s) return NULL;	// string did not change so it can't be variale

	if (token == 0x0006)	// in some cases it might not be variable, can be a function.
	{
		if ( (chars_before(start,((char *) *ptr)-1) == FALSE) && (chars_after(s) == FALSE ) )
		{
			token = 0x0012;
		}
	}

	*ptr = s;

	if ((token == 0x0006)||(token == 0x000C))	// normal variables & labels use this format.
	{
		if (flags & flag_verbose ) printf("[%04X,%04X,%02X,%02X,%s%s] ", token, unknown, length+(length&1), type, buffer, length &1 ? ",00" : "");
		token_buffer = tokenWriter( token_buffer, token, "2, 1, 1, s" , unknown, length+(length&1), type, buffer );
	}
	else	// some token use this format.
	{
		if (flags & flag_verbose ) printf("[%04X,%04X,%02X,%02X,%s%s] ", token, unknown, length, type, buffer, length &1 ? ",00" : "");
		token_buffer = tokenWriter( token_buffer, token, "2, 1, 1, s" , unknown, length, type, buffer );
	}

	return token_buffer;
};



vector<DynamicCommand *> DCommands;

void init_cmd_list()
{
	DynamicCommand *_new;
	struct special *specialItem;

	for (struct native *item=nativeList; item -> name; item++)
	{
		_new = new DynamicCommand( item -> token, 0, (char *) item -> name, 
			number_of_args( item -> args ),
			return_value( item -> args));

		if (_new)	DCommands.push_back(_new);
	}

	for (specialItem=Special;specialItem->name;specialItem++)
	{
		_new = new DynamicCommand( specialItem );
		if (_new)	DCommands.push_back(_new);
	}
}


// when we look for commands we looks for command with longest names and most arguments first

void order_by_cmd_length_and_args()
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
			if ((DCommands[i]->len < DCommands[i+1]->len) || 
				(DCommands[i]->len == DCommands[i+1]->len) && ( DCommands[i]->args < DCommands[i+1]->args))
			{
				sorted = TRUE;
				tmp = DCommands[i];
				DCommands[i] = DCommands[i+1];
				DCommands[i+1] = tmp;
				break;
			}
		}
	} while (sorted);
}

struct find_token_return 
{
	unsigned short token;
	unsigned short extension;
};


// parentheses shoud be -1, if command shoud have return value

int get_parmeters_count( const char *str , int parentheses)
{
	BOOL is_string_double = FALSE;
	BOOL is_string_single = FALSE;
	BOOL has_ascii = FALSE;
	const char *ptr;
	int comma = 0;

	if (parentheses==-1)	// '(' symbol is expected
	{
		const char *p;
		for (p=str;(*p==' ')||(*p=='\'');p++);	// skip spaces and tabs
		if (*p!='(') return 0;		// this one is not valid
	}

	if ((last_token!=0x0054)&&(last_token!=0x0000)&&(*str == ',')) return 0;	

	for (ptr = str; *ptr; ptr++)
	{
		if (((is_string_double)||(is_string_single)) == FALSE)
		{

			if ((parentheses == 0)&&(*ptr==')'))
			{
				return (comma>0) ? (comma+1) : (has_ascii ? 1 : 0) ;
			}

			if (strncasecmp(ptr," to ",4)==0)
			{
				if (parentheses == 0) comma++; 
				ptr+=3;
			}
			else
			{
				switch (*ptr)
				{
					case '(':	parentheses++;	break;
					case ')':	parentheses--;		break;

					case ':':	
						return (comma>0) ? (comma+1) : (has_ascii ? 1 : 0)  ;
						// exit end of this command.

					case ',':	if (parentheses == 0) comma++; break;

					case '\t':
					case ' ':	break; // ignore spaces and tabs.

					default:	if (parentheses >-1) has_ascii = TRUE;
				}
			}
		}

		if ( (*ptr=='"') && (is_string_single == FALSE) )	is_string_double = !is_string_double;
		if ( (*ptr=='\'') && (is_string_double == FALSE) )	is_string_single = !is_string_single;	
	}

	return (comma>0) ? (comma+1) : (has_ascii ? 1 : 0) ;
}

DynamicCommand *find_token(const char **input )
{
	int i;
	char c;
	int pc;

	for (i = 0 ; i<DCommands.size() - 1; i++ )
	{
		if (strncasecmp( *input, DCommands[i]->name, DCommands[i]->len ) == 0 )
		{
			c = ((char *) *input ) [ DCommands[i] -> len ];

			if ((c==0)||(c=='<')||(c=='>')||(c=='/')||(c=='*')||(c=='-')||(c=='+')||(c==',')||(c=='(')||(c==')')||(c=='[')||(c==']')||(c==' ')||(c=='=')||((c>='0')&&(c<='9')))		// the correct terminated command name in a prompt.
			{
				// so we found a command with right name, but does have correct number paramiters?

				if ( DCommands[i] -> args > 0)
				{
					pc = get_parmeters_count( ((char *) (*input)) + DCommands[i]->len,  DCommands[i] -> return_value ? -1 : 0 );

					if (flags & flag_verbose ) 
					{
						printf("Token %04X: %s %s Args %d\n", DCommands[i] -> token, DCommands[i] -> return_value ? "=" : ":", DCommands[i]->name, DCommands[i]-> args );
						printf("get_parmeters_count = %d\n", pc );
					}
				}
				else pc = 0;

				if ( DCommands[i] -> args == pc ) 
				{
					*input += DCommands[i] -> len;
					return DCommands[i] ;
				}
			}
		}
	}	

	return NULL;
}


int reformat_string(char *str)
{
	BOOL is_rem = FALSE;
	BOOL is_str = FALSE;
	char *dest;
	char *ptr;
	char lc = 0;
	int level = 0;

	dest = str;
	for ( ; (*str==' ')||(*str=='\t') ; str ++) { level+= (*str==' ') ? 1 : 3 ; }	// if line starts with tabs or spaces, AMOS don't use tabs

	// need this, rem's should not be formated.

	if (strncasecmp(str,"rem ",4)==0) is_rem = TRUE;
	if (*str=='\'') is_rem = TRUE;

	if (is_rem)
	{
		for (ptr=str;*ptr;ptr++)
		{
			*dest++=*ptr;
		}
		*dest = 0;
	}
	else
	{
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
	}
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

		while ( (getline( myfile, line)) && (_error == FALSE) )
		{
			reformated_str = strdup( line.c_str() );

			if (flags & flag_verbose) printf("%s\n",reformated_str);

			if (reformated_str) if (reformated_str[0] != 0)
			{
				ptr_token_buffer = encode_line( reformated_str, src_token_buffer );

				free(reformated_str);
				reformated_str = NULL;

				// length should be writen to start of line as a char, length is in x * short
				bufferSize = ((int) ptr_token_buffer - (int) src_token_buffer);
				src_token_buffer[0] = (char) (bufferSize / 2) ;
				tokenBlockSize +=  (int) ptr_token_buffer - (int) src_token_buffer;
				if (fd) writeAMOSFileBuffer( fd, src_token_buffer, bufferSize, tokenBlockSize );
			}
			else 
			{
				ptr_token_buffer = _start_of_line_( src_token_buffer, 0, 1 );
				ptr_token_buffer = _end_of_line_( ptr_token_buffer );
				bufferSize = ((int) ptr_token_buffer - (int) src_token_buffer);
				src_token_buffer[0] = (char) (bufferSize / 2) ;
				tokenBlockSize +=  (int) ptr_token_buffer - (int) src_token_buffer;
				if (fd) writeAMOSFileBuffer( fd, src_token_buffer, bufferSize, tokenBlockSize );
			}

			if (flags & flag_verbose) printf("\n");
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
		printf("\n\nEnter AMOS command line:\n");
		getline(cin, line);

		reformated_str = strdup( line.c_str() );

		if (reformated_str) if (reformated_str[0] != 0)
		{
			ptr_token_buffer = encode_line( reformated_str, src_token_buffer );

			free(reformated_str);
			reformated_str = NULL;

			// length should be writen to start of line as a char, length is in x * short
			*src_token_buffer = ((int) ptr_token_buffer - (int) src_token_buffer) / 2 ;	
		}

	} while ( line.length() != 0 ) ;
}


char	* encode_line(char *reformated_str, char *ptr_token_buffer)
{
	int level;
	const char *ptr;
	unsigned short token;
	char *ret = NULL;

	level = reformat_string(reformated_str);
				
	ptr_token_buffer = _start_of_line_( ptr_token_buffer, 0, level + 1 );		// we don't know the length yet.

	last_token_is = is_newline_type ;

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
				_error = TRUE;
				break;
			}

			ptr_token_buffer = ret;
			last_token_is = is_string_type;
		}
		else	if (is_bin(ptr))
		{
			ptr_token_buffer = _bin_(ptr_token_buffer, &ptr );
			ret = (char *) 1;
			last_token_is = is_number_type;
		}
		else if (is_hex(ptr))
		{
			ptr_token_buffer = _hex_(ptr_token_buffer, &ptr );
			ret = (char *) 1;
			last_token_is = is_number_type;
		}
		else if (is_float(ptr))
		{
			ptr_token_buffer = _float_(ptr_token_buffer, &ptr );
			ret = (char *) 1;
			last_token_is = is_number_type;
		}
		else if (is_number(ptr))
		{
			ret = _number_(ptr_token_buffer, reformated_str,  &ptr );
			if (ret) 
			{
				ptr_token_buffer = ret;
				last_token_is = is_number_type;
			}
		}
					
		if (!ret)
		{
			if (!ret)
			{
				ret = symbolToken(ptr_token_buffer , &ptr);
				if (ret)
				{
					ptr_token_buffer = ret;
					last_token_is = is_symbol_type;
				}
			}

			if (!ret)	//  its not a special command, and its not symbolToken.
			{
				DynamicCommand *info = find_token( &ptr );
				if (info)
				{
					if (info->extension == 0)
					{
						if (info -> fn == NULL)
						{
							if (flags & flag_verbose) printf("[%04X] ", info->token);
							ret = ptr_token_buffer = tokenWriter( ptr_token_buffer, info->token, "" );
							last_token_is = is_command_type;
						}
						else
						{
							ret = ptr_token_buffer = info->fn( ptr_token_buffer, &ptr );
							last_token_is = is_command_type;
						}
					}
					else
					{
						ret = ptr_token_buffer = extensionToken( ptr_token_buffer,  info->token, info->extension );
						last_token_is = is_command_type;
					}
				}
			}

			if (!ret)
			{
				ret = _variable_(ptr_token_buffer, reformated_str, &ptr );
				if (ret)
				{
					ptr_token_buffer = ret;
					last_token_is = is_var_type;
				}
			}

			if (!ret)
			{
				printf("**break - can't decode\n");
				break;
			}
		}

		while ((*ptr==' ')||(*ptr=='\t')) ptr++;
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

		if (read_args(args,arg))
		{
			if (flags & flag_help)
			{
				print_help();
			}
			else	if (flags & flag_Interactive )
			{
				init_extensions();
				load_extensions( NULL );
				extensions_to_commands();
				free_extensions();
				order_by_cmd_length_and_args();

				flags |= flag_verbose;
				interactiveAmosCommandLine();
			}
			else
			{
				init_extensions();
				load_extensions( asc_filename);
				extensions_to_commands();
				free_extensions();
				order_by_cmd_length_and_args();

				if ( ( asc_filename ) && ( amos_filename ) )
				{
					asciiAmosFile( asc_filename , amos_filename );
				}
				else
				{
					print_help();
				}
			}

			free_commands();
		}
		else
		{
			print_help();
		}

		if (asc_filename) free(asc_filename);
		if (amos_filename) free(amos_filename);

		asc_filename = NULL;
		amos_filename = NULL;
	}

	closedown();	// not all libs needs to be opened to run closedown.

	return 0;
}
