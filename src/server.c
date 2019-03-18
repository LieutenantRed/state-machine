#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <pthread.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "orcish.h"
#include "routine.h"

#ifndef BUF_SIZE
#define BUF_SIZE 120
#endif

#define CLIENTS 10

int main(int argc, char** argv) {
	struct sockaddr_in host_addr;
	memset(&host_addr, 0, sizeof(host_addr));
	host_addr.sin_family = AF_INET;
	proto_t protocol;

	/*recieve options for server*/
	int opt;
	while ((opt = getopt(argc, argv, "a:p:t:")) != -1) {
		switch(opt) {
		case 'a': {
			if (inet_aton(optarg, &host_addr.sin_addr) == 0) 
				handle_error("invalid address");
			break;
		}
		case 'p': {
			host_addr.sin_port = htons(atoi(optarg));
			break;
		}
		case 't': {
			if (strcmp(optarg, "tcp") == 0)
				protocol = tcp;
			if (strcmp(optarg, "udp") == 0)
				protocol = udp;	
		}
		default: {
			printf("Usage: %s -a [ip address] -p [port] -t [tcp/udp]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		}	
	}

	if ((host_addr.sin_port == 0) || (host_addr.sin_addr.s_addr == 0)) {
		printf("Usage: %s -a [ip address] -p [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/*Create IPC*/
	int msgid = msgget(IPC_PRIVATE, 0666);
	if (msgid == -1)
		handle_error("cannot create msgque");
	
	struct udpthread_routine_info info;
	info.sockd = sockd;
	info.msgid = msgid;

	/*start routines*/
	 //will be filled later, will be used after routine rec msg from que
	pthread_t daemon;
	for (int i = 0; i < CLIENTS; ++i) {
		if (pthread_create(&daemon, NULL, serve_forever, &msgid) != 0)
			handle_error("invalid thread");
	}

	struct msg_type msg;
	memset(&msg_snd, 0, sizeof(msg_snd));
	int sockd;
	switch(protocol) {
	case tcp: {
		/* TCP msg:
		 * long type == TCP_REQUEST
		 * int sock
		 */
		sockd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockd == -1)
			handle_error("invalid socket");

		if (bind(sockd, (struct sockaddr*)&host_addr, sizeof(host_addr)) == -1)
			handle_error("invalid address"); 

		listen(sockd, CLIENTS);
		
		while (1) {
			int new_connect = accept(sockd, NULL, NULL)
			if (new_connect == -1) {
				fprintf("WARNING: lost connection");	
				break;
			}
			msg.type = TCP_REQUEST;
			memcpy(&msg.data, &new_connect, sizeof(new_connect));
			msgsnd(msgid, &msg, BUF_SIZE + sizeof(long), 0);
		}
	}
	case udp: {
		/* UDP msg:
		 * long type == UDP_REQUEST
		 * int sock
		 * sockaddr_in peer
		 * char* msg
		 */
		sockd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockd == -1)
			handle_error("invalid socket");

		if (bind(sockd, (struct sockaddr*)&host_addr, sizeof(host_addr)) == -1)
			handle_error("invalid address"); 

		int sockadr_l = sizeof(host_addr);

		while(1) {
			msg.type = UDP_REQUEST;
			memcpy(&msg.data, &sockd, sizeof(sockd));

			recvfrom (
				sockd, 
				// data placed after sockaddr struct and socket descriptor 
				&msg.data + sockadr_l + sieof(sockd), 
				BUF_SIZE, 
				0, 
				(struct sockaddr*)(&(msg.data) + sizeof(sockd)), 
				(socklen_t*)&sockadr_l
			);
			msgsnd(msgid, &msg, BUF_SIZE + sizeof(long), 0);
			memset(&msg, 0, sizeof(msg));
		}

	}
	default:
		handle_error("invalid protocol");
	}

	return 0;
}