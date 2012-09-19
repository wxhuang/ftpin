#ifndef _FTPIN_CMD_H_
#define _FTPIN_CMD_H_

#include "ftpin_config.h"

#define CMD_CONN_TIMEOUT 300
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
	FTPIN_CMD_UNKNOWN,
	FTPIN_CMD_ERR_PARAM,
	FTPIN_CMD_STAT_NUM,
}FTPIN_COMMAND;

typedef struct _ftpin_cmd_t
{
	FTPIN_COMMAND idx;
	char args[256];
}ftpin_cmd_t;

int ftpin_parse_cmd(ftpin_cmd_t* cmd, char* str);
int ftpin_recv_msg(int sock, ftpin_cmd_t* tar);
int ftpin_send_msg(int sock, char* str);


#endif
