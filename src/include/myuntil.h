#ifndef _UNTIL_H_
#define _UNTIL_H_

#define BOOL  int
#define True  1
#define False 0
#define N 100

#include "simple-tdbg.h"

size_t breakpoint_injection(pid_t child, size_t addr);

int is_prefix(char* cmd, char* target);

int parse_break_cmd(pid_t child, char * cmd);

int is_exits_func(char* func);

int execture_instruction(pid_t child, int num);

int parse_stepi_cmd(pid_t child, char* cmd);

void run(pid_t child);

int parse_until_cmd(pid_t child, char * cmd);

int execture_until(pid_t child, int num);

int Run(pid_t child);

int delete_bp_by_idx(pid_t child, int idx);

int parse_clear_cmd(pid_t child );

int parse_delete_cmd(pid_t child, char* cmd);

int parse_info_cmd(pid_t child, char* cmd);

void show_breakpoints();

void show_registers(pid_t child);

#endif  // _UNTIL_H_