#include <defs.h>
#include <syscall.h>
#include <sys/idt.h>
#include <stdio.h>
#include <sys/sched.h>
#include <sys/mm.h>
#include <sys/pagetab.h>
#include <sys/pmap.h>
#include <sys/tarfs.h>
#include <sys/elf.h>
#include <sys/file.h>
#include <sys/dirent.h>
#include <math.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/dirent.h>
#include <sys/pic.h>

struct isr_stack_frame* g_regVals1;

char currentPath[100] = "";
char envPaths[200] = "";
char* lastPtr = envPaths;



struct registers_t* g_regVals;

#define NOTFOUND 0
#define FOUND 1



#define BLOCKSIZE 512

uint64_t add = 0x100000000;

int getSizes(char *p) 
{
   int k = 0;
    while (*p) {
       k = (k<<3)+(k<<1)+(*p)-'0';
        p++;
         }

     int decimal=0, i=0, rem; 
      while (k!=0) 
         { 
             rem = k%10; 
               k/=10; 
                 decimal += rem*pow(8,i); 
                   ++i; 
                    } 
       return decimal; 
}


extern char _binary_tarfs_start;
int errno;

 const char * _getdirname (const struct dirent *dp)
{
  return dp -> d_name;
}


void displayentry(struct posix_header_ustar* entry){
	printf("\nname - %s, size-%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s",entry ->name, entry->size, entry-> mode , entry-> uid, entry-> gid, entry-> mtime, entry-> checksum, entry-> typeflag, entry-> linkname, entry-> magic, entry-> version, entry-> uname, entry-> gname, entry-> devmajor, entry-> devminor , entry-> prefix  , entry-> pad);
}


int findNext(struct dirent* current)
{
	struct posix_header_ustar* entry = (struct posix_header_ustar*)&_binary_tarfs_start + current->offset;
	if(entry != NULL){
		int padding =0; int offset = current -> offset;
		char tempstr[NAMEMAX], dirname[NAMEMAX]; int index;
		strncpy(tempstr,entry->name,strlen(entry->name));
		if(strcmp(entry->typeflag,"5") == 0 || strcmp(entry->typeflag,"0") == 0){
			strncpy(tempstr,entry->name,strlen(entry->name)-1);
			tempstr[strlen(entry->name)-1] = 0;
			index = lastIndexOf (tempstr, "/");
			if(index == -1 || index == strlen(entry->name)-1)
			dirname[0] = '\0';
			else
			substring(dirname ,entry->name , 0, index);
		}
		
		do{		
			int size = getSizes(entry->size);	
			//increasing by size of the structure + size of file
			entry = (struct posix_header_ustar *)((char*)entry + sizeof(struct posix_header_ustar) + size );
			offset += 1 + (size/BLOCKSIZE);
			if(size > 0){
				padding = BLOCKSIZE - size%BLOCKSIZE;
				//printf("value - %d & padding - %d",(char*)&_binary_tarfs_end - (char*)entry, padding);
				if((char*)&_binary_tarfs_end - (char*)entry >= BLOCKSIZE && padding > 0)
				{
					entry = (struct posix_header_ustar *)((char*)entry + padding);
					offset++;
				}
				//printf(" ,address:%p",entry);
				}	
			if(starts_with( entry-> name,dirname) && strcmp(entry->typeflag,"5")  == 0) //starts_with( base,prefix)
			{
				if(indexOf_shift(entry-> name, "/", strlen(dirname)+1) == strlen(entry->name)-1){
					//indexOf_shift(base, str, startIndex)
					current -> offset = offset;	
					strncpy(current -> d_name, entry->name, NAMEMAX-1);
					//printf("found - %s",current->d_name);
					return 0;
				}
			}
			if(starts_with( entry-> name,dirname) && strcmp(entry->typeflag,"0")  == 0 ){
				if((indexOf_shift(entry-> name, "/", strlen(dirname)+1))==-1){
					current -> offset = offset;	
					strncpy(current -> d_name, entry->name, NAMEMAX-1);
					//printf("found - %s",current->d_name);
					return 0;
				}
			}	
			}while((uint64_t)entry < (uint64_t)&_binary_tarfs_end);
		}
		errno = ENOENT;
		return errno;
 }



int findfirst(char* dirname, char spec , struct dirent* data )
{	
	struct posix_header_ustar * entry = (struct posix_header_ustar *)&_binary_tarfs_start;
	
	//specs of files: not considering for now, (_A_SUBDIR | _A_RDONLY | _A_ARCH | _A_SYSTEM | _A_HIDDEN)
	if(entry != NULL){
		int padding =0, offset = 0;
		while( (uint64_t)entry < (uint64_t)&_binary_tarfs_end){
			int size = getSizes(entry->size);
			if(starts_with(entry-> name,dirname) && strcmp(entry->typeflag,"5")  == 0) //starts_with( base,prefix)
			{
				if(indexOf_shift(entry-> name, "/", strlen(dirname)+1) == strlen(entry->name)-1){
					//indexOf_shift(base, str, startIndex)
					strncpy(data -> d_name,entry -> name,NAMEMAX-1);
					data -> offset = offset;
					//printf("found - %s",data->d_name);
					return 0;
				}
			}
			if(starts_with( entry-> name,dirname) && strcmp(entry->typeflag,"0")  == 0 ){
				if((indexOf_shift(entry-> name, "/", strlen(dirname)+1))==-1){
					strncpy(data -> d_name,entry -> name,NAMEMAX-1);
					data -> offset = offset;
					//printf("found - %s",data->d_name);
					return 0;
				}
			}
			//increasing by size of the structure + size of file
			entry = (struct posix_header_ustar *)((char*)entry + sizeof(struct posix_header_ustar) + size );
			offset += 1 + (size/BLOCKSIZE);
			if(size > 0){
				padding = BLOCKSIZE - size%BLOCKSIZE;
				//printf("value - %d & padding - %d",(char*)&_binary_tarfs_end - (char*)entry, padding);
				if((char*)&_binary_tarfs_end - (char*)entry >= BLOCKSIZE && padding > 0)
				{
					entry = (struct posix_header_ustar *)((char*)entry + padding);
					offset += 1;
				}
				//printf(" ,address:%p",entry);
				}	
			}
		}
		errno = ENOENT;
		return errno;
}



 int initdir (DIR *dirp)
{ 
  dirp->dirent_filled = 0;

  if (findfirst (dirp->dirname, _A_SUBDIR | _A_RDONLY | _A_ARCH | _A_SYSTEM | _A_HIDDEN, &dirp->current) != 0)
  {
    /* findfirst will set errno to ENOENT when no 
     * more entries could be retrieved. */
    return 0;
  }
  // initialize DIR and it's first entry 
  dirp->dirent_filled = 1;
  return 1;
}



 DIR * opendir(const char *dirname)
{
  DIR *dirp;
  if(dirname != NULL){
	uint64_t page = (uint64_t)kmalloc_mem();
	mmap((uint64_t)add, page);
	dirp = (DIR *)(add);
	add = add+4096;
	//dirp = (DIR*)kmalloc(PAGE_SIZE);
	if (dirp != NULL) {
    char *p;
    strncpy (dirp->dirname, dirname, NAMEMAX);
    p = strchr (dirp->dirname, '\0'); //Return substring starting from character c
    if (dirp->dirname < p  &&
        *(p - 1) != '\\'  &&  *(p - 1) != '/'  &&  *(p - 1) != ':')
    {
      strcpy (p++, "/");
    }
	
    // open stream 
    if (initdir (dirp) == 0) {//Returns 1 if directory is found.
      // initialization failed 
      //FreePage(getMappedPhysicalAddress((uint64_t)dirp));
	  errno = ENOENT;
      return NULL;
	}
	//printf("found : %s",dirname);
	}
  return dirp;
  }
  errno = ENOTDIR;
  return NULL;
}


struct dirent * readdir (DIR *dirp)
{
  if (dirp == NULL) {
    errno = EBADF;
    return NULL;
  }
  if (dirp->dirent_filled != 0) {
  
    dirp->dirent_filled = 0;
  } else if (findNext(&dirp->current) != 0) {
      /* findnext will set errno to ENOENT when no
       * more entries could be retrieved. */
       return NULL;
  }
 // printf("\n\nfound next : %s",dirp->current.d_name);
  return &dirp->current;
} 

 int closedir (DIR *dirp)
 {   
  int retcode = 0;
 // make sure that dirp points to legal structure 
  if (dirp == NULL) {
    errno = EBADF;
    return -1;
  }
  // clear dirp structure to make sure that it cannot be used anymore
  //FreePage(GetMappedPhysicalAddress((uint64_t)dirp));
  return retcode;
}

uint64_t w_opendir(uint64_t rbx,uint64_t rcx)
 {
  DIR * directory1,* directory2;
  directory1 = (DIR *)rcx;
  directory2 = opendir((char*)rbx);
  if(directory2 != NULL){
	  strcpy(directory1->dirname, directory2->dirname);
	  strcpy(directory1->current.d_name, directory2->current.d_name);
	  directory1->current.offset = directory2->current.offset;
	  directory1->dirent_filled = directory2->dirent_filled;
	  return 1;
  } else{
	  return 0;
  }
 }

//Have to handle null return cases 
 uint64_t w_readdir(uint64_t rbx,uint64_t rcx)
 {
  dirent * dirent1, * dirent2;
  dirent1 = (dirent *)rcx;
  dirent2 = readdir((DIR *)rbx);
  if(dirent2 != NULL){
	  strcpy(dirent1->d_name, dirent2->d_name);
	  dirent1->offset = dirent2->offset;
	  return 1;
  } else{
	  return 0;
  }
}


int w_closedir(uint64_t rbx)
 {
  int ret;
  ret = closedir((DIR *)rbx);
  if(ret != -1){
	  return 1;
  }else{
	  return 0;
  }
 }


uint64_t w_open(uint64_t rbx,uint64_t rcx)
 {
  uint64_t fd;
  fd = (uint64_t) open((char*)rbx,(int)rcx);
  return fd;
 }

int w_close(uint64_t rbx)
 {
  uint64_t ret;
  ret = close((int)rbx);
  if(ret != -1){
	  return 1;
  }else{
	  return 0;
  }
 }



void GetCurrentPath(uint64_t rbx)
{
    char *str = (char*)rbx;
    char *str1 = currentPath;
  
	*str++ = '~';
        while(*str1)
        	*str++ = *str1++;
	 *str++ = ']';
        *str = '\0';
}

void UpdateEnvPath(uint64_t rbx)
{
	char *str = (char*)rbx;

	//only supporting PATH
	if(starts_with(str,"PATH=") == 1)
	{
		char temp[100]="";
		substring(temp,str,5,strlen(str));

		char *newEnv = temp;
		if(*newEnv  == '/')//should be full path
		{
		
			newEnv  +=1;
			DIR * dir =  opendir(newEnv);

	 		if(dir  != NULL)
	 		{
				int i=0;
				for(i=0;i<200;i++)
					envPaths[i] = '\0';

				lastPtr =envPaths;

				while(*newEnv)
					*lastPtr ++ = *newEnv ++;
			}
		}
		else if(starts_with(temp,"$PATH:") == 1)
		{
			char temp1[100]="";
			substring(temp1,temp,6,strlen(temp));

			char *newenv = temp1;
			if(*newenv == '/')//should be full path
			{
				newenv +=1;
				DIR * dir =  opendir(newenv);

	 			if(dir  != NULL)
	 			{
					if(lastPtr  == envPaths)//first one
					{
						while(*newenv)
							*lastPtr ++ = *newenv++;
					}
					else//new one being added
					{
						*lastPtr ++ = ':';
						while(*newenv)
							*lastPtr ++ = *newenv++;
					}
				}
			}
		}

	}

}
void UpdateCurrentPath(uint64_t rbx)
{
    char *newPath = (char*)rbx;

    if(*newPath == '/')
    {
	newPath = newPath+1;
	char temp[100] = "";
	char *tempPath = temp;
	while(*newPath)
			*tempPath++ = *newPath++;

	
	DIR * dir =  opendir(temp);

	 if(dir  != NULL)
	 {
		int i=0;
		for(i=0;i<100;i++)
			currentPath[i] = '\0';
		
		char *str1 = currentPath;
		char *tempPath = temp;
		while(*tempPath )
			*str1++ = *tempPath++;

	 }
    }
    else if(*newPath == '.')
    {
	 if(*(newPath+1) == '.') //parent directory case
	 {
		char temp[100] = "";
		char parent[100] = "";
		strncpy(temp,currentPath,strlen(currentPath)-1);
		temp[strlen(currentPath)-1] = '\0';
		int index = lastIndexOf (temp, "/");
		
		if(index == -1 || index == strlen(currentPath)-1)
		{
			parent[0] = '\0';
		}

			substring(parent,currentPath, 0, index);
			char *tempPath = (parent+index);
			newPath = newPath+2;
			while(*newPath )
        			*tempPath++ = *newPath++;

			DIR * dir =  opendir(parent);

	 		if(dir  != NULL)
	 		{
				int i=0;
				for(i=0;i<100;i++)
					currentPath[i] = '\0';

				char *cur = currentPath;
      				char *tempPath = parent;

				while(*tempPath)
        			*cur++ = *tempPath++;
	 		}

	 }
	 else //current directory case
	 {
		char temp[100] = "";
		char *cur = currentPath;
      		char *tempPath = temp;
	
		while(*cur)
        		*tempPath++ = *cur++;

		newPath = newPath+2;

		while(*newPath )
        		*tempPath++ = *newPath++;

		DIR * dir =  opendir(temp);

	 	if(dir  != NULL)
	 	{
			int i=0;
			for(i=0;i<100;i++)
				currentPath[i] = '\0';

			char *cur = currentPath;
      			char *tempPath = temp;

			while(*tempPath)
        		*cur++ = *tempPath++;
	 	}
	 }
    }
    else
    {
      char temp[100] = "";
      char *cur = currentPath;
      char *tempPath = temp;
	
	while(*cur)
        	*tempPath++ = *cur++;
	
	while(*newPath)
			*tempPath++ = *newPath++;

	DIR * dir =  opendir(temp);

	 if(dir  != NULL)
	 {
		int i=0;
		for(i=0;i<100;i++)
			currentPath[i] = '\0';

		char *cur = currentPath;
      		char *tempPath = temp;

		while(*tempPath)
        	*cur++ = *tempPath++;
	 }	
    }
}

void syscall_common_handler(struct isr_stack_frame* s)
{
	//int d=current_task->pid;
	//printf("Value %d\n",d);

	g_regVals1 = s;

	if(s->rax == EXIT)
	{
		//call the exit system call
		sys_exit();
	}

	else if(s->rax == PRINTF)
	{
		//call for printf
		//printf("Value %d\n",s->rbx);
		sys_putchar(s->rbx);
	}

	else if(s->rax == FORK)
	{
		//call for fork
		sys_fork();
	}

	else if(s->rax == EXECVE)
	{
		s->rax = sys_execve((char *)s->rbx, NULL, NULL);
	}

	else if(s->rax == GETPID)
	{
		//call for getpid
		sys_get_pid();
	}

	else if(s->rax == SLEEP)
	{
		//call for sleep
		sys_sleep(s->rbx);
	}

	else if(s->rax == LISTPROCESS)
	{		
		sys_list_process();
	}
	else if(s->rax == MALLOC)
	{
		void * ptr = sbrk_s(s->rbx);
		s->rax = (uint64_t)ptr;
	}
	else if(s->rax == WAIT)
	{
		WaitSysCall();
	}

	else if(s->rax == WAITPID)
	{
		WaitPidSysCall(s->rbx);
	}

	else if(s->rax == OPENDIR)
	{
		s->rax = w_opendir(s->rbx,s->rcx);
	}
	else if(s->rax == READDIR)
	{
		s->rax = w_readdir(s->rbx,s->rcx);
	}
	else if(s->rax == CLOSEDIR)
	{
		s->rax = w_closedir(s->rbx);
	}

	else if(s->rax == CURRENTPATH)
	{
		GetCurrentPath(s->rbx);
	}
	else if(s->rax == UPDATEPATH)
	{
		UpdateCurrentPath(s->rbx);
	}
	else if(s->rax == UPDATE_ENV)
	{
		UpdateEnvPath(s->rbx);
	}
	else if(s->rax == READ)
	{
		s->rax = sys_read(s->rbx,s->rcx,s->rdx);		
	}
	else if(s->rax == OPEN)
	{
		s->rax=w_open(s->rbx,s->rcx);
	}
	else if(s->rax == WRITE)
	{
		sys_write(s->rbx,s->rcx,s->rdx);		
	}
	else if(s->rax == CLOSE)
	{
		s->rax = close(s->rbx);		
	}
	else if (s->rax == DISABLE_INTERRUPT)
	{
		IRQ_set_mask(0);
		//asm volatile("cli");
	}
	else if (s->rax == ENABLE_INTERRUPT)
	{
		IRQ_clear_mask(0);
		//asm volatile("sti");
	}
}

void sys_putchar(uint64_t c)
{
	//printf("I AM IN PUTCHAR\n");
	//putchar(c);		//calling putchar of write.c which is implemented for kernel
	printf((char *)c);

};

int sys_get_pid()
{
	//printf("I AM IN GETPID %d\n",current_task->pid);
	return  current_task->pid;
};

int sys_get_ppid()
{
	//printf("I AM IN GETPID %d\n",current_task->pid);
	return  current_task->ppid;
};


int sys_exit()
{
	//first i need to implement kfree() for freeing memory


	//again put the parent back in active queue if waitpid is called
	struct task_struct *parent = current_task->parent;

	if(parent != NULL)
	{

		int parentId = parent->pid;

		struct taskList *waitlist = waitQueue;

		while(waitlist != NULL && waitlist->task != NULL)
		{
			if(waitlist->task->pid == parentId )
			{
				if(waitlist->task->state == TASK_WAITING)
				{
					allProcesses = addToTailTaskList(allProcesses, parent);
					waitQueue = removeFromTaskList(waitQueue, parent);

					break;
				}
			}
			waitlist = waitlist->next;
		}
	}

	allProcesses = removeFromTaskList(allProcesses, current_task);
	deadQueue = addToTailTaskList(deadQueue, current_task);


	schedule();


	free_task_struct(current_task);
	//printf("Status %d\n",status);
	//we can free task_struct here or its better to do it in wait as mentioned in the mail thread..
	kfree((uint64_t)current_task, sizeof(struct task_struct));
	return 1;
	//schedule();
	
}





