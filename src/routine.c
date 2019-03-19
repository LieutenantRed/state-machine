#include "routine.h"

void * serve_forever(void* args) {
	state_t STATE;

	struct msg_type msg;
	struct sockaddr_in peer;
	char buf[BUFFER_SIZE];
	int sockd;

	while(ALWAYS) {	
		switch(STATE) {

		case WAIT_QUEUE:
			memset(&msg, 0, sizeof(msg));
			msgrcv(*((int*)args), &msg, sizeof(msg), 0, 0);
			STATE = MSG_RCV;
			break;

		case MSG_RCV:
			switch(msg.type) {
			case UDP_REQUEST:
				memset(buf, 0, BUFFER_SIZE);
				memcpy(&sockd, &msg.data, sizeof(sockd));
				memcpy(&peer, (char*)&msg.data + sizeof(sockd), sizeof(peer));
				memcpy(
					buf, 
					(char*)&msg.data + sizeof(sockd) + sizeof (peer), 
					BUFFER_SIZE - sizeof(sockd) - sizeof (peer)
				);
				STATE = WORK;
				break;

			case TCP_REQUEST:
				memcpy(&sockd, &msg.data, sizeof(sockd));
				STATE = WAIT_CLIENT;
				break;
			}
			break;

		case WAIT_CLIENT:
			memset(buf, 0, BUFFER_SIZE);
			recv(sockd, buf, BUFFER_SIZE, 0);
			STATE = WORK;
			break;

		case WORK:
			if (strcmp(TERM_SEQ, buf) == 0) {
				/*TERM_SEQ recieved, stop*/
				STATE = WAIT_QUEUE;
				break;
			}
			into_orcish(buf);
			STATE = SEND;
			break;
			
		case SEND:
			switch(msg.type) {
			case UDP_REQUEST:
				sendto(sockd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&peer, sizeof(peer));
				STATE = WAIT_QUEUE;
				break;

			case TCP_REQUEST:
				send(sockd, buf, BUFFER_SIZE, 0);
				STATE = WAIT_CLIENT;
				break;
			}
		}
	}
}





