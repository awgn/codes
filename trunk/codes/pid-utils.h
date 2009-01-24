/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef PID_UTIL_H
#define PID_UITL_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

    extern int pid_filter(const struct dirent *d);
    extern int pid_sort(const void *a, const void *b);

    extern pid_t get_parent(pid_t p);
    extern pid_t get_sibling(pid_t pid, pid_t sib);
    extern pid_t get_child(pid_t pid, pid_t ch);
    extern pid_t get_pid(const char *cmd, pid_t start);
    extern int get_process(pid_t pid, const char *target, char *proc, size_t size);

#ifdef __cplusplus
}   /* C++ */
#endif

#endif /* PID_UTIL_H */
