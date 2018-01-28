
#ifndef amos2ascii_h
#define amos2ascii_h

// enums are used to format text, by know the last command we know if need a space or not between,

enum
{
	is_newline_type = 0,
	is_command_type,
	is_symbol_type,
	is_number_type,
	is_string_type,
	is_label_type,
	is_var_type,
	is_procedure_type,
	is_commandDivider_type
};

extern unsigned short last_token_is ;
#endif
