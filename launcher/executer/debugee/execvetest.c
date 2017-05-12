#include <stdio.h>
#include <unistd.h>

int main(){
    execve("/bin/sh", NULL, NULL);
    return 1;
}
