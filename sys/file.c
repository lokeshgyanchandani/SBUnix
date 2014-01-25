#include <stdio.h>
#include <defs.h>
#include <sys/errno.h>
#include <sys/pmap.h>
#include<sys/pagetab.h>
#include <sys/errno.h>
#include <math.h>
#include <sys/file.h>
#include <sys/elf.h>

#define INVALIDDESCRIPTER -1
#define BLOCKSIZE 512

file_descriptor_t fd_list[OPEN_MAX];
struct File* getFile(const char *fileName);
int cur_fd=3;


int close (int fd){
	if(fd_list[fd].number!=0){
		fd_list[fd].number = INVALIDDESCRIPTER;
		//FreePage((uint64_t)fd_list[fd].fp);
		return 0;
	}else{
		errno = EBADF;
		return -1;
	}
}


int open(const char *pathname, int flags){
	//printf("To open %s",pathname);
	struct File* file = getFile(pathname);
	int temp = 3;
	if(file != NULL){
		while(temp < cur_fd && fd_list[temp++].number!= INVALIDDESCRIPTER);
		fd_list[temp].number = temp+1;
		fd_list[temp].fp = file;
		fd_list[temp].seek = 0;
		fd_list[temp].flags = flags;
		//printf("fd_list - %d", fd_list[temp].number);
	}else{
		return -1;
	}
    return temp==cur_fd ? cur_fd++ : temp+1;
}


struct File* getFile(const char *fileName)
{
    struct posix_header_ustar * entry = (struct posix_header_ustar*)(&_binary_tarfs_start);
	int padding =0;
	struct File* file = (struct File *)kmalloc(PAGE_SIZE);
	int exitflag = 0;
    while((uint64_t)entry < (uint64_t)&_binary_tarfs_end)
        {
			int size = getSize(entry->size);
			//printf("entry->name - %s",entry->name);
			if(strcmp(entry->typeflag,"5")  == 0 && strncmp(fileName,entry->name,lastIndexOf (fileName, "/")+1) == 0){
				strncpy(file->parent.d_name,entry->name,NAMEMAX);
				file->parent.offset = (char*)entry - (char*)&_binary_tarfs_start;
				if(++exitflag >= 2)
					break;
			}
            if(strcmp(fileName,entry->name) == 0 && strcmp(entry->typeflag,"0")  == 0){
				strncpy(file->path, fileName, NAMEMAX);
				file->offset = (char*)entry - (char*)&_binary_tarfs_start;
				if(++exitflag >= 2)
					break;
			}
			entry = (struct posix_header_ustar *)((char*)entry + sizeof(struct posix_header_ustar) + size );
			if(size > 0){
				padding = BLOCKSIZE - size%BLOCKSIZE;
				if((char*)&_binary_tarfs_end - (char*)entry >=BLOCKSIZE)
				{
					entry = (struct posix_header_ustar *)((char*)entry + padding);
				}
			}	
		}
		/*if(strcmp(fileName,entry->name)!=0){
			printf("freeing page");
			FreePage((uint64_t)file);
			return NULL;
		}*/
		return file;
}


int t_read(int fildes, void *buf, int nbyte){	
	int readBytes=0,index=-1;
	while(fd_list[++index].number != fildes);
	if(fildes >= 0 && fd_list[index].number != INVALIDDESCRIPTER){
		char* curser = (char*)(&_binary_tarfs_start)+(fd_list[index].fp->offset) + BLOCKSIZE + fd_list[index].seek; //Bypassing the header of file.
		//printf("Content being copied -");
		while(readBytes < nbyte && *curser != EOF){
			//printf("%d-%c,",readBytes,*curser);
			*((char*)(buf+readBytes++)) = *(curser++);
			fd_list[fildes].seek++;
		}
		*((char*)(buf+readBytes)) = 0;
	}else{
		printf("\nfile does not exist");
		errno = EBADF;
	}
	return readBytes;
}

void init_fdlist(){
	file_descriptor_t fd_list[OPEN_MAX];
	int i = 0;
	//printf("initializing fdlist");
	for(i = 0;i < OPEN_MAX; i++){
		fd_list[i].number = INVALIDDESCRIPTER;
		fd_list[i].seek = 0;
		fd_list[i].fp = NULL;
		fd_list[i].flags = 0;
	}
		printf("fd_list - %d", fd_list[0].number);
}