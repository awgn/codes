/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int timedsystem(const char *filename, char * const argv[], char * const envp[], int sec, int signo)
{    
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, NULL);

    signal(SIGCHLD, exit);

    pid_t pid = fork();
    if ( pid == -1) {
        return -1;
    }
    if ( pid == 0 ) { /* child */
        signal(SIGCHLD,SIG_DFL);
        sigprocmask(SIG_UNBLOCK, &set, NULL);

        if ( execve(filename,argv,envp) < 0 ) 
            raise(SIGABRT);
    }

    /* parent */
    struct timespec timeout = { sec, 0 };
    siginfo_t info;

    if ( sigtimedwait(&set, &info, &timeout) == SIGCHLD )
    {
        // free the resource...
        wait(NULL);
        signal(SIGCHLD,SIG_DFL);
        sigprocmask(SIG_UNBLOCK, &set, NULL);

        if (info.si_code == CLD_EXITED) {
            return __W_EXITCODE(info.si_status,0);
        }
        if (info.si_code == CLD_KILLED) {
            return __W_EXITCODE(0, info.si_status);
        }
        if (info.si_code == CLD_STOPPED) {
            return __W_STOPCODE(info.si_status);
        }
        return -1;
    }

    // timeout
    //

    signal(SIGCHLD,SIG_DFL);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    kill(signo, pid);
    return -1;
}

int
main(int argc, char *argv[])
{
    char * const arg[] = { "/bin/sleep", "10", NULL };

    int n = timedsystem( arg[0], arg, NULL, 2, SIGTERM ); 
    if ( n != -1 ) {
        printf("exit status=%d\n", WEXITSTATUS(n));
    }
    if ( n == -1 ) {
        printf("timeout!\n");
    }
    return 0;
}
 
