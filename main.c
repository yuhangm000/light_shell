#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>

//多个空格合一
void shark_space(char *s){
    char *p=s;
    while(*s!='\0'){
        while(*s==' ' &&*(s+1)==' '){
            s++;
        }
        *p=*s;
        s++;
        p++;
    }
    *p='\0';
}

//将字符串分解
void del_str(char** command,char* str,char* sep){
    shark_space(str);
    char* token = strtok(str,sep);
    int count=0;
    while(token != NULL) {
        command[count]=token;
        count++;
        token = strtok(NULL,sep);
    }
}

//计算所有参数个数
int Arg_num(char** cmd){
    int count=0;
    for(int i=0;i<10;i++){
        if(cmd[i]!=NULL)
            count++;
        else
            break;
    }
    return count;
}

//print error
void Error(char* str){
    printf("%s\n",str);
}

//cd命令
int cd_command(char* add_path){
    char cur_dir[256]={};
    char final_dir[256]={};
    getcwd(cur_dir,sizeof(cur_dir));

    if(strcmp(add_path,"..")==0){
        char*lastFile=strrchr(cur_dir,'/');
        int length=strlen(cur_dir)-strlen(lastFile);
        strncpy(final_dir,cur_dir, length);
    }
    else if(strcmp(add_path,".")==0){
        strncpy(final_dir,cur_dir, strlen(cur_dir));
    }
    else{
        if(add_path[0]!='/')
            strcat(cur_dir,"/\0");
        strcat(cur_dir,add_path);
        strncpy(final_dir,cur_dir, strlen(cur_dir));
    }

    if(chdir(final_dir)==-1) {
        printf("path don't exist.\n");
        return -1;
    }
    printf("current path: %s\n",final_dir);

    return 0;
}

void pwd_command(){
    cd_command(".");
}

void new_fork(char* cmd,char* arg){

    pid_t pid;
    pid=fork();
    int status=0;
    if (pid < 0) {
        printf("error in fork!");
        return;
    }
    else if (pid == 0) {
        execv(cmd,arg);
        //printf("i am the child process, my process id is %d\n",getpid());
        exit(0);
    }
    else {
        waitpid(pid,&status,0);
        //printf("i am the parent process, my process id is %d\n",getpid());
    }
}


void execute(char**cmd){
    char path[]="/bin/";
    strcat(path,cmd[0]);
    execv(path,cmd);
}


//命令选择
void switch_command(char** command){
    if(strcmp(command[0],"cd")==0) {
        if ((Arg_num(command) - 1) == 1)
            cd_command(command[1]);
        else
            Error("参数过多");
    }
    else if(strcmp(command[0],"pwd")==0){
        if ((Arg_num(command) - 1) == 0)
            pwd_command();
        else
            Error("参数过多");
    }
    else if(strcmp(command[0],"ls")==0){
        if((Arg_num(command) - 1) <= 1)
            new_fork("/bin/ls",command);
        else
            Error("参数过多");
    }
    else if(strcmp(command[0],"cat")==0){
        if((Arg_num(command) - 1) == 1)
            new_fork("/bin/cat",command);
        else
            Error("参数个数错误");
    }
    else if(strcmp(command[0],"grep")==0){
            new_fork("/bin/grep",command);
    }
    else{
        Error("找不到命令");
    }
}

/**
 * deal with commands with grep
 * @param command command groups
 * @param n number of command group
 */
void deal_pipe(char** command,int n){
    //int savefd = dup(STDOUT_FILENO);
    if(n<Arg_num(command)) {
        char* cur_command[10]={NULL};
        del_str(cur_command,command[n]," ");
        char* future_command[10]={NULL};
        del_str(future_command,command[n+1]," ");

        int fd[2];
        int status=0;
        int ret=pipe(fd);//创建管道
        if (ret==-1){
            fprintf(stderr, "%s\n", "pipe error!");
            exit(-1);
        }
        //创建进程
        int pid=fork();
        if(pid<0){
            fprintf(stderr, "%s\n", "fork error!");
            exit(-1);
        }
        else if(pid==0){//在子进程中
            close(fd[1]);
            dup2(fd[0],STDIN_FILENO);//将子进程的标准输入重定向到fd[0]
            execute(future_command);
            exit(0);
        }
        else{
            //father process
            close(fd[0]);
            dup2(fd[1],STDOUT_FILENO);// 将父进程的标准输出重定向到fd[1]
            execute(cur_command);
            waitpid(pid,&status,0);
        }
    }
}

int main() {

    while(1) {
        char input[128]={NULL};
        gets(&input);
        if(strlen(input)==0)
            continue;
        if (!strcmp(input, "exit"))
            break;
        if (strcmp(input, "\n")==0) {
            continue;
        }

        char* command[10]={NULL};//最多10个命令
        if(strstr(input,"|")){
            int pid=fork();
            if(pid==0){
                del_str(command,input,"|");
                //每个command[n]都是command[n+1]的父进程(前者是后者的管道输入)
                deal_pipe(command,0);
            }
        }
        else{
            //逻辑简单
            del_str(command, input," ");
            switch_command(command);
        }

    }

    return 0;
}