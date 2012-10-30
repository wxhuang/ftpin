#include "ftpin_config.h"
#include "ftpin_cmd.h"
#include "ftpin_server.h"

int main(int argc, char** argv)
{
	ftpin_server_init(1096);
	return 0;
}
