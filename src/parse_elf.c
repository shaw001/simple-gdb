
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<elf.h>
#include<string.h>
#include<sys/user.h>
#include "include/parse_elf.h"


int extract_ELF_format(char* file){
	Elf64_Ehdr e_header;

	FILE *fp = fopen(file, "r");
	if(!fp){
		perror("software[tracee] open failed\n");
        exit(-1);
	}

	//read e_header
	fread(&e_header, sizeof(Elf64_Ehdr), 1, fp);
	switch(e_header.e_type){
		case ET_EXEC:{
			//call the func to parse e_header
			func_count = parse_elf_exec(&e_header, fp);
			fclose(fp);
			if(func_count < 0){
				exit(-1);
			}
			break;
		}
		case ET_DYN:{
			//TODO -> support the DYN file
			perror("don't support the DYN(Shared object file).\n");
			fclose(fp);
        	exit(-1);
		}
		case ET_REL:{
			perror("don't support the REL(Relocatable file).\n");
			fclose(fp);
        	exit(-1);
		}
	}
}


int parse_elf_exec(Elf64_Ehdr* header, FILE* fp){
    Elf64_Shdr section_header; 
    fseek(fp, header->e_shoff, SEEK_SET);

    int _func_count = 0;   // the number of functions
	fseek(fp, header->e_shoff, SEEK_SET);

    for(int i=0;i < header->e_shnum;i++){
	/* scan section header */
	
		fread(&section_header, sizeof(Elf64_Shdr), 1, fp); 
		if(section_header.sh_type == SHT_SYMTAB){	// if this section is symbol table?
			Elf64_Shdr str_section_header;
			int sym_entry_count=0;
			size_t str_table_offset = header->e_shoff + section_header.sh_link * sizeof(Elf64_Shdr);		// section_header.sh_link 输出为 30，也就是symbol table在段表中的索引。
			fseek(fp,str_table_offset,SEEK_SET);					//定位到字符串表
			fread(&str_section_header, sizeof(Elf64_Shdr), 1, fp);	//读取字符串表表头 
			
			fseek(fp,section_header.sh_offset,SEEK_SET);			//定位到符号表表头
			sym_entry_count = section_header.sh_size/section_header.sh_entsize;
			//printf("[*] %d entries in symbol table\n",sym_entry_count);

            

            for(int i=0;i<sym_entry_count; i++){
				//符号表中每一个元素是一个 Elf64_Sym
				Elf64_Sym sym;
				fread(&sym, sizeof(Elf64_Sym), 1,fp);				//每次读一个Symbol
				if(ELF64_ST_TYPE(sym.st_info) == STT_FUNC && sym.st_name!=0 && sym.st_value != 0){
					_func_count++;
				}
			}

            func_lists = (FuncList*) malloc(sizeof(func_lists) * func_count);
            if(func_lists == NULL){
                perror("malloc func_lists failed.");
                return -1;
            }

			fseek(fp,section_header.sh_offset,SEEK_SET);			//定位到符号表表头

            int idx = 0;

			for(int i=0;i<sym_entry_count; i++){
				//符号表中每一个元素是一个 Elf64_Sym
				Elf64_Sym sym;
				fread(&sym, sizeof(Elf64_Sym), 1,fp);				//每次读一个Symbol
				if(ELF64_ST_TYPE(sym.st_info) == STT_FUNC && sym.st_name!=0 && sym.st_value != 0){
					/* 如果该符号是一个函数或其他可执行代码，在字符串表中，且虚拟地址不为0 */
					long file_ops = ftell(fp);										//保存此时fp的位置
					fseek(fp,str_section_header.sh_offset+sym.st_name,SEEK_SET);	//定位到字符串表中对应符号的位置
					func_lists[idx].addr = sym.st_value;
					fread(func_lists[idx++].name,25,sizeof(char),fp);						//读取对应的符号
					fseek(fp,file_ops,SEEK_SET);									//恢复fp到上一次读取的Symbol的位置，准备下一次读取。
				}
			}

            break;
		}
	}
    return _func_count;
}