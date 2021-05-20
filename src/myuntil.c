#include<sys/ptrace.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include "include/myuntil.h"

#define BOOL  int
#define True  1
#define False 0

size_t breakpoint_injection(pid_t child, size_t addr){
	/* we inject the [0xcc](int 3) in the addr
        return : orig_code*/

	//使用ptrace读出一个字节存在orgi code中
	size_t orig_code = ptrace(PTRACE_PEEKTEXT, child, addr,0);

	#ifdef DEBUG
	printf("[*] Set Breakpoint %d in address: 0x%lx\n", bp_count, addr);
	#endif

	ptrace(PTRACE_POKETEXT, child, addr, (orig_code & 0xFFFFFFFFFFFFFF00) | INT_3);	//将最低为的字节打入 int 3

	// #ifdef DEBUG
	// check_bp(child);
	// #endif

    return orig_code;
}

int is_prefix(char* cmd, char* target){
	size_t n = strlen(cmd), m = strlen(target);
	if(n < m) return 0;

	for(int i=0; i<m; ++i){
		if(cmd[i] != target[i]) return 0;
	}
	return 1;
}

BOOL parse_break_cmd(pid_t child, char* cmd){
	BOOL flag = False;
	int idx = 5;
	size_t n = strlen(cmd);
	while(idx < n-1 && cmd[idx]==' ') idx++;

	if(idx>=n || (idx>=n-2)&&cmd[idx]=='0') return False;
	if(is_prefix(cmd+idx, "0x") || is_prefix(cmd+idx, "0X")){
		size_t addr = 0;
		sscanf(cmd+idx, "%lx", &addr);
		#ifdef DEBUG
		printf("%s -> %ld -> %lx", cmd+idx, addr, addr);
		#endif
		size_t orig_code = breakpoint_injection(child, addr);
		
		bp_list[bp_count].idx = bp_count;
		bp_list[bp_count].name_idx = -1;
		bp_list[bp_count].addr = addr;
		bp_list[bp_count].orig_code = orig_code;
		bp_list[bp_count].is_valid = 1;

		bp_count++;
		flag = True;
	}else if(is_exits_func(cmd+idx) != -1){
		int func_idx = is_exits_func(cmd+idx);
		size_t addr = func_lists[func_idx].addr;

		#ifdef DEBUG
		printf("%s -> %s -> %lx", cmd+idx, func_lists[func_idx].name, addr);
		#endif

		size_t orig_code = breakpoint_injection(child, addr);
		
		bp_list[bp_count].idx = bp_count;
		bp_list[bp_count].name_idx = func_idx;
		bp_list[bp_count].addr = addr;
		bp_list[bp_count].orig_code = orig_code;
		bp_list[bp_count].is_valid = 1;
		bp_count++;
		flag = True;
	}
	return flag;
}

//if exits return the index, or -1
int is_exits_func(char* func){
	int m;
	for(int i=0; i<func_count; ++i){
		m = strlen(func_lists[i].name);
		int j = 0;
		for(; j<m; ++j){
			if(func[j] != func_lists[i].name[j]) break;
		}
		if(j == m) return i;
	}
	return -1;
}

int execture_instruction(pid_t child, int num){
	int status;
	for(int i=0; i<num; ){
		ptrace(PTRACE_SINGLESTEP,child,0,0);
		//wait(NULL);
		waitpid(child,&status,0);		//等待子进程的信号

		/* 捕获信号之后判断信号类型	*/
		if(WIFEXITED(status)){
			/* 如果是EXit信号 */
			printf("\n[+] Child process EXITED!\n");
			return -1;
		}
		if(WIFSTOPPED(status))
		{
			#ifdef DEBUG
			printf("stepi: %d\n",i+1);
			#endif
			i++;
		}
	}
	return 1;
}

int parse_stepi_cmd(pid_t child, char* cmd){
	int flag = 0;
	int idx = 5;
	size_t n = strlen(cmd);

	while(idx < n-1 && cmd[idx]==' ') idx++;

	if(cmd[idx]>'0' && cmd[idx]<='9'){
		int num = 0;
		while(idx<n && cmd[idx]>='0' && cmd[idx]<='9'){
			num = num*10 + (cmd[idx]-'0');
			idx++;
		}
		flag = execture_instruction(child, num);
	}else{
		flag = execture_instruction(child, 1);
	}
	return flag;
}

void run(pid_t child){
	ptrace(PTRACE_CONT,child,0,0);
}

int if_bp_hit(struct user_regs_struct regs)
{
		for(int i=0;i<bp_count;i++)
		{
			if(bp_list[i].addr==(regs.rip-1) && bp_list[i].is_valid)
			{
				#ifdef DEBUG
				printf("Hit Breakpoint: 0x%lx\n",bp_list[i].addr);
				#endif
				return i;
			}
		}
		return -1;
}


int execture_until(pid_t child, int num){
	int status = 0;
	struct user_regs_struct regs;
	if(num >= bp_count || bp_list[num].is_valid==0){
		printf("breakpoint index error!\n");
		return 0;
	}
	size_t target_addr = bp_list[num].addr;
	run(child);
	while(1){
		waitpid(child,&status,0);		//等待子进程的信号
			
		/* 捕获信号之后判断信号类型	*/
		if(WIFEXITED(status)){
			/* 如果是EXit信号 */
			printf("\n[+] Child process EXITED!\n");
			return -1;
		}
		if(WIFSTOPPED(status))
		{
			/* 如果是STOP信号 */
			if(WSTOPSIG(status)==SIGTRAP)
			{				//如果是触发了SIGTRAP,说明碰到了断点
				ptrace(PTRACE_GETREGS,child,0,&regs);	//读取此时用户态寄存器的值，准备为回退做准备
				
				if(target_addr==(regs.rip-1))
				{
					/*如果命中*/
					/*输出命中信息*/
					//printf("%s()\n",bp_list[hit_index].name);
					/*把INT 3 patch 回本来正常的指令*/
					ptrace(PTRACE_POKETEXT,child,bp_list[num].addr,bp_list[num].orig_code);
					/*执行流回退，重新执行正确的指令*/
					regs.rip = bp_list[num].addr;
					ptrace(PTRACE_SETREGS,child,0,&regs);
					/*单步执行一次，然后恢复断点*/
					ptrace(PTRACE_SINGLESTEP,child,0,0);
					wait(NULL);
					/*恢复断点*/
					ptrace(PTRACE_POKETEXT, child, bp_list[num].addr, (bp_list[num].orig_code & 0xFFFFFFFFFFFFFF00) | INT_3);
					return 1;
				}
			}	
		}
	}
}

int parse_until_cmd(pid_t child, char * cmd){
	int flag = 0;
	int idx = 5, num = 0;
	size_t n = strlen(cmd);

	while(idx < n-1 && cmd[idx]==' ') idx++;

	if(idx>=n) return flag;
	while(idx<n && cmd[idx]>='0' && cmd[idx]<='9'){
			num = num*10 + (cmd[idx]-'0');
			idx++;
	}

	flag  = execture_until(child, num);
	return flag;
}

int Run(pid_t child){
	int status = 0, flag = 0, hit_index;
	struct user_regs_struct regs;
	run(child);
	puts("[+] Start");
	while(1){
		//puts(1);
		waitpid(child,&status,0);		//等待子进程的信号
		
		/* 捕获信号之后判断信号类型	*/
		if(WIFEXITED(status)){
			/* 如果是EXit信号 */
			printf("\n[+] Child process EXITED!\n");
			return -1;
		}
		if(WIFSTOPPED(status))
		{
			/* 如果是STOP信号 */
			if(WSTOPSIG(status)==SIGTRAP)
			{				//如果是触发了SIGTRAP,说明碰到了断点
				ptrace(PTRACE_GETREGS,child,0,&regs);	//读取此时用户态寄存器的值，准备为回退做准备
				//printf("[+] SIGTRAP rip:0x%llx\n",regs.rip);
				/* 将此时的addr与我们bp_list中维护的addr做对比，如果查找到，说明断点命中 */
				if((hit_index=if_bp_hit(regs))==-1)
				{
					/*未命中*/
					printf("MISS, fail to hit:0x%llx\n",regs.rip);
					exit(-1);
				}
				else
				{
					/*如果命中*/
					/*输出命中信息*/
					printf("%s()\n",func_lists[bp_list[hit_index].name_idx].name);
					/*把INT 3 patch 回本来正常的指令*/
					ptrace(PTRACE_POKETEXT,child,bp_list[hit_index].addr,bp_list[hit_index].orig_code);
					/*执行流回退，重新执行正确的指令*/
					regs.rip = bp_list[hit_index].addr;
					ptrace(PTRACE_SETREGS,child,0,&regs);
					/*单步执行一次，然后恢复断点*/
					ptrace(PTRACE_SINGLESTEP,child,0,0);
					wait(NULL);
					/*恢复断点*/
					ptrace(PTRACE_POKETEXT, child, bp_list[hit_index].addr, (bp_list[hit_index].orig_code & 0xFFFFFFFFFFFFFF00) | INT_3);
					flag = 1;
					break;
				}
			}	
		}
		ptrace(PTRACE_CONT,child,0,0);
	}
	return flag;
}

int delete_bp_by_idx(pid_t child, int idx){
	if(idx<0 || idx>=bp_count || bp_list[idx].is_valid==0){
		return -1;
	}else{
		bp_list[idx].is_valid=0;
		ptrace(PTRACE_POKETEXT,child,bp_list[idx].addr,bp_list[idx].orig_code);
		return 1;
	}
}

int parse_delete_cmd(pid_t child, char* cmd){
	int flag = 0;
	int idx = 6, num = -1;
	size_t n = strlen(cmd);

	while(idx < n && cmd[idx]==' ') idx++;

	if(idx>=n) return flag;

	if(cmd[idx]>='0' && cmd[idx]<='9') num = 0;

	while(idx<n && cmd[idx]>='0' && cmd[idx]<='9'){
			num = num*10 + (cmd[idx]-'0');
			idx++;
	}

	flag  = delete_bp_by_idx(child, num);
	if(flag<0) printf("No breakpoint number %d\n", num);
	return flag;

}

int parse_clear_cmd(pid_t child){
	printf("Delete all breakpoints? (y or n) ");
	char ans[100];
	fgets(ans, 100, stdin);
	switch(ans[0]){
		case 'Y':
		case 'y':{
			for(int i=0; i<bp_count; ++i){
				delete_bp_by_idx(child, i);
			}
			break;
		}
		case 'N':
		case 'n':{
			break;
		}
	}
	return 1;
}

void show_breakpoints(){
	printf("***************** Breakpoints Lists ****************\n");
	printf("Num\t Address\t Name\n");
	char name[25] = "--";
	for(int i=0; i<bp_count; ++i){
		if(bp_list[i].is_valid){
			int idx = bp_list[i].name_idx;
			printf("%d\t 0x%lx\t %s\n", bp_list[i].idx, bp_list[i].addr, idx>=0? func_lists[idx].name : name);
		}
	}
}

void show_registers(pid_t child){
	struct user_regs_struct regs;
	ptrace(PTRACE_GETREGS,child,0,&regs);

	printf("******************************* Register State *******************************\n");
	printf("r15:     0x%-16lld  r14: 0x%-16lld  r13: 0x%-12lld\n", regs.r15, regs.r14, regs.r13);
	printf("r12:     0x%-16lld  rbp: 0x%-16lld  rbx: 0x%-12lld\n", regs.r12, regs.rbp, regs.rbx);
	printf("r11:     0x%-16lld  r10: 0x%-16lld  r9: 0x%-12lld\n", regs.r11, regs.r10, regs.r9);
	printf("r8:      0x%-16lld  rax: 0x%-16lld  rcx: 0x%-12lld\n", regs.r8, regs.rax, regs.rcx);
	printf("rdx:     0x%-16lld  rsi: 0x%-16lld  rdi: 0x%-12lld\n", regs.rdx, regs.rsi, regs.rdi);
	printf("rip:     0x%-16lld  cs:  0x%-16lld  eflags: 0x%-12lld\n", regs.rip, regs.cs, regs.eflags);
	printf("rsp:     0x%-16lld  ss:  0x%-16lld  fs_base: 0x%-12lld\n", regs.rsp, regs.ss, regs.fs_base);
	printf("gs_base: 0x%-16lld  ds:  0x%-16lld  es: 0x%-12lld\n", regs.gs_base, regs.ds, regs.es);
	printf("fs:      0x%-16lld  gs:  0x%-16lld\n", regs.fs, regs.gs);
}

int parse_info_cmd(pid_t child, char* cmd){
	int flag = 0;
	int idx = 4;
	size_t n = strlen(cmd);
	while(idx < n && cmd[idx]==' ') idx++;

	if(idx>=n) return flag;   // params error!
	flag = 1;
	if(is_prefix(cmd+idx, "breakpoints")){
		show_breakpoints();
	}else if(is_prefix(cmd+idx, "registers")){
		show_registers(child);
	}else{
		flag = 0;
	}
	return flag;
}