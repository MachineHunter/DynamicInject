#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char g_path[100] = "none";
unsigned long g_pid = 0;


int existFile(const char* path)
{
	FILE* fp = fopen(path, "r");
	if (fp == NULL) return 0;
	fclose(fp);
	return 1;
}

void print_logo() {
	system("cls");
	printf("\n");
	printf("\x1b[31m ____                              _        ___        _           _   \n");
	printf("\x1b[31m|  _ \\ _   _ _ __   __ _ _ __ ___ (_) ___  |_ _|_ __  (_) ___  ___| |_ \n");
	printf("\x1b[31m| | | | | | | '_ \\ / _` | '_ ` _ \\| |/ __|  | || '_ \\ | |/ _ \\/ __| __|\n");
	printf("\x1b[31m| |_| | |_| | | | | (_| | | | | | | | (__   | || | | || |  __/ (__| |_ \n");
	printf("\x1b[31m|____/ \\__, |_| |_|\\__,_|_| |_| |_|_|\\___| |___|_| |_|/ |\\___|\\___|\\__|\n");
	printf("\x1b[31m       |___/                                        |__/               \n\x1b[39m");
	printf("\n");
	printf("[*] this is x64 version\n");
	printf("[*] make sure DLL files and target process is x64\n");
	printf("\n\n");
}

void print_fail(char *str) {
	printf("\x1b[31m[X] \x1b[39m%s failed\n\n", str);
	exit(1);
}

void print_ok(char *str) {
	printf("\x1b[32m[O] \x1b[39m%s\n", str);
}

void print_help() {
	printf("\nCommands:\n");
	printf("--------------------------------------------------------------\n");
	printf("set: set PID and DLL\n");
	printf("show: show current settings\n");
	printf("inject: execute DLL injection\n");
	printf("getpid: get PID of the given process\n");
	printf("exit: exit program\n");
	printf("--------------------------------------------------------------\n\n");
}

void get_pid() {
	char query[20];
	char cmd[50] = "tasklist | findstr ";

	printf("enter process name: ");
	scanf("%s", query);
	strcat(cmd, query);
	printf("\n");
	system("tasklist | findstr PID & tasklist | findstr ==");
	system(cmd);
	scanf("%c");
	printf("\n");
}

void set() {
	char buf[30];
	printf("target PID: ");
	scanf("%d", &g_pid);
	printf("DLL file name: ");
	getcwd(g_path, sizeof g_path);
	strcat(g_path, "\\DLLs\\");
	scanf("%s", buf);
	strcat(g_path, buf);
	scanf("%c");
	printf("\n");
}

void show() {
	printf("\nCurrent options:\n");
	printf("--------------------------------------------------------------\n");
	printf("  target PID -> %d\n", g_pid);
	printf("  DLL file path -> %s\n", g_path);
	printf("--------------------------------------------------------------\n\n");
}

int check() {
	if(g_pid<1) {
		printf("set target PID!\n\n");
		return 0;
	}
	else if(strcmp(g_path, "none")==0) {
		printf("choose DLL!\n\n");
		return 0;
	}
	else if(!existFile(g_path)) {
		printf("DLL not found in DLLs/ !\n\n");
		return 0;
	}

	char check;
	show();
	printf("Inject? (y/n) : ");
	scanf("%c", &check);
	if(check=='y') return 1;
	return 0;
}

void quit() {
	printf("\nexiting...\n\n");
	exit(0);
}

void inject() {
    DWORD pid;
    HANDLE proc;
    LPSTR libPath;
    LPSTR remoteLibPath;
    DWORD pathSize;
    libPath = g_path;

	 if(!check()) return;

    pid = (unsigned long)g_pid;

    proc = OpenProcess(
        PROCESS_CREATE_THREAD      //CreateRemoteThread
            | PROCESS_VM_OPERATION //VirtualAllocEX
            | PROCESS_VM_WRITE,    //WriteProcessMemory
        FALSE,                     //InheritHandle
        pid);
	 if(proc==NULL) print_fail("OpenProcess");
	 else print_ok("opened process");
	 printf("[*] process %ld's open handle is %ld\n", pid, proc);

    pathSize = strlen(libPath) + 1;

    remoteLibPath = VirtualAllocEx(proc, NULL, pathSize, MEM_COMMIT, PAGE_READWRITE);
	 if(remoteLibPath==NULL) print_fail("VirtualAllocEx");
	 else print_ok("allocated virtual memory");
    
    int memwrite_check = WriteProcessMemory(proc, remoteLibPath, libPath, pathSize, NULL);
	 if(memwrite_check==0) print_fail("WriteProcessMemory");
	 else print_ok("wrote process to memory");

	 HANDLE hproc_check = CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, remoteLibPath, 0, NULL);
	 if(hproc_check==NULL) print_fail("CreateRemoteThread");
	 else print_ok("created remote thread");

	 print_ok("injected!!");
	 printf("[*] make sure to 'kill' the process you injected\n\n");
	 scanf("%c");
}


void cmd_handler(char *cmd) {
	if(strcmp(cmd, "help\n")==0)        print_help();
	else if(strcmp(cmd, "show\n")==0)   show();
	else if(strcmp(cmd, "set\n")==0)    set();
	else if(strcmp(cmd, "inject\n")==0) inject();
	else if(strcmp(cmd, "exit\n")==0)   quit();
	else if(strcmp(cmd, "getpid\n")==0) get_pid();
	else if(strcmp(cmd, "\n")==0)       return;
	else printf("command not found\n");
}


int main() {
	 print_logo();

	 while(1) {
		 char cmd[20];
		 printf("DInject> ");
		 fgets(cmd, sizeof cmd, stdin);
		 cmd_handler(cmd);
	 }
}
