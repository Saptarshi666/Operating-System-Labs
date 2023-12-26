#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
struct job{
    // job id
    int jobid;
    int status; //0-running, 1-done,2-stopped
    char * job_string;
    int pgid;
    struct job * next_job;
    struct job * prev_job;
    //job number
};
int pid_ch1,existpipe;
static int yash;
struct job * head = NULL;
struct job * tail = NULL;
struct job * foreground = NULL;
void add_job(char * args, int pid, int stopped)
{
    struct job * J = malloc(sizeof(struct job));
    J->job_string = (char*) malloc(sizeof(char) * strlen(args)) ;
    for(int i = 0; i<strlen(args); i++)
    {
        J->job_string[i] = args[i];
    }
    J->job_string[strlen(args)] = NULL;
    if(stopped == 2)
    {
        J->status = stopped;
    }
    else

    {
        J->status = 0;
        }
    J->pgid = pid;
    //setpgid(pid,0);
    if(head == NULL){
        J->jobid = 1;
        J->next_job= NULL;
        J->prev_job = NULL;
        head = J;
        tail = J;
    }
    else
    {
        J->jobid = tail->jobid + 1;
        tail->next_job = J;
        J->prev_job = tail;
        J->next_job = NULL;
        tail = J;
    }
}
void remove_job()
{
    struct job *cur = head;
    while(cur != NULL)
    {
        if(cur->status == 1)
        {   
                printf("[%d]%s %s          %s\n",cur->jobid,"+","Done",cur->job_string);   
            if((cur == head) && (cur == tail))
            {
                free(cur->job_string);
                free(cur);
                head = NULL;
                tail = NULL;
                break;
            }
            else if(cur == head)
            {
                head = cur->next_job;
                cur->next_job->prev_job = NULL;
                free(cur->job_string);
                free(cur);
            }
            else if(cur == tail)
            {
                tail = cur->prev_job;
                cur->prev_job->next_job = NULL;
                free(cur->job_string);
                free(cur);
            }
            else
            {
                cur->prev_job->next_job = cur->next_job;
                cur->next_job->prev_job = cur->prev_job;
                free(cur->job_string);
                free(cur);
            }
        }

        
            cur = cur->next_job;
        
    }
}
void check_jobs()
{
    struct job * cur = head;
    while(cur != NULL)
    {
        if(waitpid(cur->pgid,NULL ,WNOHANG| WUNTRACED)!= 0 && cur->status == 0)
        {
            cur->status = 1;
        }
        cur = cur->next_job;
    }
}
void backgr()
{
   struct job * cur = tail;
   int flag = 0; 
   while(cur != NULL)
   {
    if(cur->status== 2)
    {
        kill(-1 * cur->pgid,SIGCONT);
        cur->status = 0;
        printf("%s\n",cur->job_string);
        break;
    }
    cur = cur->prev_job;
   }
}
void forgr()
{
    int status;
     struct job * cur = tail;
     if(tail == NULL)
      return;
    tcsetpgrp(STDOUT_FILENO,cur->pgid); // give terminal control to job
     kill(-1 * cur->pgid,SIGCONT);
     printf("%s\n",cur->job_string);
     waitpid(cur->pgid,&status,WUNTRACED);
     pid_ch1 = cur->pgid;
     tcsetpgrp(STDOUT_FILENO,yash);
     // give terminal control back to yash
     if(cur-> prev_job != NULL)
      {tail = cur->prev_job;
      free(cur->job_string);
      free(cur);}
      else
      {
        free(cur->job_string);
        free(cur);
        tail = NULL;
        head = NULL;
      }
     }
void print(){
    struct job * cur = head;
    while(cur != NULL )
    {
        if(cur->status == 1)
        {
            if(cur->next_job == NULL)
            {
                printf("[%d]%s %s          %s\n",cur->jobid,"+","Done",cur->job_string);
            }else{
                printf("[%d]%s %s          %s\n",cur->jobid,"-","Done",cur->job_string);
            }
        }
        else if(cur->status == 0){
             if(cur->next_job == NULL)
            {
                printf("[%d]%s %s          %s\n",cur->jobid,"+","Running",cur->job_string);
            }else{
                printf("[%d]%s %s          %s\n",cur->jobid,"-","Running",cur->job_string);
            }
        }
        else
        {
            if(cur->next_job == NULL)
            {
                printf("[%d]%s %s          %s\n",cur->jobid,"+","Stopped",cur->job_string);
            }else{
                printf("[%d]%s %s          %s\n",cur->jobid,"-","Stopped",cur->job_string);
            }
        }
        cur = cur->next_job;
    }
}
void ecmd(char* str_array[],int size, int bg,char * cmd);
void fileredirec(char* str_array[],int size, int bg,char * cmd);
void Pi1(char* str_array[],int size, int p,int bg,char * cmd);
void fileredirec1(char* str_array[],int size);
void Pi1(char* str_array[],int size, int p, int bg,char * cmd)
{
    char* sub_str_array1[size];
    char* sub_str_array2[size];
    char* sub_str_array3[size];
    char* sub_str_array4[size];
    for(int i = 0 ; i<p; i++)
    {
       sub_str_array1 [i] = str_array[i];
    }
    sub_str_array1[p] = NULL;
    int j = 0;
    for(int i = p+1; i<size; i++)
    {
        sub_str_array2[j] = str_array[i];
        j++;
    }
    if(bg == 0)
    sub_str_array2[j] = NULL;
    else
      sub_str_array2[j-1] = NULL;
     int pfd[2];
    int cpid1, cpid2, status,val1,val2;
    pipe(pfd);
    cpid1 = fork();
    pid_ch1 = cpid1;
    if(cpid1>0)
    {
        cpid2 = fork();
        if(cpid2>0)
        {   signal(SIGTTIN, SIG_IGN);
            signal(SIGTTOU, SIG_IGN);
            /* signal(SIGINT, SIG_DFL);
signal(SIGTSTP, SIG_DFL);*/
            close(pfd[0]); //close the pipe in the parent
            close(pfd[1]);
            waitpid(cpid1, &status,WNOHANG|WUNTRACED);
            waitpid(cpid2,&status,WNOHANG|WUNTRACED);
       // exit(1);
    }
    else if (cpid2 == 0)
    {   
        signal(SIGTTIN, SIG_IGN);
signal(SIGTTOU, SIG_IGN);
signal(SIGINT, SIG_DFL);
signal(SIGTSTP, SIG_DFL);
        sleep(1);
        setpgid(0,cpid1);
        dup2(pfd[0],0);
        close(pfd[1]);
        fileredirec1(sub_str_array2, j);
        int i = 0;
         for(; i<j ; i++)
        {
            if((strcmp(sub_str_array2[i],">")==0) || (strcmp(sub_str_array2[i],"<")==0)|| (strcmp(sub_str_array2[i],"2>")==0) )
                         {
                            
                            break;
                         }
                         else
                         {
                            sub_str_array3[i] = sub_str_array2[i];
                         }
        }
        sub_str_array3[i] = NULL;
        signal(SIGINT, SIG_DFL);
signal(SIGTSTP, SIG_DFL);
            execvp(sub_str_array3[0], sub_str_array3);
        exit(-1);
    }
    }
     else if(cpid1 == 0){
            signal(SIGTTIN, SIG_IGN);
signal(SIGTTOU, SIG_IGN);
signal(SIGINT, SIG_DFL);
signal(SIGTSTP, SIG_DFL);
           setpgid(0,0);       
        dup2(pfd[1],1);
        close(pfd[0]);
        fileredirec1(sub_str_array1, p);
        int i = 0;
        for(; i<p ; i++)
        {
            if((strcmp(sub_str_array1[i],">")==0) || (strcmp(sub_str_array1[i],"<")==0)|| (strcmp(sub_str_array1[i],"2>")==0) )
                         {
                            
                            break;
                         }
                         else
                         {
                            sub_str_array4[i] = sub_str_array1[i];
                         }
        }
        sub_str_array4[i] = NULL;
        signal(SIGINT, SIG_DFL);
signal(SIGTSTP, SIG_DFL);
        execvp(sub_str_array4[0], sub_str_array4);
        exit(-1);
    }
    if (cpid1 < 0 || cpid2 < 0){
        perror("fork failed");
        printf("%d",cpid1);
        printf("%d",cpid2);
        _exit(1);
    }
    if (bg == 1)
    {
        add_job(cmd,cpid1, 0);
    }
    if(!bg)
    {
            addJobf(cmd,cpid1);
            waitpid(cpid1, &status,WUNTRACED);
            waitpid(cpid2,&status,WUNTRACED);
            tcsetpgrp(STDOUT_FILENO,getpgid(0));
            
    }
}
void fileredirec1(char* str_array[],int size)
{   
            for(int i = 0; i<size; i++){
        if((strcmp(str_array[i],">")==0))
        {
            
           
             
               int fd = open(str_array[i+1],O_WRONLY|O_CREAT| O_TRUNC, S_IRWXU);
               if(fd<0)
               return;
                dup2(fd,1);
                close(fd);
             
        }
        else if((strcmp(str_array[i],"<")==0))
        {
            
         
             
               int fd = open(str_array[i+1],O_RDONLY, S_IRWXU);
                if (fd < 0) {
                return;}
                dup2(fd,0);
                 close(fd);
             
        }
        else if((strcmp(str_array[i],"2>")==0))
        {
            
            
               int fd = open(str_array[i+1], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                if (fd < 0){
                return;}
                dup2(fd,2);
                 close(fd);
             
        }
            }
}
char *str_array[3008];

void parse1(char *cmd)
{
    int size = 0;
    char* token;
    char* rest = cmd;
    char* rest1 = (char*) malloc((strlen(cmd)+1)*sizeof(char));
    for(int i = 0; i<strlen(cmd); i++)
    {
        rest1[i] = cmd[i];
    }
    rest1[strlen(cmd)]= NULL;
    int i=0;
    while((token= strtok_r(rest," ",&rest)))
    {
        str_array[i]=(char*)malloc(strlen(token)*sizeof(char));
        strcpy(str_array[i],token);
        i++;
        size++;
    }
    str_array[size] = NULL;
 if((strcmp(str_array[0],"<")==0)|| (strcmp(str_array[0],">")==0) || (strcmp(str_array[0],"2>")==0)||(strcmp(str_array[0],"&")==0)||(strcmp(str_array[0],"|")==0))
 {
     for(int i = 0; i<size+1; i++)
 {
    free(str_array[i]);
 }
 free(rest1);
    return;
 }

 // check for bg,jobs,fg first
    if(strcmp(str_array[0],"fg")==0)
    {
        forgr();
    }
    if(strcmp(str_array[0],"bg")==0)
    {
        backgr();
    }
    if(strcmp(str_array[0],"jobs")==0)
    {
        print();
    }
 //check for pipes and & issues
 int flag = 0;
 for(int i = 0; i<size; i++)
 {
    if(strcmp(str_array[i],"|")==0)
    {
        existpipe = 1;
        flag =1; 
            if(strcmp(str_array[size-1],"&")==0)
            {
                Pi1(str_array,size,i,1,rest1);
                break;
            }
            else
            {
                Pi1(str_array,size,i,0,rest1);
                break;
            }
    }
    
 }
 for(int i = 0 ; i<size; i++)
 {
     if((strcmp(str_array[i],">")==0) || (strcmp(str_array[i],"<")==0)|| (strcmp(str_array[i],"2>")==0) )
        { 
            flag = 1;
             if(strcmp(str_array[size-1],"&")==0)
            {
                fileredirec(str_array,size,1,rest1);
                break;
            }
            else
            {
                fileredirec(str_array,size,0,rest1);
                break;
            }
        }
 }
if(flag == 0)
{
     if(strcmp(str_array[size-1],"&")==0)
            {
                ecmd(str_array,size,1,rest1);
            }
            else
            {
                ecmd(str_array,size,0,rest1);
            }
}
 if(size != 0)
 {
 for(int i = 0; i<size; i++)
 {
    free(str_array[i]);
 }
 free(rest1);
 }
}
void ecmd(char* str_array[],int size, int bg,char * cmd)
{
    volatile int status,val;
     int pid = fork();
     pid_ch1 = pid;
    if (pid < 0){
        perror("fork failed");
        _exit(1);
    }else if(pid == 0){
        //setsid();
        setpgid(0,0);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGINT, SIG_DFL);
signal(SIGTSTP, SIG_DFL);
        if(strcmp(str_array[size-1],"&") == 0)
            str_array[size-1] = NULL;
        execvp(str_array[0], str_array);
        exit(-1);
    }
    else{
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        /* signal(SIGINT, SIG_DFL);
signal(SIGTSTP, SIG_DFL);*/
        if(bg == 1)
        add_job(cmd, pid,0);
        if(!bg){
            addJobf(cmd,pid);
           val =  waitpid(pid,&status,WUNTRACED);
            tcsetpgrp(STDOUT_FILENO,getpgid(0));
            
        }
    }
}
void fileredirec(char* str_array[],int size, int bg,char * cmd)
{
    int cpid = fork(), status;
        if(cpid == 0)
        {
           setpgid(0,0);
            signal(SIGTTIN, SIG_IGN);
signal(SIGTTOU, SIG_IGN);
signal(SIGINT, SIG_DFL);
signal(SIGTSTP, SIG_DFL); 
            for(int i = 0; i<size; i++){
        if((strcmp(str_array[i],">")==0))
        {
            
            if(cpid == 0)
             {
               int fd = open(str_array[i+1],O_WRONLY|O_CREAT| O_TRUNC, S_IRWXU);
               if(fd<0)
               return;
                dup2(fd,1);
                close(fd);
                 char* sub_str_array1[size];
                 
                 for(int p = 0;p<i; p++)
                    {
                         if((strcmp(str_array[p],">")==0) || (strcmp(str_array[p],"<")==0)|| (strcmp(str_array[p],"2>")==0) )
                         {
                            break;
                         }
                         else
                        sub_str_array1[p] = str_array[p];
                    }
                    sub_str_array1[i] = NULL;
                    signal(SIGINT, SIG_DFL);
signal(SIGTSTP, SIG_DFL);
                execvp(str_array[0],sub_str_array1);
                exit(-1);
             }
        }
        else if((strcmp(str_array[i],"<")==0))
        {
            
            if(cpid == 0)
             {
               int fd = open(str_array[i+1],O_RDONLY, S_IRWXU);
                if (fd < 0) {
                return;}
                dup2(fd,0);
                 close(fd);
                 char* sub_str_array1[size];
                 for(int p = 0 ; p<i; p++)
                    {
                         if((strcmp(str_array[p],">")==0) || (strcmp(str_array[p],"<")==0)|| (strcmp(str_array[p],"2>")==0) )
                         {
                            break;
                         }
                         else
                         sub_str_array1[p] = str_array[p];
                    }
                    sub_str_array1[i] = NULL;
                    signal(SIGINT, SIG_DFL);
signal(SIGTSTP, SIG_DFL);
                execvp(str_array[0],sub_str_array1);
                exit(-1);
             }
        }
        else if((strcmp(str_array[i],"2>")==0))
        {
            
            if(cpid == 0)
             {
               int fd = open(str_array[i+1], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                if (fd < 0){
                return;}
                dup2(fd,2);
                 close(fd);
                 char* sub_str_array1[size];
                 for(int p = 0 ; p<i; p++)
                    {
                         if((strcmp(str_array[p],">")==0) || (strcmp(str_array[p],"<")==0)|| (strcmp(str_array[p],"2>")==0) )
                         {
                            break;
                         }
                         else
                        sub_str_array1[p] = str_array[p];
                    }
                    sub_str_array1[i] = NULL;
                    signal(SIGINT, SIG_DFL);
signal(SIGTSTP, SIG_DFL);
                execvp(str_array[0],sub_str_array1);
                exit(-1);
             }
        }
        }
        }
        else
        {
            waitpid(cpid, &status,WNOHANG|WUNTRACED);
            signal(SIGTTIN, SIG_IGN);
signal(SIGTTOU, SIG_IGN);
 /*signal(SIGINT, SIG_DFL);
signal(SIGTSTP, SIG_DFL);*/
             if(bg == 1 && existpipe == 0)
        add_job(cmd, cpid,0);
        if(!bg){
            addJobf(cmd,cpid);
            wait(NULL);
            
        }
        }
        //pid_ch1 = cpid;
    
}
void addJobf(char * args, int pid)
{
    
     foreground = malloc(sizeof(struct job));
    foreground->job_string = (char*) malloc(sizeof(char) * strlen(args)) ;
    for(int i = 0; i<strlen(args); i++)
    {
        foreground->job_string[i] = args[i];
    }
    foreground->job_string[strlen(args)] = NULL;
    foreground->status = 0;
    foreground->pgid = pid;
    
}
void delJobf()
{
    free(foreground->job_string);
    free(foreground);
    foreground = NULL;
}
void handler1(int signum)
{
    if(foreground)
    {
        tcsetpgrp(STDOUT_FILENO,yash);
        kill(-foreground->pgid,SIGINT);
        delJobf();
    }
   else{
    return;
   }
}
void handler2(int signum)
{
    
    if(foreground)
    {
         tcsetpgrp(STDOUT_FILENO,yash);
         kill(-foreground->pgid,SIGTSTP);
         add_job(foreground->job_string,foreground->pgid,2);
         delJobf();
         
    }
    else
    {
        return;
    }
   // kill(-pid_ch1,SIGTSTP);
   // tcsetpgrp(STDOUT_FILENO,getpgid(0));
}

int main(int argc, char **argv){
    signal(SIGINT, &handler1);
    signal(SIGTSTP,&handler2);
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
   /*  signal(SIGINT, SIG_DFL);
signal(SIGTSTP, SIG_DFL);*/
    char *cmd;
int shell = getpid();
setpgid(0,0);
tcsetpgrp(0,shell);
yash = shell;
foreground = NULL;
    while (1){
      tcsetpgrp(STDOUT_FILENO,getpgid(0));  
    existpipe = 0;
	cmd = readline("# ");
    if(strcmp(cmd,"")!=0)
	    parse1(cmd);
    check_jobs();
    remove_job();
    if(foreground != NULL)
        delJobf();
    }
    printf("done\n");
    exit(0);
}