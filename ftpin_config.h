#ifndef _FTPIN_CONFIG_H_
#define _FTPIN_CONFIG_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include <pthread.h>

#define ftpin_malloc(a) malloc(a)
#define ftpin_free(a) free(a)


#define ftpin_socket(a,b,c) socket(a,b,c)
#define ftpin_setsockopt(a,b,c,d,e) setsockopt(a,b,c,d,e)
#define ftpin_bind(a,b,c) bind(a,b,c)
#define ftpin_listen(a,b) listen(a,b)
#define ftpin_accept(a,b,c) accept(a,b,c)
#define ftpin_send(a,b,c,d) send(a,b,c,d)
#define ftpin_recv(a,b,c,d) recv(a,b,c,d)
#define ftpin_close(a) close(a)
#define ftpin_getsockname(a,b,c) getsockname(a,b,c)
#define ftpin_connect(a,b,c) connect(a,b,c)

#define ftpin_debug printf

#endif
