#include "routine.h"

void * serve_forever(void* args) {
	while(ALWAYS) {
		routine(args);
	}
}

void routine(void* args) {
	struct msg_type msg;
	struct sockaddr_in peer;
	char buf[BUFFER_SIZE];
	int sockd;
	memset(buf, 0, sizeof(buf));
	memset(&msg, 0, sizeof(msg));

	/*WAIT_MSG*/
	msgrcv(*((int*)args), &msg, sizeof(msg), 0, 0);
	switch(msg.type) {
	case UDP_REQUEST: ;		
		
		/* UDP msg format:
		 * long type == UDP_REQUEST
		 * int sock
		 * sockaddr_in peer
		 * char* msg
		 */
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
		/*UDP recieved, --> WORK*/

		into_orcish(buf);

		/*SEND*/
		sendto(sockd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&peer, sizeof(peer));
		break;

	case TCP_REQUEST:	
		/* TCP msg format:
		 * long type == TCP_REQUEST
		 * int sock
		 */
		memcpy(&sockd, &msg.data, sizeof(sockd));

		/*TCP recieved --> WAIT_CLIENT*/
		while(ALWAYS) {
			recv(sockd, buf, BUFFER_SIZE, 0);
			if (strcmp(TERM_SEQ, buf) == 0)
				/*TERM_SEQ recieved, stop*/
				break;
			else {
				/*msg recieved --> WORK*/
				into_orcish(buf);
				/*SEND*/
				send(sockd, buf, BUFFER_SIZE, 0);
				memset(buf, 0, BUFFER_SIZE);
			}
		}

	}
}




