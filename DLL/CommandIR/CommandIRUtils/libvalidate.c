#include "libvalidate.h"
#include "libtcp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int is_numeric_digit(char byte);
int is_hex_digit(char byte);

extern unsigned char standalone;
void sendErrorString_tcp(unsigned char * sendDat, int bytes, struct tcp_server_conn *p);
void sendErrorString_pipe(unsigned char * sendDat, int bytes);

int redirect_error_type = ERRORS_CONSOLE;
void * redirect_error_specific_device = 0;

void redirect_errors(int device_type, void * specific_device)
{
  redirect_error_type = device_type;
  redirect_error_specific_device = specific_device;
}

char errorBuffer[1024];

/* must have zero-terminated error strings */
void send_error(int type)
{
  char * strError = "ERROR: \0";
  char * strOk = "OK: \0";
  
  switch(redirect_error_type)
  {
    case ERRORS_CONSOLE:
      if(type == SEND_ERROR)
        printf("ERROR: %s\n", errorBuffer);
      else
        printf("OK: %s\n", errorBuffer);
      break;
    case ERRORS_PIPE:
      if(type == SEND_ERROR)
        sendErrorString_pipe(strError, strlen(strError));
      else
        sendErrorString_pipe(strOk, strlen(strOk));
        
      sendErrorString_pipe(errorBuffer, strlen(errorBuffer));
      break;
    case ERRORS_TCP:
      if(type == SEND_ERROR)
        sendErrorString_tcp(strError, strlen(strError), (struct tcp_server_conn *)redirect_error_specific_device);
      else
        sendErrorString_tcp(strOk, strlen(strOk), (struct tcp_server_conn *)redirect_error_specific_device);
        
      sendErrorString_tcp(errorBuffer, strlen(errorBuffer), (struct tcp_server_conn *)redirect_error_specific_device);
      break;
  }
}


void raise_error(char * s)
{
  strcpy(errorBuffer, s);
  send_error(SEND_ERROR);
}

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
	sprintf(errorBuffer, "Invalid option: %s\n\0", errmsg);
	send_error(SEND_ERROR);
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
			sprintf(errorBuffer, "%s\n  (E) Invalid hex digit '%c'\n", 
				errhelp, 
				p[x]
				);
			send_error(SEND_ERROR);
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
			sprintf(errorBuffer, "%s\n  (E) Invalid digit '%c'\n", 
				errhelp, 
				p[x]
				);
			send_error(SEND_ERROR);
			return exit_or_error();
		}
	}
	return 0;
}

// [0] is '-', [1] is letter, validation start at [2]
int validate_string_parameter(char * p, char * errhelp)
{
	if(strlen(p) < 3) {
		sprintf(errorBuffer, "%s\n  (E) Missing string after -%c\n", errhelp, p[1]);
		send_error(SEND_ERROR);
		return exit_or_error();
	}
	
	// Just verify that the first parameter is a char; the string can have number
	if(!is_char(p[2]))
	{
		sprintf(errorBuffer, "%s\n  (E) String required - String must begin with an alphabetic character,\n      and '%c' is not a string.\n", errhelp, p[2]);
		send_error(SEND_ERROR);
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

