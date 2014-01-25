#ifndef FILE_H
#define FILE_H
#include <sys/dirent.h>
#define MAXFDENTRIES 150
#define OPEN_MAX 150
#define EOF 26

struct file_descriptor{
//24
int number;
struct File* fp;
uint64_t seek; 
unsigned int flags;
};

typedef struct file_descriptor file_descriptor_t;

//Size 532
struct File {
char  path[NAMEMAX]; //256
uint64_t offset; //8
struct dirent parent; //264
unsigned int flags; //4
};

int open(const char *pathname, int flags);
//int open(const char *pathname, int flags, mode_t mode);         
//int read(struct file_descriptor_t * fd);
int close(int fd);

void init_fdlist();
int t_read(int fildes, void *buf, int nbyte);

#endif