#ifndef _TARFS_H
#define _TARFS_H

#include <sys/sched.h>

extern char _binary_tarfs_start;
extern char _binary_tarfs_end;

#define ELF_MAGIC 0x464C457F
#define ELF_PROG_LOAD	 1
#define DIRECTORY 5
#define FILE_TYPE 0



struct posix_header_ustar {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char typeflag[1];
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char pad[12];
};



typedef struct {
   char name[100];
   int size;
   int typeflag;
   uint64_t addr_hdr;
   int par_ind;
} tarfs_table_col;

extern tarfs_table_col tarfs_table[100];
extern int tarfs_row_count;

/*
int get_per_ind(char *dir,int flag);
int get_per_ind_file(char *dir);
uint64_t is_file_exists(char* filename);
uint64_t open_dir(char * dir);
uint64_t read_dir(char * dir);
uint64_t open(char * file);
int read_file(uint64_t file_addr, int size, uint64_t buf);
void tarfs_table_initialise(char *binary_tarfs_start);
void print_file_table(tarfs_table_col *table);
*/
#endif
