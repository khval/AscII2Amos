#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "dynamicCommands.h"

DynamicCommand::DynamicCommand(unsigned short _token, unsigned short _extension, char *_name,int _args,BOOL _return_value)
{
	token = _token;
	extension = _extension;
	args = _args;
	return_value = _return_value;

	printf("%s%s,%d",
		return_value  ? "=" : "",
		_name,
		args);

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

