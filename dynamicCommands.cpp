#include <string.h>
#include <stdlib.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "dynamicCommands.h"

DynamicCommand::DynamicCommand(unsigned short _token, unsigned short _extension, char *_name,int args,BOOL return_value)
{
	token = _token;
	extension = _extension;

	if (_name)
	{
		name = strdup(_name);
		len = strlen(name);
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

