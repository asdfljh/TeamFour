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

char* PATH_FLAG = "/var/ctf";
int PATH_MAX = 1024;
int BUFSIZE = 1024;

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

	if(chdir(PATH_FLAG) < 0)
	{
		perror("daemonize::chdir");
		exit( EXIT_FAILURE );
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

void listen_client(){
	FILE *fp;

	fp = fopen ("notary.flag", "ab+");
	if(fp == NULL){
		perror("ERROR opening notary.flag");
	}

	sleep(1);
	fprintf(fp, "Coucou");
	fflush(fp);

	fclose(fp);
}

int main(void)
{
	daemonize();

	listen_client();	

	return( EXIT_SUCCESS );
}

