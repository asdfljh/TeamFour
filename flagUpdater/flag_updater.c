#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <gpgme.h>
#include <locale.h> 
#include <errno.h>

int BUFSIZE = 5120;

void daemonize(void)
{
	pid_t pid, sid;
	pid = fork();

	if(pid < 0)
		exit( EXIT_FAILURE );
	else if(pid > 0)
		exit( EXIT_SUCCESS );

	umask(0);

	sid = setsid();

	if(sid < 0)
	{
		perror("daemonize::sid");
		exit( EXIT_FAILURE );
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

int verify_signature(char buf[BUFSIZE]){
	FILE *fp, *sed;
	char path[BUFSIZE];

	// Import GPG Keys
	fp = popen("gpg --import gpg_public_key/*.pub", "r"); //import all public keys
	fgets(path, BUFSIZE, fp);
	pclose(fp);

	// GPG Decryption

	fp = fopen("received_encrypt.flag.gpg", "w+");
	fprintf(fp, "%s", buf);
	fclose(fp);

	fp = popen(" gpg --decrypt received_encrypt.flag.gpg >> verif.flag", "r"); // TODO : to test use test/test.c.gpg instead of received_encrypt.flag.gpg
	fgets(path, BUFSIZE, fp);
	pclose(fp);

	// Extract data from the json file 

	const char s[2] = "\"";
	char *token;
	char signature[BUFSIZE];
	char githubID[BUFSIZE];
	char newflag[BUFSIZE];
	char carac;
	char line[BUFSIZE];
	int i = 0;

	sed = fopen("verif.flag", "r");

	while (fgets(line, sizeof(line), sed) != NULL){
		if(line != "\n"){
			line[strlen(line) - 1] = '\0';

			char *data = strdup(line);

			token = strtok(data, s);
			while( token != NULL ){
				i++;
				if(i == 5){
					strcpy(githubID, token);
				}
				if(i == 10){
					strcpy(newflag, token);
				}
				if(i == 15){
					strcpy(signature, token);	
				}
				token = strtok(NULL, s);
			}
		}
	}

	fclose(sed);

	printf("Github ID : %s\n", githubID);
        printf("New Flag : %s\n", newflag);
	printf("Signature : %s\n", signature);

	/* Verify the signature */

	char check_signature[BUFSIZE];
	char *decode64_signature;

	strcpy(check_signature, githubID);
	strcat(check_signature, ":");
	strcat(check_signature, newflag);

	puts(check_signature);

	//Decode64 the signature
	fp = fopen("signature_encrypt64.flag", "w+"); //encrypt64.flag contains the signature 64-encoded and signed by one of the TA
        fprintf(fp, "%s", signature);
        fclose(fp);

	fp = popen("python flagUpdater/script_64.py >> signature_decrypt64.flag.gpg", "r"); //decode_64.flag.gpg contains the signature signed by one of the TA
        fgets(path, BUFSIZE, fp);
        pclose(fp);

	//GPG Decryption
	fp = popen("gpg --decrypt signature_decrypt64.flag.gpg >> signature_decrypted.flag", "r"); //decode_64.flag contains the final signature unsigned 
        fgets(path, BUFSIZE, fp);
        pclose(fp);


	//Check the signatures
	char chain[BUFSIZE];
        char final[BUFSIZE];

	sed = fopen("signature_decrypted.flag", "r");
	while (fgets(chain, BUFSIZE, sed) != NULL){
            strcpy(final, chain);
        }
	puts("Decode_64 sign");
	chain[strlen(chain) - 1] = '\0';
	puts(chain);
        fclose(sed);

	int ret = strcmp(chain, check_signature);
	if(ret != 0)
		return -1;	

	return 0;

}

void listen_client(){
	FILE *fp;
	int parentfd; /* parent socket */
	int childfd; /* child socket */
	int clientlen; /* byte size of client's address */
	struct sockaddr_in serveraddr; /* server's addr */
	struct sockaddr_in clientaddr; /* client addr */
	struct hostent *hostp; /* client host info */
	char buf[BUFSIZE]; /* message buffer */
	char *hostaddrp; /* dotted decimal host addr string */
	int optval; /* flag value for setsockopt */
	int n; /* message byte size */
	int portno = 42;
	char path[BUFSIZE];

	// Create the parent socket 
	parentfd = socket(AF_INET, SOCK_STREAM, 0);
	if (parentfd < 0)
		perror("ERROR opening socket");

	optval = 1;
	setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR,
			(const void *)&optval , sizeof(int));

	// Build the server's Internet address 
	bzero((char *) &serveraddr, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);

	// Associate the parent socket with a port 
	if (bind(parentfd, (struct sockaddr *) &serveraddr,
				sizeof(serveraddr)) < 0)
		perror("ERROR on binding");

	// Make this socket ready to accept connection requests 
	if (listen(parentfd, 5) < 0) // allow 5 requests to queue up
		perror("ERROR on listen");

	// Wait for a connection request, echo input line, then close connection 
	clientlen = sizeof(clientaddr);
	while (1) {

		// Wait for a connection request 
		childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
		if (childfd < 0)
			perror("ERROR on accept");

		// Determine who sent the message 
		hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
				sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		if (hostp == NULL)
			perror("ERROR on gethostbyaddr");
		hostaddrp = inet_ntoa(clientaddr.sin_addr);
		if (hostaddrp == NULL)
			perror("ERROR on inet_ntoa\n");
		printf("server established connection with %s (%s)\n",
				hostp->h_name, hostaddrp);

		// Read input string from the client 
		bzero(buf, BUFSIZE);
		n = read(childfd, buf, BUFSIZE);
		if (n < 0)
			perror("ERROR reading from socket");
		printf("server received %d bytes: %s", n, buf);

		if(verify_signature(buf) == -1){
			perror("The signature of the file isn't good\n");
		}else{
			//Updates the content of the file
			fp = popen("mv verif.flag /var/ctf/; mv /var/ctf/verif.flag /var/ctf/notary.flag", "r");
			fgets(path, BUFSIZE, fp);
			pclose(fp);
		}

		fp = popen("rm *.flag *.gpg", "r");
                fgets(path, BUFSIZE, fp);
                pclose(fp);

		close(childfd);
	}
}

int main(void)
{
	daemonize();
	listen_client();	
	return( EXIT_SUCCESS );
}

