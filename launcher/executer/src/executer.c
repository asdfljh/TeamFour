#include <stdio.h>

void show_usage(char* p){
    printf("(Usage) %s <executable_path>\n", p);
}

int main(int argc, char** argv){
    if(argc < 2){
        show_usage(argv[0]);
        return 1;
    }
    return 1;
}
