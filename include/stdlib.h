#ifndef _STDLIB_H
#define _STDLIB_H

#include <defs.h>
#include <sys/file.h>

int main(int argc, char* argv[]);
void exit(int status);

uint64_t fork();
int get_pid();
int get_ppid();
void list_process();
void *malloc(uint64_t no_bytes);
void wait();
void waitpid(int pid);
int execve(char *filename, char *argv[], char *env[]);
void di();
void si();

dirent *readdir(DIR* dir);
int open(const char *pathname, int flags);
DIR *opendir(const char *fullpath);
int closedir(DIR *dir);
void getPath(char* buffer);
void updatePath(char* buffer);
void updateEnvPath(char* buffer);

int read(int fd, char* buffer,int size);
void write(int fd, char* buffer,int size);
int close(int fd);
int sleep(int seconds);

#endif
