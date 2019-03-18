#include "routine.h"

void * serve_forever(void* args) {
	while(ALWAYS) {
		routine(args);
	}
}

void routine(void* args) {
	state_t STATE = WAIT_QUEUE;

	struct msg_type msg;
	struct sockaddr_in peer;
	char buf[BUFFER_SIZE];
	int sockd;

	memset(buf, 0, sizeof(buf));
	memset(&msg, 0, sizeof(msg));

	msgrcv(*((int*)args), &msg, sizeof(msg), 0, 0);
	STATE = MSG_RCV;

	switch(msg.type) {
	case UDP_REQUEST: ;
		/* UDP msg format:
		 * long type == UDP_REQUEST
		 * int sock
		 * sockaddr_in peer
		 * char* msg
		 */
		STATE = WORK;
		memcpy(&sockd, &msg.data, sizeof(sockd));
		memcpy(&peer, (char*)&msg.data + sizeof(sockd), sizeof(peer));
		memcpy(
			buf, 
			(char*)&msg.data + sizeof(sockd) + sizeof (peer), 
			BUFFER_SIZE - sizeof(sockd) - sizeof (peer)
		);

		if (strcmp(TERM_SEQ, buf) == 0)
			/*TERM_SEQ recieved, stop*/
			break;
		
		into_orcish(buf);
		
		STATE = SEND;	
		sendto(sockd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&peer, sizeof(peer));
		break;

	case TCP_REQUEST: ;
		/* TCP msg format:
		 * long type == TCP_REQUEST
		 * int sock
		 */
		memcpy(&sockd, &msg.data, sizeof(sockd));

		STATE = WAIT_CLIENT;
		while(ALWAYS) {
			recv(sockd, buf, BUFFER_SIZE, 0);
			STATE = WORK;
			if (strcmp(TERM_SEQ, buf) == 0)
				/*TERM_SEQ recieved, stop*/
				break;
			else {
				/*msg recieved --> WORK*/
				into_orcish(buf);
				STATE = SEND;
				send(sockd, buf, BUFFER_SIZE, 0);
				memset(buf, 0, BUFFER_SIZE);
				STATE = WAIT_CLIENT;
			}
		}

	}
}




