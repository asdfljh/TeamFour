#include <stdio.h>
#include <stdlib.h>
int verify(char* verifiee, char* output);

int main(int argc, char** argv){
    
    char* verifiee;
    char* output;
    verifiee = (char*)argv[1];
    output = (char*)argv[2];
    
    printf("verify result : %d\n", verify(verifiee, output));    
    return 1;
}

int verify(char* verifiee, char* output){
    char cmdline[150];
    char buff[1024];
    FILE *fp;
    sprintf(cmdline, "gpg --output %s --decrypt %s 2>&1", output, verifiee);
    printf("%s\n", cmdline);
    fp = popen(cmdline, "r");
    if(fp == NULL) { // ERROR
        perror("gpg_verify");
        return -1;
    }
    int idx = 0; 
    char* good = "gpg: Good signature from \"IS521_Notary <IS521_Notary@kaist.ac.kr>\"";
    while(fgets(buff, sizeof(buff), fp) != NULL){
        if(idx++ == 1){
            int i;
            for(i = 0 ; i < strlen(good) ; i++){
                if(buff[i] != good[i]){
                    pclose(fp);
                    return -1;
                }
            }
        }
        //printf(">> %s", buff);
    }
    pclose(fp);
    return 1;
}
