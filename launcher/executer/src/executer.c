#include <stdio.h>
#include <unistd.h> // chroot, getpid, pid_t
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <signal.h>
void show_usage(char* p){
    printf("(Usage) %s <executable_path>\n", p);
}

void debug(pid_t pid);

int main(int argc, char** argv){
    if(argc < 2){
        show_usage(argv[0]);
        return 1;
    }
    
    char* executable_path = argv[1];
    
    pid_t child;
    child = fork();
    if(child == 0){ // childe
        chroot("./");
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        execl(executable_path, executable_path, 0);
    }else if (child < 0){
        perror("fork");
    }else{ // parent
        debug(child);
    }

    return 1;
}

void debug(pid_t pid){
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
    //printf("NONONO\n");
}
