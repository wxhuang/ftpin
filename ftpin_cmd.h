#ifndef _FTPIN_CMD_H_
#define _FTPIN_CMD_H_

#include "ftpin_config.h"
typedef enum
{
	USER = 0,
	QUIT,
	PORT,
	PASV,
	TYPE,
	MODE,
	STRU,
	RETR,
	STOR,
	NOOP,
	FTPIN_CMD_MAX,
}FTPIN_COMMAND;

typedef struct _ftpin_cmd_t
{
	FTPIN_COMMAND idx;
	char args[256];
}ftpin_cmd_t;

int ftpin_parse_cmd(ftpin_cmd_t* cmd, char* str);



#endif
