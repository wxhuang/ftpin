#include "ftpin_config.h"
#include "ftpin_server.h"
#include "ftpin_cmd.h"

typedef struct _ftpin_server_handle
{
	int conn_cnts;
}ftpin_server_handle;

typedef struct _ftpin_server_state
{
	int is_running;
	int is_login;
	int is_pasv;
	int conn_sock;
	int data_sock;
	int pasv_sock;

	ftpin_server_handle* hndl;
	ftpin_cmd_t cmd;
	char* data_buf;
	char cmd_buf[512];
}ftpin_server_state;

/************************************************************/
#define MAX_CONN_CNTS 1
/************************************************************/
static ftpin_server_handle svr_instance;
/************************************************************/
static int ftpin_create_listen_socket(int port);
static int ftpin_accept_conn(int sock);

static void ftpin_server_task(void* state);
static int ftpin_server_task_create(int sock, ftpin_server_handle* hndl);
static void ftpin_response(ftpin_server_state* state, ftpin_cmd_t* cmd);
/************************************************************/

int ftpin_server_init(int port)
{
	int sock = ftpin_create_listen_socket(port);
	struct sockaddr_in client_addr;
	int len = sizeof(client_addr);
	int conn;
	ftpin_server_handle* hndl = &svr_instance;
	while(1)
	{
		conn = ftpin_accept(sock, (struct sockaddr*)&client_addr, &len);
		if(conn >= 0)
		{
			ftpin_server_task_create(conn, hndl);
		}else
		{
			ftpin_debug("accept failed\n");
		}
	}
	return 0;
}
static int ftpin_create_listen_socket(int port)
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

static int ftpin_server_task_create(int sock, ftpin_server_handle* hndl)
{
	ftpin_server_state* state;
	if(hndl->conn_cnts >= MAX_CONN_CNTS)
	{
		ftpin_debug("too much connections\n");
		ftpin_send_msg(sock, "421 too much connections,exit");
		ftpin_close(sock);
		return -1;
	}
	ftpin_debug("server_task_create\n");
	hndl->conn_cnts += 1;
	state = ftpin_malloc(sizeof(ftpin_server_state));
	memset(state, 0, sizeof(ftpin_server_state));
	state->is_running = 1;
	state->conn_sock = sock;
	state->data_sock = -1;
	state->pasv_sock = -1;
	state->hndl = hndl;
	ftpin_server_task(state);
	return 0;
}

static void ftpin_server_task(void* pstate)
{
	ftpin_server_state* state = pstate;
	ftpin_cmd_t* cmd = &(state->cmd);
	ftpin_send_msg(state->conn_sock, "220 welcome, connection will be closed without any operation in 30 seconds.");
	while(state->is_running)
	{
		if(ftpin_recv_msg(state->conn_sock, cmd))
			break;
		ftpin_response(state, cmd);
	}
	ftpin_close(state->conn_sock);
	ftpin_close(state->pasv_sock);
	ftpin_close(state->data_sock);
	state->hndl->conn_cnts -= 1;
	ftpin_free(state);
	return;
}
/*****************************************************************/
typedef void (*cmd_handle)(ftpin_server_state*, ftpin_cmd_t*);
static void ftpin_gen_port(unsigned short* porth, unsigned short* portl)
{
	srand(time(NULL));
	*porth = 128 + (rand() % 64);
	*portl = rand() % 0xff;
}
static void ftpin_get_host_ip(int sock, unsigned char* ip)
{
	int host, i;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;
	ftpin_getsockname(sock, (struct sockaddr*)&addr, &addr_size);
	host = (addr.sin_addr.s_addr);
	for(i = 0; i < 4; i++)
		ip[i] = (host >> i * 8)&0xff;
}
/*****************************************************************/

static void ftpin_cmd_user(ftpin_server_state* state, ftpin_cmd_t* cmd){
	ftpin_debug("%s\n", __FUNCTION__);
	if(!strcmp(cmd->args, "anonymous"))
	{
		state->is_login = 1;
		ftpin_send_msg(state->conn_sock, "230 login");
	}else
	{
		ftpin_send_msg(state->conn_sock, "530 invalid user name");
	}
}

static void ftpin_cmd_quit(ftpin_server_state* state, ftpin_cmd_t* cmd){
	state->is_running = 0;
	ftpin_debug("%s\n", __FUNCTION__);
	ftpin_send_msg(state->conn_sock, "221 bye");
}

static void ftpin_cmd_port(ftpin_server_state* state, ftpin_cmd_t* cmd){
	ftpin_debug("%s\n", __FUNCTION__);
	if(!state->is_login){
		ftpin_send_msg(state->conn_sock, "530 pls login");
		return;
	}
	ftpin_send_msg(state->conn_sock, "502 pls use PASV mode instead");
}

static void ftpin_cmd_pasv(ftpin_server_state* state, ftpin_cmd_t* cmd){
	ftpin_debug("%s\n", __FUNCTION__);
	if(!state->is_login){
		ftpin_send_msg(state->conn_sock, "530 pls login");
		return;
	}
	int pasv_sock;
	unsigned short porth, portl;
	unsigned char ip[4];
	char buf[128];
	ftpin_gen_port(&porth, &portl);
	pasv_sock = ftpin_create_listen_socket(porth << 8 | portl);
	if(pasv_sock < 0){
		ftpin_send_msg(state->conn_sock, "502 pasv socket create error");
		return;
	}
	state->is_pasv = 1;
	ftpin_get_host_ip(state->conn_sock, ip);
	ftpin_close(state->pasv_sock);
	state->pasv_sock = pasv_sock;
	sprintf(buf, "227 pasv (%d,%d,%d,%d,%d,%d)", ip[0], ip[1], ip[2], ip[3], porth, portl);
	ftpin_send_msg(state->conn_sock, buf);
	return;
}

static void ftpin_cmd_type(ftpin_server_state* state, ftpin_cmd_t* cmd){
	ftpin_debug("%s\n", __FUNCTION__);
	if(!state->is_login){
		ftpin_send_msg(state->conn_sock, "530 pls login");
		return;
	}
	switch(cmd->args[0]){
		case 'A':
			ftpin_send_msg(state->conn_sock, "200 ok");
			break;
		default:
			ftpin_send_msg(state->conn_sock, "504 unspported type");
			break;
	}
}

static void ftpin_cmd_mode(ftpin_server_state* state, ftpin_cmd_t* cmd){
	ftpin_debug("%s\n", __FUNCTION__);
	ftpin_send_msg(state->conn_sock, "500 unsupported command");
}

static void ftpin_cmd_stru(ftpin_server_state* state, ftpin_cmd_t* cmd){
	ftpin_debug("%s\n", __FUNCTION__);
	ftpin_send_msg(state->conn_sock, "500 unsupported command");
}

static void ftpin_cmd_retr(ftpin_server_state* state, ftpin_cmd_t* cmd){
	ftpin_debug("%s\n", __FUNCTION__);
	ftpin_send_msg(state->conn_sock, "500 unsupported command");
}

static void ftpin_cmd_stor(ftpin_server_state* state, ftpin_cmd_t* cmd){
	ftpin_debug("%s\n", __FUNCTION__);
	if(!state->is_login){
		ftpin_send_msg(state->conn_sock, "530 pls login");
		return;
	}
	if(!state->is_pasv){
		ftpin_send_msg(state->conn_sock, "502 pls use pasv mode");
		return;
	}
	char buf[1024];
	int bytes_read;
	int data_sock = ftpin_accept_conn(state->pasv_sock);
	if(data_sock < 0){
		ftpin_send_msg(state->conn_sock, "425 error data connection");
		return;
	}
	ftpin_close(state->pasv_sock);
	state->pasv_sock = -1;
	state->data_sock = data_sock;
	ftpin_send_msg(state->conn_sock, "150 data connection opened");
	/* TODO:file processing. */
	do{
		bytes_read = ftpin_recv(data_sock, buf, 1023, 0);
		if(bytes_read <= 0)
			break;
		buf[bytes_read] = 0;
		printf("%s", buf);
	}while(1);
	ftpin_close(data_sock);
	state->data_sock = -1;
	ftpin_send_msg(state->conn_sock, "226 file send success");
}

static void ftpin_cmd_noop(ftpin_server_state* state, ftpin_cmd_t* cmd){
	ftpin_debug("%s\n", __FUNCTION__);
	ftpin_send_msg(state->conn_sock, "200 ok");
}
static void ftpin_cmd_err_param(ftpin_server_state* state, ftpin_cmd_t* cmd){
	ftpin_debug("%s\n", __FUNCTION__);
	ftpin_send_msg(state->conn_sock, "501 error params");
}
static void ftpin_cmd_unknown(ftpin_server_state* state, ftpin_cmd_t* cmd){
	ftpin_debug("%s\n", __FUNCTION__);
	ftpin_send_msg(state->conn_sock, "500 unknown command");
}
static cmd_handle cmd_hndl_map[] = 
{
	ftpin_cmd_user,
	ftpin_cmd_quit,
	ftpin_cmd_port,
	ftpin_cmd_pasv,
	ftpin_cmd_type,
	ftpin_cmd_mode,
	ftpin_cmd_stru,
	ftpin_cmd_retr,
	ftpin_cmd_stor,
	ftpin_cmd_noop,
	ftpin_cmd_unknown,
	ftpin_cmd_err_param,
};
static void ftpin_response(ftpin_server_state* state, ftpin_cmd_t* cmd)
{
	if(cmd->idx <= FTPIN_CMD_STAT_NUM)
		cmd_hndl_map[cmd->idx](state, cmd);
}
