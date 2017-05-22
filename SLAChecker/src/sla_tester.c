#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <errno.h>
#define BUFSIZE 255
int client_len;
int client_sockfd;
struct sockaddr_in clientaddr;

char send_data[BUFSIZE];
char client[BUFSIZE];

int handout_github(char* github_id);
void createSocket(char* ip, int port);


void print_usage(char* arg){
    printf("(Usage) %s <IP> <Port_Num>\n", arg);
    return;
}

int main(int argc, char** argv){
    if(argc != 3){
        print_usage(argv[0]);
        return;
    }
    char* ip = argv[1];
    int port = atoi(argv[2]);     
    createSocket(ip, port);
    
    int connect = timeout_connect();
    if(connect<1)
        return 2;
    printf("Testing \"HANDOUT_GITHUBID\"\n"); 
    int github = handout_github("jaemoon-sim");
    if(github < 1)
        return 1; 
    return 0;
}

/*
createSocket() : Create a socket.
                 Saves client_sockfd, client_len, and clientaddr in the global variable
Input  : None
Output : None

*/
void createSocket(char* ip, int port){
    client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_addr.s_addr = inet_addr(ip);
    clientaddr.sin_port = htons(port);
    
    struct timeval tv;
    tv.tv_usec = 100000 ; //0.01 second timeout
    setsockopt(client_sockfd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));    
    client_len = sizeof(clientaddr);
}

/*
timeout_connect() : Using created socket (client_sockfd), make a connection with time-out
Input  : None
Output : 1 (connection success), 0 (connection failed)
reference : http://egloos.zum.com/jjunda/v/4202422
*/
int timeout_connect(){
    int res;
    long arg;    
    fd_set myset;
    struct timeval tv;
    int valopt;
    socklen_t lon; 
    //Set socket non-blocking
    arg = fcntl(client_sockfd, F_GETFL, NULL);
    if(arg < 0)
        return 0;
    arg |= O_NONBLOCK;
    if(fcntl(client_sockfd, F_SETFL, arg) < 0)
        return 0;
    
    //Trying to connect;
    res = connect(client_sockfd, (struct sockaddr *)&clientaddr, client_len);
    if(res < 0){
         if (errno == EINPROGRESS) { 
            //fprintf(stderr, "EINPROGRESS in connect() - selecting\n"); 
            do { 
                tv.tv_sec = 3;
                tv.tv_usec = 0; 
                FD_ZERO(&myset); 
                FD_SET(client_sockfd, &myset); 
                res = select(client_sockfd+1, NULL, &myset, NULL, &tv); 
                if (res < 0 && errno != EINTR) { 
                    fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
                    return 0; 
                } else if (res > 0) { 
                    // Socket selected for write 
                    lon = sizeof(int); 
                    if (getsockopt(client_sockfd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
                        fprintf(stderr, "Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
                        return 0; 
                    } 
              // Check the value returned... 
                    if (valopt) { 
                        fprintf(stderr, "Error in delayed connection() %d - %s\n", valopt, strerror(valopt) 
); 
                        return 0; 
                    } 
                    break; 
               } else { 
                    fprintf(stderr, "Timeout in select() - Cancelling!\n"); 
                    return 0; 
                } 
        } while (1); 
        } else { 
            fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
            return 0; 
        } 
    } 
  // Set to blocking mode again... 
    if( (arg = fcntl(client_sockfd, F_GETFL, NULL)) < 0) { 
        fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
        return 0; 
    } 
    arg &= (~O_NONBLOCK); 
    if( fcntl(client_sockfd, F_SETFL, arg) < 0) { 
        fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
        return 0; 
    }
    return 1; 
}


int handout_github(char* github_id){
    int ret_recv, ret_send;
    char recv_data[1024];
    char send_data[1024];
    int err;
    ret_recv = recv(client_sockfd, recv_data, sizeof(recv_data), 0);
    ret_send = send(client_sockfd, github_id, strlen(github_id)+1, 0);
    ret_recv = recv(client_sockfd, recv_data, sizeof(recv_data), 0);
    err = errno;
    if(ret_recv < 0){
        if(err == EWOULDBLOCK){
            return 0; 
        }else{
            perror("sending github id : ");
            return 0;
        }
    }
    printf("Handout_pubkey recv : %s \n", recv_data);
    return 1; 
}

/*
int handshake_notary(int sock, const char* ID, const char* privKeyPath, const char* passPath, const char* successMsg){
    gpgme_ctx_t ctx;  // the context
    gpgme_error_t err; // errors
    gpgme_key_t key[2] = {NULL, NULL}; // the key
    gpgme_key_t pri_key[2]  = {NULL, NULL};

    gpgme_data_t clear_buf, encrypted_buf, import_key_buf, decrypted_buf, recv_buf; // plain buf, encryped buf
    gpgme_user_id_t user; //the users
    gpgme_encrypt_result_t  result; // result

    unsigned char* rand_number =NULL;
    unsigned char* buffer = NULL;
    ssize_t nbytes;
    int index = 0;
    
    init_gpgme(&ctx);
    gpgme_data_new(&recv_buf);    
}*/
