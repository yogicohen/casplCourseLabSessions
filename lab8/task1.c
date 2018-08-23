#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <sys/mman.h>
#define TRUE 1

int Currentfd;
Elf64_Ehdr *header;
void *map_start;
struct stat fd_stat;

void Examine(){
    char filename[LINE_MAX];
    char InputBuffer[LINE_MAX];
    printf("Which file would you like to examine?\n");
    
    fgets(InputBuffer,LINE_MAX,stdin);
    sscanf(InputBuffer,"%s", filename);
    
    if (Currentfd != -1)
        close(Currentfd);
    
    if((Currentfd = open(filename, O_RDONLY)) < 0 ) {
      perror("error in open");
      exit(-1);
   }
   
   if(fstat(Currentfd, &fd_stat) != 0 ) {
      perror("stat failed");
      exit(-1);
   }
   
   if ((map_start = mmap(0, fd_stat.st_size, PROT_READ , MAP_SHARED, Currentfd, 0)) == MAP_FAILED ) {
      perror("mmap failed");
      close(Currentfd);
      Currentfd = -1;
      exit(-4);
   }

   header = (Elf64_Ehdr *) map_start;
   
   if(header->e_ident[EI_MAG1] != 69 || header->e_ident[EI_MAG2] != 76 || header->e_ident[EI_MAG3] != 70){
        printf("%s","ERROR: non ELF file\n");
        munmap(map_start, fd_stat.st_size);
        close(Currentfd);
        Currentfd = -1;
        return;
   }
   
   printf("ELF Header:\n");
   
   printf("  %-35s %c %c %c\n","Magic:",header->e_ident[EI_MAG1],header->e_ident[EI_MAG2],header->e_ident[EI_MAG3]);
   
   switch(header->e_ident[EI_DATA]){
       case 1:
           printf("  %-35s %s","Data encoding:", "Little Endian\n");
           break;
       case 2:
           printf("  %-35s %s","Data encoding:", "Big Endian\n");
           break;
       default://shouldn't get to this
           break;
   }

   printf("  %-35s 0x%lx\n","Entry point address:",header->e_entry);   
   
   printf("  %-35s %ld (bytes into file)\n","Section header table offset:",header->e_shoff);
   
   printf("  %-35s %d\n","Number of section header entries:",header->e_shnum);
   
   printf("  %-35s %d (bytes)\n","Size of section headers:",header->e_shentsize);
   
   printf("  %-35s %ld (bytes into file)\n","Program header table offset:",header->e_phoff);
   
   printf("  %-35s %d\n","Number of program header entries:",header->e_phnum);
   
   printf("  %-35s %d (bytes)\n\n","Size of program headers:",header->e_phentsize);
   
   return;
}

char* getTypeName(unsigned int typeNum){
    switch(typeNum){
            case 0:
                return "NULL";
            case 1:
                return "PROGBITS";
            case 2:
                return "SYMTAB";
            case 3:
                return "STRTAB";
            case 4:
                return "RELA";
            case 5:
                return "HASH";
            case 6:
                return "DYNAMIC";
            case 7:
                return "NOTE";
            case 8:
                return "NOBITS";
            case 9:
                return "REL";
            case 10:
                return "SHLIB";
            case 11:
                return "DYNSYM";
            case 14:
                return "INIT_ARRAY";
            case 15:
                return "FINI_ARRAY";
            case 16:
                return "PREINIT_ARRAY";
            case 17:
                return "GROUP";
            case 18:
                return "SYMTAB_SHNDX";
            case 19:
                return "NUM";
            case 0x60000000:
                return "LOOS";
            case 0x6ffffff5:
                return "GNU_ATTRIBUTES";
            case 0x6ffffff6:
                return "GNU_HASH";
            case 0x6ffffff7:
                return "GNU_LIBLIST";
            case 0x6ffffff8:
                return "CHECKSUM";
            case 0x6ffffffa:
                return "LOSUNW";
            case 0x6ffffffb:
                return "SUNW_COMDAT";
            case 0x6ffffffc:
                return "SUNW_syminfo";
            case 0x6ffffffd:
                return "GNU_verdef";
            case 0x6ffffffe:
                return "GNU_verneed";
            case 0x6fffffff:
                return "HIOS";
            case 0x70000000:
                return "LOPROC";
            case 0x7fffffff:
                return "HIPROC";
            case 0x80000000:
                return "LOUSER";
            case 0x8fffffff:
                return "HIUSER";
            default:
                return "TYPE_DOESN'T_EXIST";
        }
}

void PrintSectionNames(){
    if(Currentfd == -1){
        printf("ERROR: Currentfd not valid\n");
        return;
    }
    
    int i;
    Elf64_Shdr *section_header = (Elf64_Shdr*)(map_start + header->e_shoff);
    Elf64_Shdr *shstrnTable =(void *) header + header->e_shoff + header->e_shstrndx*(sizeof(Elf64_Shdr));
    char* section_name;
    char* type_string;
    printf("Section Headers:\n");
    printf("  %s %-18s %-16s %-8s %-16s %s\n","[Nr]","Name","Address","Offset","Size","Type");
    
    for(i=0 ; i<header->e_shnum ; ++i){
        printf("  [%2d]",i);
        section_name = (void *) map_start + shstrnTable->sh_offset + section_header[i].sh_name;
        printf(" %-18s",section_name);
        printf(" %016lx",section_header[i].sh_addr);
        printf(" %08lx",section_header[i].sh_offset);
        printf(" %016lx",section_header[i].sh_size);
        type_string = getTypeName(section_header[i].sh_type);
        printf(" %s\n",type_string);
    }
    printf("\n");
}

void Quit(){
    munmap(map_start, fd_stat.st_size);
    exit(0);
}

int main(int argc, char **argv) {
    Currentfd = -1;
    int i;
    int UserChoice;
    char input[LINE_MAX];
    void (*functionArr[3])();
    
    functionArr[0] = &Examine;
    functionArr[1] = &PrintSectionNames;
    functionArr[2] = &Quit;
    
    const char* UserMenu[4];
    UserMenu[0] = "Choose action:\n";
    UserMenu[1] = "1-Examine ELF File\n";
    UserMenu[2] = "2-Print Section Names\n";
    UserMenu[3] = "3-Quit\n";
    
    while(TRUE){
        for (i=0 ; i<4 ; ++i)
            printf("%s",UserMenu[i]);
        fgets(input,LINE_MAX,stdin);
        sscanf(input,"%d", &UserChoice);
        if(UserChoice < 1 || UserChoice > 3){
          printf("%s","ERROR: Choosen action is not available\n");
          continue;
        }
        functionArr[UserChoice-1]();
    }
    
    return 0;
}