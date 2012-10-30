#include "ftpin_config.h"
#include "ftpin_cmd.h"

#define array_size(m) (sizeof(m) / sizeof(m[0]))
const char* ftpin_cmd_tab[] = 
{
	"USER",
	"QUIT",
	"PORT",
	"PASV",
	"TYPE",
	"MODE",
	"STRU",
	"RETR",
	"STOR",
	"NOOP"
};

int ftpin_parse_cmd(ftpin_cmd_t* cmd, char* str)
{
	FTPIN_COMMAND i;
	char len;
	char* p;
	for(i = 0; i < FTPIN_CMD_MAX; i++)
	{
		len = strlen(ftpin_cmd_tab[i]);
		if(!strncmp(ftpin_cmd_tab[i], str, len))
			break;
	}
	if(FTPIN_CMD_MAX == i)
		return -1;			//invalid command
	p = str + len + 1;
	if(strlen(p) > 255)
		return -2;			//arguments too long
	cmd->idx = i;
	strcpy(cmd->args, p);
	return 0;				//success
}

