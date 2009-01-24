/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <pid-utils.h>

int main(int argc, char *argv[])
{
    char process[256];

    printf("get_pid(\"xterm\", ...) test:\n");

    pid_t pid = 0;

    for( ; (pid = get_pid("xterm",pid)) && pid != 0; ) {
        get_process(pid, "exe", process,256);
        printf("   %s[%d]\n", process,pid); 
    }

    pid = getpid();
    printf("get_parent(%d) recursive test:\n", pid);

    do {

        if ( get_process(pid,"exe", process,256) == -1)
            break;

        printf("    %s[%d]\n",process, pid);
    }
    while ( (pid = get_parent(pid)) != 0); 

    pid = getpid();
    printf("get_sibling(%d,...) recursive test:\n", pid);

    pid_t sib = 0;
    while ( (sib =get_sibling(pid,sib)) != 0 )
    {
        get_process(sib,"exe", process,256);
        printf("   sibling: %s[%d]\n",process,sib);
    }
 
    pid = getppid();
    printf("get_child(%d,...) recursive test:\n", pid);

    pid_t ch = 0;
    while ( (ch =get_child(pid,ch)) != 0 )
    {
        get_process(ch,"exe",process,256);
        printf("   child: %s[%d]\n",process,ch);
    }
  
    pid = getpid();
    printf("get_process(%d,...) test:\n", pid);
    printf("   cwd: %s\n", (get_process(pid, "cwd", process, 256),  process));
    printf("   exe: %s\n", (get_process(pid, "exe", process, 256),  process));
    printf("  root: %s\n", (get_process(pid, "root", process, 256), process));

    return 0;
}
