#include "include/history.h"

static cmd_node* cmd_head=0, *cmd_end=0, *cmd_cur = 0;
static unsigned int cmd_number = 0;

void init_history(){
    cmd_end = (cmd_node*)malloc(sizeof(cmd_node));
    cmd_head = (cmd_node*)malloc(sizeof(cmd_node));
    memset(cmd_head, 0, sizeof(cmd_head));
    memset(cmd_end, 0, sizeof(cmd_end));
    cmd_end->prev = cmd_head;
    cmd_head->next = cmd_end;
    cmd_end->next = NULL;
    cmd_head->prev = NULL;
    cmd_cur = cmd_end;
}

// pack a function like getch() in conio.h
char getch(){
    char c;
    system("stty -echo");
    system("stty -icanon");
    c = getchar();
    system("stty icanon");
    system("stty echo");
    return c;
}

char getche(){
    char c;
    system("stty -icanon");
    c = getchar();
    system("stty icanon");
    return c;
}

void mycpy(char* dst, char* src){
    memset(dst, 0, 32);
    size_t n = MIN(strlen(src),CMD_LEN-1);
    for(int i=0; i<n; ++i){
        if(src[i]==0 || src[i]=='\n'){
            dst[i] = 0;
            break;
        }else{
            dst[i] = src[i];
        }
    }
}

//add cmdline to history_cmd
void add_cmd(char* cmd){
    cmd_node* node = (cmd_node*)malloc(sizeof(cmd_node));
    mycpy(node->cmd, cmd);
    node->prev = NULL;
    node->next = NULL;
    if(!cmd_head){
        init_history();
    }
    node->prev = cmd_end->prev;
    cmd_end->prev->next = node;
    node->next = cmd_end;
    cmd_end->prev = node;

    cmd_cur = cmd_end;
    cmd_number++;

    check_cmd_number();
}

void clear_a_hostory(){
    if(cmd_head->next != cmd_end){
        cmd_node* node = cmd_head->next;
        if(cmd_cur == node) cmd_cur = node->next;
        cmd_head->next = node->next;
        node->next->prev = cmd_head;
        free(node);
        cmd_number--;
    }
    
}
// delete those 
void check_cmd_number(){
    if(cmd_number<=CMD_LEN){
        return;
    }
    clear_a_hostory();
}

void clear_cmdline(){
    printf("\r");
    CLEAR_LINE();
    printf("simple-tdbg> ");
}

void clear_all_cmdline(){
    while(cmd_number>0){
        clear_a_hostory();
    }
    free(cmd_head);
    free(cmd_end);
}

void up_scroll(char* cmd){
    clear_cmdline();
    if(!cmd_cur){
        cmd[0] = 0;
        return;
    }else if(!cmd_cur->prev){
        mycpy(cmd,cmd_cur->cmd);
        return;
    }else{
        cmd_cur = cmd_cur->prev;
        mycpy(cmd,cmd_cur->cmd);
        return;
    }
}

void down_scroll(char* cmd){
    clear_cmdline();
    if(!cmd_cur){
        cmd[0] = 0;
        return;
    }else if(!cmd_cur->next){
        mycpy(cmd,cmd_cur->cmd);
        return;
    }else{
        cmd_cur = cmd_cur->next;
        mycpy(cmd,cmd_cur->cmd);
        return;
    }
}

void left_scroll(int cur, int len){
    if(cur>0) MOVELEFT(1);
}

void right_scroll(int cur, int len){
    if(cur<len) MOVERIGHT(1);
}

void show_n_cmdline(char* command, int index){
    printf("\r");
    CLEAR_LINE();
    printf("simple-tdbg> ");
    command[index] = 0;
    for(int i=0; i<index; ++i){
        putc(command[i], stdout);
    }
}

void insert_char(char* command, char c, int cur, int len){
    for(int i=len-1; i>=cur; --i){
        command[i+1] = command[i];
    }
    command[cur] = c;
}

void del_char(char* command, int cur, int len){
    if(cur == len) command[cur-1] = 0;
    else{
        for(int i=cur; i<=len; ++i){
            command[i-1] = command[i];
        }
    }
}

void get_input_cmd(char* command){
    int cur=0,len=0;
		char tmp_c;
		while(1){
			tmp_c = getch();
			insert_char(command, tmp_c, cur++, len++);
			switch(tmp_c){
				case 9: {
					del_char(command, cur--, len--);
					break;  //tab
				}
				case 27: {      // esc or other constrl key
					del_char(command, cur--, len--);
					tmp_c = getch();
					if(tmp_c == 91){
						tmp_c = getch();
						switch(tmp_c){
							case UP_OP:{
								up_scroll(command);
								printf("%s",command);
								len = strlen(command);
								cur = len;
								break;
							}
							case DOWN_OP:{
								down_scroll(command);
								len = strlen(command);
								cur = len;
								break;
							}
							case LEFT_OP:{
								left_scroll(cur,len);
								if(cur>0) cur--;
								continue;
							}
							case RIGHT_OP:{
								right_scroll(cur,len);
								if(cur<len) cur++;
								continue;
							}
							default:{
								break;
							}
						}
						//printf("%s\n", command);
					}else{
						continue;
					}
					break;
				}
				case 127:{		// backspace
					del_char(command, cur--, len--);
					if(cur>0){
						del_char(command, cur--, len--);
					}
					break;
				}
				case 10:{       //enter
					printf("\n");
					return;
				}
			}
			show_n_cmdline(command, len);
			MOVELEFT(len-cur);
			if(cur==len) MOVERIGHT(1);
		}
}