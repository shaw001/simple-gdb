#ifndef __HISTORY_H__
#define __HISTORY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOVELEFT(y) printf("\033[%dD",(y))
#define MOVERIGHT(y) printf("\033[%dC",(y))
#define CLEAR_LINE() printf("\033[K")

#define RESET_CURSOR() printf("\033[H")

#define MIN(x,y) ((x)<(y)? (x) : (y))

#define MAX_CMD_NUM 200
#define CMD_LEN 32

#define UP_OP 65
#define DOWN_OP 66
#define LEFT_OP 68
#define RIGHT_OP 67

typedef struct history_cmd{
    char cmd[CMD_LEN];
    struct history_cmd* prev,*next;
} cmd_node;

void init_history();

// pack a function like getch() in conio.h
char getch();

char getche();

//add cmdline to history_cmd
void add_cmd(char* cmd);

void clear_a_hostory();
// delete those 
void check_cmd_number();

void clear_cmdline();

void clear_all_cmdline();

void up_scroll(char* cmd);

void down_scroll(char* cmd);

void left_scroll(int cur, int len);

void right_scroll(int cur, int len);

void show_n_cmdline(char* command, int index);

void insert_char(char* command, char c, int cur, int len);

void del_char(char* command, int cur, int len);

void get_input_cmd(char* command);

#endif  //__HISTORY_H__