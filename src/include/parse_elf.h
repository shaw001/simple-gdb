#ifndef __PARSE_ELF_H__
#define __PARSE_ELF_H__
#include "simple-tdbg.h"

int extract_ELF_format(char* file);

int parse_elf_exec(Elf64_Ehdr* header, FILE* fp);

#endif  //__PARSE_ELF_H__