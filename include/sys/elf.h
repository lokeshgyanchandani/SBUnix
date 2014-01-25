#ifndef _ELF_H
#define _ELF_H

#include <defs.h>

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff
#define EI_NIDENT 16

#define PF_R            0x4
#define PF_W            0x2
#define PF_X            0x1

//#define ELF_MAGIC 0x464C457FU        	/* "\x7FELF" in little endian */

typedef struct elf64_eheader
{
	unsigned char e_ident[16];		/* ELF identification */
	uint16_t e_type;			/* Object file type */
	uint16_t e_machine;		/* Machine type */
	uint32_t e_version;		/* Object file version */
	uint64_t e_entry;			/* Entry point address */
	uint64_t e_phoff;			/* Program header offset */
	uint64_t e_shoff;			/* Section header offset */
	uint32_t e_flags;			/* Processor-specific flags */
	uint16_t e_ehsize;			/* ELF header size */
	uint16_t e_phentsize;		/* Size of program header entry */
	uint16_t e_phnum;			/* Number of program header entries */
	uint16_t e_shentsize;		/* Size of section header entry */
	uint16_t e_shnum;			/* Number of section header entries */
	uint16_t e_shstrndx;		/* Section name string table index */
} elf64_eheader;

typedef struct elf64_sheader
{
	uint32_t sh_name; 			/* Section name */
	uint32_t sh_type; 			/* Section type */
	uint64_t sh_flags; 		/* Section attributes */
	uint64_t sh_addr; 			/* Virtual address in memory */
	uint64_t sh_offset; 		/* Offset in file */
	uint64_t sh_size; 		/* Size of section */
	uint32_t sh_link; 			/* Link to other section */
	uint32_t sh_info; 			/* Miscellaneous information */
	uint64_t sh_addralign; 		/* address alignment boundary */
	uint64_t sh_entsize; 		/* Size of entries, if section has table */
} elf64_sheader;

typedef struct elf64_pheader
{
	uint32_t p_type; 			/* Type of segment */
	uint32_t p_flags; 			/* Segment attributes */
	uint64_t p_offset; 			/* offset in file */
	uint64_t p_vaddr; 			/* Virtual address in memory */
	uint64_t p_paddr; 			/* Reserved */
	uint64_t p_filesz; 		/* Size of segment in file */
	uint64_t p_memsz; 		/* Size of segment in memory */
	uint64_t p_align; 		/* Alignment of segment */
} elf64_pheader;


void* readTarfile(char *fileName);
struct task_struct* load_elf(char *filename);
struct task_struct* load_elf_into_task(char *filename, struct task_struct * tsk);
int LoadDataInMemory(struct task_struct* tsk, uint64_t vaddr);
int LoadDataInMemory1(struct task_struct* tsk);
int CopyVmaData(uint64_t addr);
int CopyCodeData(uint64_t addr);
int checkforElf(char *filename);


//*******************************************************************for tarfs*******************************************
/*
 * ELF header at the beginning of the executable.
 */

#define EI_NIDENT 16

typedef struct {
  unsigned char ident[16];
  unsigned short type;
  unsigned short machine;
  unsigned int version;
  unsigned long entry;
  unsigned long phoff;
  unsigned long sphoff;
  unsigned int flags;
  unsigned short ehsize;
  unsigned short phentsize;
  unsigned short phnum;
  unsigned short shentsize;
  unsigned short shnum;
  unsigned short shstrndx;
} elfHeader;

/*
 * An entry in the ELF program header table.
 * This describes a single segment of the executable.
 */
typedef struct {
  uint32_t type;
  uint32_t flags;
  uint64_t offset;
  uint64_t vaddr;
  uint64_t paddr;
  uint64_t fileSize;
  uint64_t memSize;
  uint64_t alignment;
} programHeader;

/*
 * Bits in flags field of programHeader.
 * These describe memory permissions required by the segment.
 */
#define PF_R        0x4 /* Pages of segment are readable. */
#define PF_W        0x2 /* Pages of segment are writable. */
#define PF_X        0x1 /* Pages of segment are executable. */

/*
 * A segment of an executable.
 * It specifies a region of the executable file to be loaded
 * into memory.
 */
struct Exe_Segment {
  uint64_t offsetInFile; /* Offset of segment in executable file */
  uint64_t lengthInFile; /* Length of segment data in executable file */
  uint64_t startAddress; /* Start address of segment in user memory */
  uint64_t sizeInMemory; /* Size of segment in memory */
  int protFlags; /* VM protection flags; combination of VM_READ,VM_WRITE,VM_EXEC */
};

/*
 * Maximum number of executable segments we allow.
 * Normally, we only need a code segment and a data segment.
 * Recent versions of gcc (3.2.3) seem to produce 3 segments.
 */
#define EXE_MAX_SEGMENTS 100

/*
 * A struct concisely representing all information needed to
 * load an execute an executable.
 */
struct Exe_Format {
  struct Exe_Segment segmentList[EXE_MAX_SEGMENTS]; /* Definition of segments */
  int numSegments; /* Number of segments contained in the executable */
  uint64_t entryAddr; /* Code entry point address */
};
//********************************************************************************************************








#endif