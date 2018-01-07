
#ifndef special_h
#define special_h

char *cmdExit( char *token_buffer, const char **ptr);
char *cmdFor( char *token_buffer, const char **ptr);
char *cmdRepeat( char *token_buffer, const char **ptr);
char *cmdWhile( char *token_buffer, const char **ptr);
char *cmdDo( char *token_buffer, const char **ptr);
char *cmdIf( char *token_buffer, const char **ptr);
char *cmdOn( char *token_buffer, const char **ptr);
char *cmdElse( char *token_buffer, const char **ptr);
char *cmdData( char *token_buffer, const char **ptr);
char *cmdElseIf( char *token_buffer, const char **ptr);
char *cmdExitIf( char *token_buffer, const char **ptr);
char *cmdEqu( char *token_buffer, const char **ptr);
char *cmdLvo( char *token_buffer, const char **ptr);
char *cmdUntil( char *token_buffer, const char **ptr);
char *cmdUntil( char *token_buffer, const char **ptr);
char *cmdThen( char *token_buffer, const char **ptr);

struct special
{
	const char *name;
	char *(*fn) ( char *, const char **);
};


#endif

