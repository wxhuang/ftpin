#include "ftpin_config.h"
#include "ftpin_server.h"
#include "ftpin_cmd.h"

typedef struct _ftpin_server_state
{
	int conn_sock;
	int data_sock;
	int pasv_sock;

	ftpin_cmd_t cmd;
	char* data_buf;
	char cmd_buf[512];
}ftpin_server_state;

/************************************************************/
static int ftpin_create_socket(int port);
static int ftpin_accept_conn(int sock);

static void ftpin_server_task(ftpin_server_state* state);
static int ftpin_server_task_create(int sock);
/************************************************************/

int ftpin_server_init(int port)
{
	return 0;
}
static int ftpin_create_socket(int port)
{
	int sock;
	int reuse = 1;
	struct sockaddr_in srv_addr = (struct sockaddr_in){
		AF_INET,
		htons(port),
		(struct in_addr){INADDR_ANY}
	};
	sock = ftpin_socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		ftpin_debug("socket create failed\n");
		return -1;
	}
	ftpin_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if(ftpin_bind(sock, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0)
	{
		ftpin_debug("bind falied\n");
		return -2;
	}
	ftpin_listen(sock, 1);
	return sock;
}

static int ftpin_accept_conn(int sock)
{
	struct sockaddr_in client_addr;
	int addrlen = sizeof(client_addr);
	return ftpin_accept(sock, (struct sockaddr*)&client_addr, &addrlen);
}
