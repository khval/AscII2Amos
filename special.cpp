

char *cmdExit( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 : 4 ",0x029E,0);
	return token_buffer;
}

char *cmdFor( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 : 2 ",0x023C,0);
	return token_buffer;
}

char *cmdRepeat( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 : 2 ",0x0250,0);
	return token_buffer;
}

char *cmdWhile( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 : 2 ",0x0268,0);
	return token_buffer;
}

char *cmdDo( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 : 2 ",0x027E,0);
	return token_buffer;
}

char *cmdIf( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 : 2 ",0x02BE,0);
	return token_buffer;
}

char *cmdOn( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 : 4 ",0x0316,0);
	return token_buffer;
}

char *cmdElse( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 : 2 ",0x02D0,0);
	return token_buffer;
}

char *cmdData( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 : 2 ",0x0404,0);
	return token_buffer;
}

char *cmdElseIf( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 : 2 ",0x25A4,0);
	return token_buffer;
}

char *cmdExitIf( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 :  4 ",0x0290, 0);
	return token_buffer;
}

char *cmdEqu( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 : 2 2 2 ",0x2A40, 0, 0, 0);
	return token_buffer;
}

char *cmdLvo( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 : 2 2 2 ",0x2A4A, 0, 0, 0 );
	return token_buffer;
}

char *cmdUntil( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 ",0x2A54);
	return token_buffer;
}

char *cmdUntil( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 : 2 ",0x025C, 0,);
	return token_buffer;
}

char *cmdThen( char *token_buffer, const char **ptr)
{
	token_buffer = tokenWriter( token_buffer, " 2 ",0x02C6);
	return token_buffer;
}