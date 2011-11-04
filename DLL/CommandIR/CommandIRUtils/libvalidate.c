#include "libvalidate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int is_numeric_digit(char byte);
int is_hex_digit(char byte);

extern unsigned char standalone;

int is_char(char byte)
{
	if( ( (byte >= 'a') && (byte <= 'z') ) || ( (byte >= 'A') && (byte <= 'Z') ) )
		return 1;
	return 0;
}

int is_hex_digit(char byte)
{
	if( ( (byte >= 'a') && (byte <= 'f') ) || ( (byte >= 'A') && (byte <= 'F') ) )
		return 1;
	return is_numeric_digit(byte);
}

int is_numeric_digit(char byte)
{
	if( ( (byte >= '0') && (byte <= '9') ) )
		return 1;
	return 0;
}

int exit_or_error()
{
	if(standalone > 0)
		exit(-1);
	else
		return -1;
}

int invalid_command_line_parameter_exit(char * errmsg)
{
	printf("Invalid option: %s\n", errmsg);
	return exit_or_error();
}

int validate_hex_parameter(char * p, char * errhelp)
{
	int x;
	if(strlen(p) < 3)
		return invalid_command_line_parameter_exit(errhelp);
	
	for(x=2; x<strlen(p); x++)
	{
		if(!is_hex_digit(p[x])) {
			printf("%s\n  (E) Invalid hex digit '%c'\n", 
				errhelp, 
				p[x]
				);
			return exit_or_error();
		}
	}
	return 0;
}

int validate_numeric_parameter(char * p, char * errhelp)
{
	int x;
	if(strlen(p) < 3)
		return invalid_command_line_parameter_exit(errhelp);

	for(x=2; x<strlen(p); x++) {
		if(!is_numeric_digit(p[x])) {
			printf("%s\n  (E) Invalid digit '%c'\n", 
				errhelp, 
				p[x]
				);
			return exit_or_error();
		}
	}
	return 0;
}

// [0] is '-', [1] is letter, validation start at [2]
int validate_string_parameter(char * p, char * errhelp)
{
	if(strlen(p) < 3) {
		printf("%s\n  (E) Missing string after -%c\n", errhelp, p[1]);
		return exit_or_error();
	}
	
	// Just verify that the first parameter is a char; the string can have number
	if(!is_char(p[2]))
	{
		printf("%s\n  (E) String required - String must begin with an alphabetic character,\n      and '%c' is not a string.\n", errhelp, p[2]);
		return exit_or_error();
	}
}

// Common Input Validation routines
int validate_param_s(char * p)
{
  return validate_string_parameter(
  	p, 
  	"Name of CommandIR required for '-s' switch. Example -sCommandIR1"
  	);
}

