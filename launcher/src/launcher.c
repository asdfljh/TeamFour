#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include "jsmn.h"

#include "gpgme.h"
#include <locale.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <b64/cencode.h>
#include <b64/cdecode.h>

#define PORT_NUMBER 8001
#define BUF_SIZE 65536*16
#define FILE_NAME_SIZE 256

#define fail_if_err(err)                    \
  do                                \
    {                               \
      if (err)                          \
        {                           \
          fprintf (stderr, "%s:%d: %s: %s\n",           \
                   __FILE__, __LINE__, gpgme_strsource (err),   \
           gpgme_strerror (err));           \
          exit (1);                     \
        }                           \
    }                               \
  while (0)

#define KEYRING_DIR "~/.gnupg"


#define MAXLEN 4096

int port_bind(char* launcher_ip);

uint32_t change_endian(uint32_t src);
int check_ip_arg(char* start_ip, char* end_ip);
int check_range(char* start_ip, char* end_ip, char* object_ip);

int jsonParse(char* filePath, char* jsonContents);
int jsoneq(const char *json, jsmntok_t *tok, const char *s);
void makeFile(char* filePath, char* nameContents, size_t fileSize, unsigned char* fileContents);

int verify(char* base64_output);

int executeFile(char* filePath);
void debug(pid_t pid);

void getMilSecond(char* milSecond);
void getDefaultPath(char* defaultPath, char* exePath);


int main(int argc, char** argv) {
    int r;
    int server_socket;
    int client_socket;
    int client_address_size;
    struct sockaddr_in client_address;
    char buffer[BUF_SIZE];
    char filePath[FILE_NAME_SIZE];
    char defaultPath[FILE_NAME_SIZE];

    /* check the number of arguments */
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <launcher_ip> <start_ip> <end_ip>\n", argv[0]);
        exit(1);
    }

    /* check argument is correct */
    if (check_ip_arg(argv[2], argv[3]) == -1) {
        fprintf(stderr, "IP Argument Error\n");
        exit(1);
    }

    getDefaultPath(defaultPath, argv[0]);

    server_socket = port_bind(argv[1]);

    /* port bind error */
    if (server_socket == -1) {
        exit(1);
    }

    while(1) {
        client_address_size = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_size);

        /* accept error */
        if (client_socket == -1) {
            fprintf(stderr, "accept() error\n");
            continue;
        }

        /* check notary program's ip */
        if (check_range(argv[2], argv[3], inet_ntoa(client_address.sin_addr)) == -1) {
            fprintf(stderr, "range error\n");
            close(client_socket);
            continue;
        }

        printf("DEBUG] Hello %s\n", inet_ntoa(client_address.sin_addr));

        /* get JSON format from notary program */
        memset(buffer, 0, BUF_SIZE);
        memset(filePath, 0, FILE_NAME_SIZE);

        strcpy(filePath, defaultPath);

        r = read(client_socket, buffer, BUF_SIZE);

        if (r <= 0) {
            break;
        }

        if (jsonParse(filePath, buffer) == 0) {
            printf("DEBUG] Client: %s\n", filePath);
            memset(buffer, 0, BUF_SIZE);
            strcpy(buffer, "Success");
            write(client_socket, buffer, strlen(buffer));
        } else {
            memset(buffer, 0, BUF_SIZE);
            strcpy(buffer, "Fail");
            write(client_socket, buffer, strlen(buffer));
        }

        close(client_socket);

    }
    close(server_socket);

    return 0;

}

/* port bind function */
int port_bind(char* launcher_ip) {
    int server_socket;
    struct sockaddr_in server_address;

    server_socket = socket(PF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) {
        fprintf(stderr, "socket() error\n");
        return -1;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(launcher_ip);
    server_address.sin_port = htons(PORT_NUMBER);

    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
        fprintf(stderr, "bind() error\n");
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, 5) == -1) {
        fprintf(stderr, "listen() error\n");
        close(server_socket);
        return -1;
    }

    return server_socket;
}

/* change endian function to check ip range */
uint32_t change_endian(uint32_t src) {
    uint32_t a, b, c, d, result;

    a = (src >> 24) & 0xFF;
    b = ((src >> 16) & 0xFF) << 8;
    c = ((src >> 8) & 0xFF) << 16;
    d = (src & 0xFF) << 24;
    result = a | b | c | d;

    return result;
}

/* check ip arguments is correct */
int check_ip_arg(char* start_ip, char* end_ip) {
    uint32_t start = change_endian(inet_addr(start_ip));
    uint32_t end = change_endian(inet_addr(end_ip));

    if (start <= end) {
        return 0;
    }

    return -1;
}

/* check ip range */
int check_range(char* start_ip, char* end_ip, char* object_ip) {
    uint32_t start = change_endian(inet_addr(start_ip));
    uint32_t end = change_endian(inet_addr(end_ip));
    uint32_t object = change_endian(inet_addr(object_ip));

    if (object >= start && object <= end) {
         return 0;
    }

    return -1;
}

/* parsing json's content name */
int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}

/* get name and contents */
int jsonParse(char* filePath, char* jsonContents) {
    int i, r;
    jsmn_parser p;
    jsmntok_t t[128];
    char* nameContents;
    char* bodyContents;
    size_t nameSize = 0;
    size_t bodySize = 0;
    size_t fileSize = 0;
    int verified = 0;

    jsmn_init(&p);
    r = jsmn_parse(&p, jsonContents, strlen(jsonContents), t, sizeof(t)/sizeof(t[0]));
    if (r < 0) {
        fprintf(stderr, "Failed to parse JSON: %d\n", r);
        return -1;
    }

    if (r < 1 || t[0].type != JSMN_OBJECT) {
        fprintf(stderr, "Object expected\n");
        return -1;
    }

    /* get name and contents */
    for (i = 1; i < r; i+=2) {
        if (jsoneq(jsonContents, &t[i], "name") == 0) {
            nameSize = t[i+1].end - t[i+1].start + 1;
            nameContents = (char*)malloc(nameSize*sizeof(char));
            memcpy(nameContents, jsonContents + t[i+1].start, nameSize - 1);
            nameContents[nameSize-1] = '\0';
        } else if (jsoneq(jsonContents, &t[i], "body") == 0) {
            bodySize = t[i+1].end - t[i+1].start + 1;
            bodyContents = (char*)malloc(bodySize*sizeof(char));
            memcpy(bodyContents, jsonContents + t[i+1].start, bodySize - 1);
            bodyContents[bodySize-1] = '\0';
        }
    }

    /* set up a destination buffer large enough to hold the encoded data */
    char* output = (char*)malloc(65535);
    /* keep track of our decoded position */
    char* c = output;
    /* store the number of bytes decoded by a single call */
    int cnt = 0;
    /* we need a decoder state */
    base64_decodestate s;

    /*---------- START DECODING ----------*/
    /* initialise the decoder state */
    base64_init_decodestate(&s);
    /* decode the input data */
    cnt = base64_decode_block(bodyContents, bodySize-1, c, &s);
    c += cnt;
    /* note: there is no base64_decode_blockend! */
    /*---------- STOP DECODING  ----------*/

    /* we want to print the decoded data, so null-terminate it: */
    *c = 0;
    makeFile(filePath, nameContents, cnt, output);

    free(output);

    /* check pgp key and execute file in contents */
    verified = verify(filePath);

    free(nameContents);
    free(bodyContents);

    if (verified == -1) {
        return -1;
    }

    return 0;
}

/* make gpg file */
void makeFile(char* filePath, char* nameContents, size_t fileSize, unsigned char* fileContents) {
    FILE* fp;
    char milSec[21];

    getMilSecond(milSec);

    strcat(filePath, milSec);
    strcat(filePath, "_");
    strcat(filePath, nameContents);

    fp = fopen(filePath, "w");

    fwrite(fileContents, 1, fileSize, fp);

    fclose(fp);
}

int verify_gpgme(char* base64_output){
    gpgme_error_t error;
    gpgme_engine_info_t info;
    gpgme_ctx_t context;
    gpgme_key_t recipients[2] = {NULL, NULL};
    gpgme_data_t signed_text, clear_text, encrypted_text;
    gpgme_encrypt_result_t result;
    gpgme_user_id_t user;
    char *buffer;
    ssize_t nbytes;
    char cmdline[1024];
    char gpg_output[FILE_NAME_SIZE];
    char exe_output[FILE_NAME_SIZE];
    char buff[1024];
    FILE *fp;

    /* convert base64 to gpg file */
    strcpy(gpg_output, base64_output);
    strcat(gpg_output, ".gpg");

    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline, "base64 --decode %s > %s", base64_output, gpg_output);
    fp = popen(cmdline, "r");
    if(fp == NULL) { // ERROR
        perror("base64 error");
        pclose(fp);
        return -1;
    }
    pclose(fp);

  /* Initializes gpgme */
    gpgme_check_version (NULL);

  /* Initialize the locale environment.  */
    setlocale (LC_ALL, "");
    gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
#ifdef LC_MESSAGES
    gpgme_set_locale (NULL, LC_MESSAGES, setlocale (LC_MESSAGES, NULL));
#endif

    error = gpgme_new(&context);
    fail_if_err(error);
  /* Setting the output type must be done at the beginning */
    gpgme_set_armor(context, 1);

  /* Check OpenPGP */
    error = gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP);
    fail_if_err(error);
    error = gpgme_get_engine_info (&info);
    fail_if_err(error);
    while (info && info->protocol != gpgme_get_protocol (context)) {
      info = info->next;
    }
  /* TODO: we should test there *is* a suitable protocol */
    fprintf (stderr, "Engine OpenPGP %s is installed at %s\n", info->version,
         info->file_name); /* And not "path" as the documentation says */

  /* Initializes the context */
    error = gpgme_ctx_set_engine_info (context, GPGME_PROTOCOL_OpenPGP, NULL,
                     KEYRING_DIR);
    fail_if_err(error);

    error = gpgme_op_keylist_start(context, NULL, 1);
    fail_if_err(error);
    error = gpgme_op_keylist_next(context, &recipients[0]);
    fail_if_err(error);

    /* Prepare the data buffers */
    error = gpgme_data_new(&clear_text);//gpgme_data_new_from_mem(&clear_text, SENTENCE, strlen(SENTENCE), 1);
    fail_if_err(error);
    error = gpgme_data_new(&encrypted_text);
    fail_if_err(error);

    int fd = open(gpg_output, O_RDONLY);

    error = gpgme_data_new_from_fd(&signed_text, fd);
    fail_if_err(error);

  /* Verify */
    error = gpgme_op_verify(context, signed_text, NULL, clear_text);
    fail_if_err(error);
    gpgme_verify_result_t verify_result = gpgme_op_verify_result(context);
    if(!(verify_result->signatures->summary & GPGME_SIGSUM_VALID)){
      printf("NOT VALID SIG\n");
      return -1;
    }
    printf("VALID SIG\n");
    gpgme_data_release(signed_text);
    strcpy(exe_output, base64_output);
    strcat(exe_output, ".exe");

    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline, "gpg --output %s --decrypt %s", exe_output, gpg_output);
    system(cmdline);
    /* give permission to execute */
    if (chmod(exe_output, 0755) == -1) {
        fprintf(stderr, "chmod err\n");
        return -1;
    }

    /* execute the file */
    executeFile(exe_output);

    /* OK */
    return 1;
}

/* verify and excute file */
int verify(char* gpg_output){
    char cmdline[1024];
    char exe_output[FILE_NAME_SIZE];
    char buff[1024];
    FILE *fp;
    int result = 0;
    char* good = "gpg: Good signature from \"IS521_Notary <IS521_Notary@kaist.ac.kr>\"\n";

    /* convert gpg to exe file with verification */
    strcpy(exe_output, gpg_output);
    strcat(exe_output, ".exe");

    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline, "gpg --output %s --decrypt %s 2>&1", exe_output, gpg_output);

    fp = popen(cmdline, "r");
    if(fp == NULL) { // ERROR
        perror("gpg error");
        pclose(fp);
        return -1;
    }

    while(fgets(buff, sizeof(buff), fp) != NULL) {
        if (strncmp(good, buff, strlen(good)) == 0) {
            result = 1;
        }

    }
    pclose(fp);

    if (result == 0) {
        fprintf(stderr, "not authenticated file\n");
        return -1;
    }

    /* give permission to execute */
    if (chmod(exe_output, 0755) == -1) {
        fprintf(stderr, "chmod err\n");
        return -1;
    }

    /* execute the file */
    executeFile(exe_output);

    return 0;
}

/* make process to execute file */
int executeFile(char* filePath) {
    pid_t child;
    child = fork();
    if(child == 0){ // childe
        chroot("./");
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        execl(filePath, filePath, 0);
    } else if (child < 0){
        perror("fork");
    }else{ // parent
        debug(child);
    }

    return 0;
}

/* watch the file using ptrace */
void debug(pid_t pid) {
    int syscall_num, status;
    struct user_regs_struct regs;
    waitpid(pid, &status, 0);
    ptrace(PTRACE_SYSCALL, pid, 0, 0);
    while(WIFSTOPPED(status)){
        ptrace(PTRACE_GETREGS, pid, 0, &regs);
        syscall_num = regs.orig_eax;
        if(syscall_num == -1){ // Error occurred
            printf("Error occurred at runtime\n");
            kill(pid, 9);
            return;
        }
        if(syscall_num == 11){ // execve
            printf("SYSCALL #%d called\n", syscall_num);
            kill(pid, 9);
            return;
        }
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
        waitpid(pid, &status, 0);
    }
    return;
}

void getMilSecond(char* milSecond) {
    struct tm t;
    struct timeval val;

    gettimeofday(&val, NULL);

    localtime_r(&val.tv_sec, &t);

    sprintf(milSecond, "%04d%02d%02d%02d%02d%02d%06ld",
        t.tm_year+1900, t.tm_mon+1, t.tm_mday,
        t.tm_hour, t.tm_min, t.tm_sec, val.tv_usec);
}

void getDefaultPath(char* defaultPath, char* exePath) {
    int i, count = 0;

    realpath(exePath, defaultPath);

    for (i = strlen(defaultPath); i >= 0; i--) {
        if (defaultPath[i] == '/') {
            count = count + 1;
        }
        defaultPath[i] == 0x00;

        if (count == 2) {
            defaultPath[i] = '\0';
            break;
        }
    }

    strcat(defaultPath, "/files/");
}
