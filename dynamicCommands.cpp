#include <string.h>
#include <stdlib.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "dynamicCommands.h"

DynamicCommand::DynamicCommand(unsigned short _token, char *_name,int args,BOOL return_value)
{
	token = _token;

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

