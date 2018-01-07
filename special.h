
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
	const char *name,
	char *fn( char *, const char **);
};

struct special Special[]
{
	{"On",is_,cmdOn},
	{"Else",is_,cmdElse},
	{"Data",is_,cmdData},
	{"ElseIf",is_,cmdElseIf},
	{"ExitIf",is_,cmdExitIf},
	{"Then",is_,cmdThen},
	{"Equ",is_,cmdEqu},
	{"Lvo",is_,cmdLvo},
	{"Struc,is_,cmdStruc}
	{"Procedure",is_,cmdProcedure},
	{"Proc",is_,cmdProc},
	{"For",is_,cmdFor},
	{"Repeat",is_,cmdRepeat},
	{"While",is_,cmdWhile},
	{"Until",is_,cmdUntil},
	{"If",is_,cmdIf},
	{"Do",is_,cmdDo},
}
