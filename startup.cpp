
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/wb.h>
#include <proto/icon.h>
#include <workbench/startup.h>
#include <proto/asl.h>

#include "startup.h"


int32 SafeAddPart(char **oldPart, char *newPart)
{
	int32 success = 0;
	int size;
	char *tmp;
	size = strlen( *oldPart ? *oldPart : "" ) + strlen(newPart) + 2;	// maybe need a / or : , need a null at end of the string.
	tmp = (char *) malloc( size );

	if (tmp)
	{
		sprintf(tmp, "%s", *oldPart ? *oldPart : "" );
		success = AddPart( tmp, newPart, size );

		if (*oldPart) free(*oldPart);
		*oldPart = tmp;
	}
	return success;
}


char *get_filename(int args,char **arg)
{
	struct WBStartup	*wb;
	char *tmp;
	char	buffer[2000];

	if (args>1)
	{
		BOOL has_path = FALSE;
		BPTR currentLock;
		char *txt = arg[1];

		for(;*txt;txt++) if ((*txt=='/')||(*txt==':')) has_path = TRUE;

		if (has_path) return strdup(arg[1]);

		buffer[0];
		currentLock = GetCurrentDir();
		if (NameFromLock( currentLock , buffer, sizeof(buffer) ))
		{
			char *name_and_path = strdup(buffer);

			SafeAddPart(&name_and_path, arg[1] );
			return name_and_path;
		}
		else
		{
			return strdup(arg[1]);
		}
	}
	else // wbstartup
	{
		BPTR lock;
		char *filename;

		wb = (struct WBStartup *) arg;
		if ( wb -> sm_NumArgs == 2 )
		{
			filename = wb -> sm_ArgList[1].wa_Name;

			if (NameFromLock(  wb -> sm_ArgList[1].wa_Lock, buffer, sizeof(buffer) ))
			{
				char *name_and_path = strdup(buffer);
				SafeAddPart(&name_and_path, filename );
				return name_and_path;
			}
			else
			{

				printf("%s:%d\n",__FUNCTION__,__LINE__);

				return strdup(filename);
			}
		}
	}
	return NULL;
}

char *asl()
{
	struct FileRequester	 *filereq;
	char *ret = NULL;
	char c;
	int l;

	if (filereq = (struct FileRequester	 *) AllocAslRequest( ASL_FileRequest, TAG_DONE ))
	{
		if (AslRequestTags( (void *) filereq, ASLFR_DrawersOnly, FALSE,	TAG_DONE ))
		{
			if ((filereq -> fr_File)&&(filereq -> fr_Drawer))
			{
				l = strlen(filereq -> fr_Drawer);

				if (l>1)
				{
					c = filereq -> fr_Drawer[l-1];

					if (ret = (char *) malloc( strlen(filereq -> fr_Drawer) + strlen(filereq -> fr_File) +2 ))
					{
						sprintf( ret, ((c == '/') || (c==':')) ? "%s%s" : "%s/%s",  filereq -> fr_Drawer, filereq -> fr_File ) ;
					}
				}
				else 	ret = strdup(filereq -> fr_File);
			}
		}
		 FreeAslRequest( filereq );
	}

	return ret;
}
