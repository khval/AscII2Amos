#include <stdlib.h>
#include <stdio.h>
#include <proto/exec.h>
#include <proto/dos.h>

BOOL is_break_char( char c )
{
	switch (c)
	{
		case ' ': return TRUE;
		case ':': return TRUE;
		case ';': return TRUE;
		case ',': return TRUE;
		case '+': return TRUE;
		case '-': return TRUE;
		case '*': return TRUE;
		case '(': return TRUE;
		case ')': return TRUE;
		case '[': return TRUE;
		case ']': return TRUE;
		case '/': return TRUE;
		case 0: return TRUE;
	}
	return FALSE;
}

BOOL is_bin(const char *ptr)
{
	if (*ptr++!='%') return false;

	for ( ;is_break_char(*ptr)==FALSE; ptr++ )
	{
		if ((*ptr!='0')&&(*ptr!='1')) return FALSE;		
	}
	return TRUE;
}

BOOL is_hex(const char *ptr)
{
	char c;
	if (*ptr++!='$') return false;
	
	for ( ;is_break_char(*ptr)==FALSE; ptr++ )
	{
		c=*ptr;
		if (( ((c>='0')&&(c<='9')) || ((c>='a')&&(c<='f')) || ((c>='A')&&(c<='F')) ) == FALSE) return FALSE;
	}
	return TRUE;
}

BOOL is_float(const char *ptr)
{
	char c;
	int dots = 0;
	int ret = 0;

	if (*ptr=='-') ptr++;		// numbers can start whit neg number.

	for ( ;is_break_char(*ptr)==FALSE; ptr++ )
	{
		c=*ptr;
		if (c == '.') dots++;
		if (((c=='.') ||  (c>='0')&&(c<='9')) == FALSE ) return FALSE;
	}

	return (dots == 1);
}

BOOL is_number(const char *ptr)
{
	char c;
	int ret = 0;

	if (*ptr=='-') ptr++;		// numbers can start whit neg number.

	for ( ;is_break_char(*ptr)==FALSE; ptr++ )
	{
		c=*ptr;
		if ( ((c>='0')&&(c<='9')) == FALSE ) return FALSE;
	}

	return TRUE;
}
