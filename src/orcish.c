#include "orcish.h"

void into_orcish(char* src) {
	int len = strlen(src);
	if (len > 5) {
		src[0] = 'W';
		for (int i = 1; i < len - 4; ++i){
			src[i] = 'A';
		}
		src[len - 4] = 'G';
		src[len - 3] = 'H';
		src[len - 2] = '!';
		src[len - 1] = '\n';
	} else {
		memset(src, 0, BUF_SIZE);
		strcpy(src, "Unknown pattern\n");
	}		 
}