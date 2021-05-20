#include"include/simple-tdbg.h"
#include"include/history.h"

int bp_count = 0;
BreakPoint bp_list[N];    //maxmum number of breakpoint is N=100;


int func_count = 0;  // function number
FuncList* func_lists = NULL;	  // function lists


int main(int argc, char **argv){
	pid_t child;
	int status;
	int hit_index;

	char command[N];
    if(argc<2){
        printf("Please input the file you wanna trace :-)\n");
		printf("usage: tdb <tracee name>\n");
        exit(0);
    }
	child = fork();

	if(child == 0){
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl(argv[1],argv[1],NULL);
		perror("fail exec");
		exit(-1);
	}
	else{
		printf("***************************************************************\n");
		printf("*************[+] This is simple-tdbg v0.01 ********************\n");
		printf("***************************************************************\n\n");

		extract_ELF_format(argv[1]);   //parse the elf64 file
		wait(NULL);

		while(1){
			printf("simple-tdbg> ");

			get_input_cmd(command);
			add_cmd(command);			

			if(is_prefix(command, "break")){
				int flag = parse_break_cmd(child, command);
				if(!flag){
					printf("param error! : break [<function name> | breakpoint address]\n");
				}
				
			}else if(is_prefix(command, "stepi")){
				int flag = parse_stepi_cmd(child, command);
				if(flag<0){
					printf("tracee died, exited\n");
					goto QUIT;
				}
				if(!flag){
					printf("param error! :stepi   [<num>]	#execute one/num instructions\n");
				}
			}else if(is_prefix(command, "until")){
				int flag = parse_until_cmd(child, command);
				if(flag<0){
					printf("tracee died, exited\n");
					goto QUIT;
				}
				if(!flag){
					printf("param error! :until   [<num>]   #continue executing until program hits breakpoint num\n");
				}
			}else if(is_prefix(command, "run")){
				int flag = Run(child);
				if(flag<0){
					printf("tracee died, exited\n");
					goto QUIT;
				}
			}else if(is_prefix(command, "info")){
				int flag = parse_info_cmd(child, command);
			}else if(is_prefix(command, "print")){
				printf("This command is not supported at this time.\n");
			}else if(is_prefix(command, "x")){
				printf("This command is not supported at this time.\n");
			}else if(is_prefix(command, "delete")){
				int flag = parse_delete_cmd(child, command);
				if(!flag){
					printf("delete  <num>    #delete the breakpoints who`s index is num\n");
				}
				
			}else if(is_prefix(command, "clear")){
				int flag = parse_clear_cmd(child);
				
			}else if(is_prefix(command, "help")){
				printf("*************************** help documents ****************************\n");
				printf("quit/exit                       #quit the software\n");
				printf("break <func_name> | <bp_addr>   #set breakpoint\n");
				printf("delete  <num>                   #delete the breakpoints who`s index is num\n");
				printf("clear                           #clear all breakpoints\n");
				printf("stepi   [<num>]                 #execute one/num instructions\n");
				printf("until   [<num>]                 #continue executing until program hits breakpoint num\n");
				printf("info  <breakpoints>|<regs>      #display breakpoints or registers\n");
				printf("help                            #get help infomation\n\n");
				printf("************************* help documents end****************************\n");
				
			}else if(is_prefix(command, "quit") || is_prefix(command, "exit")){
				QUIT:
				//TODO  other thing
				// free malloc memory
				free(func_lists);
				//free(his.cmds);
				printf("I want to stop the process!\n");
				clear_all_cmdline();
				printf("Congration! you have stoped the process\n");
				break;
			}else{
				printf("Illegal command.\n");
			}
			command[0] = 0;
		}
	
	return 0;
	}
}