#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
int main(){
    int result = mkdir("./testtest",S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    printf("mkdir result : %d\n");
    return 1;
}
