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

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 120
#endif

#define CLIENTS 1

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
			break;
		}
		default: {
			printf("Usage: %s -a [ip address] -p [port] -t [tcp/udp]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		}	
	}

	if ((host_addr.sin_port == 0) || (host_addr.sin_addr.s_addr == 0)) {
		printf("Usage: %s -a [ip address] -p [port] -t [tcp/udp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/*Create IPC*/
	int msgid = msgget(IPC_PRIVATE, 0666);
	if (msgid == -1)
		handle_error("cannot create msgque");

	/*start routines*/
	 //will be filled later, will be used after routine rec msg from que
	pthread_t daemon;
	for (int i = 0; i < CLIENTS; ++i) {
		if (pthread_create(&daemon, NULL, serve_forever, &msgid) != 0)
			handle_error("invalid thread");
	}

	struct msg_type msg;
	memset(&msg, 0, sizeof(msg));
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
		fprintf(stderr, "Start listening");
		
		while (1) {
			int new_connect = accept(sockd, NULL, NULL);
			if (new_connect == -1) {
				fprintf(stderr, "WARNING: lost connection");	
				break;
			}
			fprintf(stderr, "Connect accepted");
			msg.type = TCP_REQUEST;
			memcpy(&msg.data, &new_connect, sizeof(new_connect));
			msgsnd(msgid, &msg, sizeof(msg), 0);
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

		while(1) {
			int* sock_p = (int*)&msg.data;
			struct sockaddr_in* peer_p = (struct sockaddr_in*)((char*)(&msg.data) + sizeof(int));
			char* buffer_p = (char*)((char*)&msg.data + sizeof(int) + sizeof(struct sockaddr_in));

			struct sockaddr_in peer;
			int sockadr_l = sizeof(struct sockaddr_in);
			char buffer_in[BUFFER_SIZE];
			memset(buffer_in, 0, BUFFER_SIZE);

			recvfrom (
				sockd, 
				buffer_in, 
				BUFFER_SIZE, 
				0, 
				(struct sockaddr*)&peer, 
				(socklen_t*)&sockadr_l
			);
			long udp = UDP_REQUEST;
			
			memcpy(&msg.type, &udp, sizeof(long));
			memcpy(sock_p, &sockd, sizeof(sockd));
			memcpy(peer_p, &peer, sizeof(peer));
			memcpy(buffer_p, &buffer_in, BUFFER_SIZE - sizeof(peer) - sizeof(sockd));

			if (msgsnd(msgid, &msg, BUFFER_SIZE + sizeof(long), 0) != 0)
				handle_error("msg send");
			memset(&msg, 0, sizeof(msg));
		}

	}
	default:
		handle_error("invalid protocol");
	}

	return 0;
}