int invalid_command_line_parameter_exit(char * errmsg);

int validate_hex_parameter(char * p, char * errhelp);
int validate_string_parameter(char * p, char * errhelp);

/* Shared input validation routines */
int validate_param_s(char * p);

void raise_error(char * s);

#define ERRORS_CONSOLE 0
#define ERRORS_PIPE 1
#define ERRORS_TCP 2

extern char errorBuffer[1024];
extern void send_error(int);

#define SEND_ERROR 0
#define SEND_OK 1

