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

char* PATH_FLAG = "/var/ctf/";
int PATH_MAX = 1024;
int BUFSIZE = 1024;

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

void build_decoding_table() {

    decoding_table = malloc(256);

    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}

unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length) {

    if (decoding_table == NULL) build_decoding_table();

    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}

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
	char path[PATH_MAX];
	char new_buf[BUFSIZE];
	int disc = 10;

	// Import GPG Keys
	fp = popen("gpg --import gpg_public_key/*.pub", "r"); //import all public keys
	fgets(path, PATH_MAX, fp);
	pclose(fp);

	// GPG Decryption

	//TODO : handle the problem brought by the miss of the caracter "^@" in buf...

	fp = fopen("encrypt.c.gpg", "w+");
	fprintf(fp, "%s", buf);
	fclose(fp);

	fp = popen(" gpg --decrypt test/test.c.gpg > verif.flag", "r"); //decrypt the file / TODO : change test/test.c.gpg by encrypt.c.gpg 
	fgets(path, PATH_MAX, fp);
	pclose(fp);

	// Extract data from the json file (decrypt.flag)

	const char s[2] = "\"";
	char *token;
	char signature[100];
	char githubID[100];
	char newflag[100];
	char carac;
	char line[100];
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
	printf("Signature : %s\n", signature);
	printf("New Flag : %s\n", newflag);

	/* Verify the signature */

	char check_signature[100];

	strcpy(check_signature, githubID);
	strcat(check_signature, ":");
	strcat(check_signature, newflag);

	puts(check_signature);

	//TODO : decode64 the signature, PGP verification and strcmp

	size_t *out_length;
	base64_decode(signature, strlen(signature), out_length);
	
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
	char path[PATH_MAX];

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
			fp = popen("mv verif.flag /var/ctf/; cp /var/ctf/verif.flag /var/ctf/notary.flag", "r");
			fgets(path, PATH_MAX, fp);
			pclose(fp);
		}
		close(childfd);
	}
}

int main(void)
{
	daemonize();
	listen_client();	
	return( EXIT_SUCCESS );
}

