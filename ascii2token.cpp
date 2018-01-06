#include <stdio.h>
#include <stdlib.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/asl.h>
#include <proto/amosextension.h>
#include "init.h"

#include <iostream>
#include <string>

using namespace std;

// For this test we only have one command..


BOOL is_command(char *input, int args, BOOL has_return_value)
{
	const char *cmd_name = "screen open";

	while (*input == ' ') input++;

	if (strncasecmp(input,cmd_name,strlen(cmd_name))==0)
	{
		if (has_return_value == FALSE);
		{
			if (args == 4) return TRUE;
		}
	}

	return FALSE;
}

struct cmd_info 
{
	char ret;
	int comma;
	BOOL return_value;
};


// return break point, to next command.

const char *get_info( const char *str, cmd_info &out )
{
	BOOL is_string = FALSE;
	BOOL has_ascii = FALSE;
	int equal_symbol = FALSE;
	int parentheses = 0;
//	int comma = 0;
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



int main(int args, char **arg)
{
	string line;
	int read;
	cmd_info command_info;
	const char *lptr;
	const char *ptr, *next_ptr;

	if (init())
	{

		do
		{
			printf("Enter AMOS command line:\n");
			getline(cin, line);
			
			ptr = line.c_str();
			do
			{
				lptr = ptr;
				next_ptr = get_info( ptr, command_info );

				if (is_command( (char *) ptr, command_info.comma + 1, command_info.return_value ))
				{
					printf("yep");
				}

				printf("-- %08x next command -- \n", next_ptr);
				Delay(10);
		

				if (lptr == next_ptr) {
					printf("broken\n");
					break;
				}

				ptr = next_ptr;

			} while (*ptr != 0);

			printf("\n\n");
		} while ( line.length() != 0 );

		closedown();
	}

	return 0;
}

