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
	for(i = 0; i < FTPIN_CMD_UNKNOWN; i++)
	{
		len = strlen(ftpin_cmd_tab[i]);
		if(!strncmp(ftpin_cmd_tab[i], str, len))
			break;
	}
	if(FTPIN_CMD_UNKNOWN != i)
	{
		p = str + len;
		if(*p)
		{
			p++;
			if(strlen(p) > 255)
				i = FTPIN_CMD_ERR_PARAM;			//arguments too long
			else
				strcpy(cmd->args, p);
		}
	}
	cmd->idx = i;
	return 0;				//success
}

int ftpin_send_msg(int sock, char* str)
{
	char buf[128];
	sprintf(buf, "%s%s", str, "\r\n");
	return ftpin_send(sock, buf, strlen(buf), 0);
}

int ftpin_recv_msg(int sock, ftpin_cmd_t* tar)
{
	int bytes_read;
	char buf[512];
	char* p;
	struct timeval timeout = {CMD_CONN_TIMEOUT, 0};
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock, &set);
	if(select(sock + 1, &set, NULL, NULL, &timeout) > 0)
	{
		bytes_read = ftpin_recv(sock, buf, 512, 0);
		if(bytes_read <= 0)
			return -1;
		p = strstr(buf, "\r\n");
		*p = 0;
		ftpin_debug("\nread %d bytes\nrecv: %s\n", bytes_read, buf);
		return ftpin_parse_cmd(tar, buf);
	}else
	{
		//timeout
		ftpin_debug("timeout\n");
		ftpin_send_msg(sock, "421 timeout.");
		return -2;
	}
}
