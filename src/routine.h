#ifndef __ROUTINE_H__
#define __ROUTINE_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "orcish.h"

#define UDP_REQUEST 22
#define TCP_REQUEST 23

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 150
#endif

#define TERM_SEQ "exit"
#define ALWAYS 1

#define handle_error(msg) \
    do { fprintf(stderr, msg); exit(EXIT_FAILURE); } while (0)

typedef struct msg_type{
	long type;
	char data[BUFFER_SIZE];
} msg_type;

typedef enum {tcp, udp} proto_t;

void * serve_forever(void* args);

static void routine(void* args);

#endif