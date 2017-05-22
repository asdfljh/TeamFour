#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

int getFile(char* filePath, char* fileContents) {
	FILE* fp = fopen(filePath, "rb");
	int size;	
	size = fread(fileContents, 1, 65536, fp);
	fclose(fp);

	return size;
}

int main(int argc, char** argv) {
	int client_socket;
	struct sockaddr_in server_address;
	char fileContents[65536];
	char sendBuffer[65536];
	char buffer[65536];
	int buffer_length;
	int fileSize=0;
	int totalSize=0;
	int tmpSize=0;

	char jsonName[20] = "{\"name\": \"";
	char jsonBody[20] = "\", \"body\": \"";
	char jsonEnd[20] = "\"}";

	if (argc != 3) {
		printf("Usage: %s <ip> <port>\n", argv[0]);
		exit(1);
	}

	client_socket = socket(PF_INET, SOCK_STREAM, 0);

	if (client_socket == -1) {
		fprintf(stderr, "socket() error\n");
		exit(1);
	}

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(argv[1]);
	server_address.sin_port = htons(atoi(argv[2]));

	if (connect(client_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
		fprintf(stderr, "connect() error\n");
		exit(1);
	}

//	while (1) {
		memset(fileContents, 0, 65536);
		
		printf("name: ");
//		scanf("%s", buffer);
		strcpy(buffer, "hello");
		tmpSize = strlen(jsonName);
		memcpy(sendBuffer, jsonName, tmpSize);
		totalSize += tmpSize;

		tmpSize = strlen(buffer);
		memcpy(sendBuffer+totalSize, buffer, tmpSize);
		totalSize += tmpSize;

		tmpSize = strlen(jsonBody);
		memcpy(sendBuffer+totalSize, jsonBody, tmpSize);
		totalSize += tmpSize;

		memset(buffer, 0, sizeof(buffer));
		printf("filePath: ");
//		scanf("%s", buffer);
		strcpy(buffer, "helloWorld.gpg.base");
		fileSize = getFile(buffer, fileContents);

		tmpSize = strlen(fileContents);
		memcpy(sendBuffer+totalSize, fileContents, tmpSize);
		totalSize += tmpSize;

		tmpSize = strlen(jsonEnd);
		memcpy(sendBuffer+totalSize, jsonEnd, tmpSize);
		totalSize += tmpSize;

		printf("You: ");
		printf("%s, %d\n", sendBuffer, sizeof(sendBuffer));
		
		write(client_socket, sendBuffer, totalSize);
		
		memset(buffer, 0, sizeof(buffer));
		memset(sendBuffer, 0, sizeof(sendBuffer));

//	}
	close(client_socket);

	return 0;
}
