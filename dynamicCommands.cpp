#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "special.h"
#include "dynamicCommands.h"

DynamicCommand::DynamicCommand(unsigned short _token, unsigned short _extension, char *_name,int _args,BOOL _return_value)
{
	token = _token;
	name = NULL;
	extension = _extension;
	return_value = _return_value;
	fn = NULL;
	args = _args;

	if (_name)
	{
		name = strdup(_name);
		len = strlen(name);
	}
}

DynamicCommand::DynamicCommand( struct special *item )
{
	token = -1;
	name = NULL;
	extension = 0;
	return_value = FALSE;
	fn = item -> fn;
	args = 0;

	if ( item -> name)
	{
		name = strdup( item -> name);
		len = strlen( item -> name);
	}
}

DynamicCommand::DynamicCommand()
{
	name = NULL;
}

DynamicCommand::~DynamicCommand()
{
	if (name) free(name);

	name = NULL;
}

