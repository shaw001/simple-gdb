#ifndef __SIMPLE_TDBG_H__
#define __SIMPLE_TDBG_H__

#include<stdio.h>
#include<unistd.h>
#include<sys/ptrace.h>
#include<stdlib.h>
#include<elf.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/user.h>

#include "myuntil.h"
#include "parse_elf.h"

#define DEBUG
#define INT_3 0xCC
#define N 100
#define cmd_N 50

#define UP    "\033[A"
#define Down  "\033[B"

typedef struct func_addr_list{
	size_t  addr;
	char 	name[25];
	size_t  orig_code;
}FuncList;

typedef struct breakpoint{
	int     idx;	  //the id of breakpoints
	int     name_idx; //the index im the func_addr_list, or -1 when the brakpoint is addr;
	size_t  addr;     //breakpoint's address
    size_t  orig_code; //save the orig_code in addr
	int     is_valid;  // vaild:1, invaild:0
}BreakPoint;

struct line_cmd{
	char cmd[cmd_N];
};

extern FuncList* func_lists;
extern int func_count;
extern int bp_count;
extern BreakPoint bp_list[N];

#endif //__SIMPLE_TDBG_H__