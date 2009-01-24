/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <pid-utils.h>
#include <assert.h>

/* pid filter and sort callbacks
 */

#ifndef MAX
#define MAX(a,b) \
       ({typeof(a) _a = (a); typeof(b) _b = (b); _a > _b ? _a : _b; })
#endif

#ifndef MIN
#define MIN(a,b) \
       ({typeof(a) _a = (a); typeof(b) _b = (b); _a < _b ? _a : _b; })
#endif


static inline
void free_dirent(struct dirent **d, int n)
{
    assert(n>=0);
    int i;
    for (i=0; i< n; i++)
        free(d[i]);
    free(d);
}

int
pid_filter(const struct dirent *d)
{
    if (atoi(d->d_name))
        return 1;
    return 0;
}


int 
pid_sort(const void *a, const void *b)
{
    struct dirent **dir_a = (struct dirent **)a;
    struct dirent **dir_b = (struct dirent **)b;

    unsigned int a_pid = atoi((*dir_a)->d_name);
    unsigned int b_pid = atoi((*dir_b)->d_name);
    return a_pid-b_pid;
}


pid_t 
get_parent(pid_t p)
{
    char status[24], *line=NULL;
    size_t len = 0;
    pid_t pid = 0;
    FILE *fp;
    size_t n;

    snprintf(status,sizeof(status),"/proc/%d/status",p);

    fp = fopen(status, "r");
    if (fp == NULL) 
        return 0;

    while ( (n = getline(&line,&len,fp)) != -1 ) {

        if ( strstr( line, "PPid:") == line ) {
            if ( sscanf(line,"%*s %d\n",&pid) != 1 ) 
                pid = 0;
            break;
        }
    }

    fclose(fp);
    free(line);

    return pid;
}


pid_t
get_sibling(pid_t pid, pid_t sib)
{
    struct dirent **filelist = NULL;
    int n, i;
    pid_t ret = 0;

    pid_t parent = get_parent(pid);
    if ( parent == 0 )
        return 0;

    n = scandir("/proc", &filelist, pid_filter , pid_sort);
    for(i=0; i<n; ++i) {

        pid_t p = atoi(filelist[i]->d_name);

        if (p<= sib) 
            continue;

        if ( p == pid )
            continue;

        int q = get_parent(p);
        if ( !q || q != parent)
            continue;
        
     /* q == parent */
        ret = p;
        break;
    }

    free_dirent(filelist, n);
    return ret;
}


pid_t
get_child(pid_t pid, pid_t child)
{
    struct dirent **filelist = NULL;
    int n, i;
    pid_t ret;

    n = scandir("/proc", &filelist, pid_filter , pid_sort);
    for(i=0; i<n; ++i) {

        pid_t p = atoi(filelist[i]->d_name);

        if (p<= child) 
            continue;

        if ( p == pid )
            continue;

        int q = get_parent(p);
        if ( !q || q != pid)
            continue;
        
    /* q == parent */
        ret = p;
    }

    free_dirent(filelist, n);
    return 0;
}


pid_t
get_pid(const char *cmd, pid_t start)
{
    char status[80], *line = NULL;
    struct dirent **filelist;
    size_t size = 0;
    FILE *f;
    int i, n;
 
    pid_t pid = 0;
    n = scandir("/proc", &filelist, pid_filter, pid_sort);
    for(i=0; i<n; i++) {
        pid_t p = atoi(filelist[i]->d_name);

        if ( p <= start ) 
            continue;
        
        if ( sprintf(status,"/proc/%d/status", p) < 0 )
            continue;
        f = fopen(status,"r");
        if ( f == NULL ) {
            continue;
        }
        if ( getline(&line, &size, f) < 0 ) {
            goto next;
        }

        if ( sscanf(line,"Name: %s", status) != 1 )
            goto next;

        if (!strcmp(status,cmd)) {
            fclose(f);
            pid = p;
            break;
        }
next:
        fclose(f);
    }

    free(line);
    free_dirent(filelist, n);
    return pid;
}


int 
get_process(pid_t pid, const char *target, char *proc, size_t size)
{
    char lk[256];

    assert(!( strcmp(target, "cwd") &&
              strcmp(target, "exe") &&
              strcmp(target, "root") ) &&
             "bad_target: exe, cwd, root");

    snprintf(lk, sizeof(lk), "/proc/%d/%s",pid,target);

    int n;
    if ( (n=readlink(lk,proc,size-1)) == -1) 
        return -1;

    proc[ MIN(n, size-1) ]='\0';
    return 0;
}

