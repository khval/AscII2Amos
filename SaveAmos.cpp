#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <proto/exec.h>
#include <proto/dos.h>

FILE *writeAMOSFileStart(const char *name)
{
	FILE *fd;

	fd=fopen(name,"w");
	if (fd)
	{
		char *id = (char *) "AMOS Basic v124 ";
		fwrite( id, strlen(id), 1 , fd );
	}

	return fd;
}

void writeAMOSFileBuffer( FILE *fd, char *buffer, int bufferSize, int blockSize )
{
	fseek(fd, 0x10, SEEK_SET);		// overwrite the size
	fwrite( &blockSize, sizeof(int), 1, fd);

	fseek(fd, 0, SEEK_END);			// apped buffer to end of file
	fwrite( buffer, bufferSize,1,fd);
}

void writeAMOSFileEnd( FILE *fd)
{
	int _null = 0 ;

	fwrite( "AmBs", 4, 1, fd);
	fwrite( &_null, 4, 1, fd);
	fwrite( &_null, 4, 1, fd);
	fwrite( &_null, 4, 1, fd);
	fwrite( &_null, 4, 1, fd);
	fclose(fd);
}

void writeAMOSLineAsFile(const char *name,char *buffer, int size)
{
	int _null = 0 ;
	FILE *fd;

	fd=writeAMOSFileStart(name);
	if (fd)
	{
		writeAMOSFileBuffer(fd,buffer,size,size);
		writeAMOSFileEnd(fd);
	}
}
