
FILE *writeAMOSFileStart(const char *name);
void writeAMOSFileBuffer( FILE *fd, char *buffer, int bufferSize, int blockSize );
void writeAMOSFileEnd( FILE *fd);
void writeAMOSLineAsFile(const char *name,char *buffer, int size);

