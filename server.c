#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>

#define buf_size 15000
#define setSize 10
#define max_name_len 10
#define max_fileName_len 15
#define max_fileName_num 10
#define max_process 10
#define sec 0
#define usec 100000
#define loading 100000
struct frame
{
    char command[10];
    char fileName[max_fileName_len];
    char info[buf_size];
    int len;
    int already_send;
    off_t f_size;
};
struct process
{
    int client_id;
    int using;
    char path[max_fileName_len];
    char fileName[max_fileName_len];
    int fd;
    off_t f_size;
    int aleady_send;
};
struct user_file
{
    char name[max_name_len];
    int fileNum;
    char file[max_fileName_num][max_fileName_len];
};
struct client_info
{
    int user_id;
    int fd;
    int fileNum;
    int sleep;
    char file[max_fileName_num][max_fileName_len];
};
static void erroHandle(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./server [bind port]\n");
        exit(EXIT_FAILURE);
    }
    struct frame packet;
    struct sockaddr_in servaddr,cliaddr;
    struct timeval timeSet;
    struct client_info client[setSize];
    struct user_file user[setSize];
    struct process work[max_process];
    struct stat status;

    char buf[buf_size];
    char *fileName;
    char line[buf_size],reply[buf_size];
    ssize_t addrLen;
    socklen_t clilen;
    int listenfd,maxi,maxfd,connfd,sockfd,user_num=0;

    FILE *fptr;
    fd_set rset,wset,allset;
    int nready;
    /* clear sockaddr_in */
    memset(&servaddr,0,sizeof(servaddr));

    /* set receiver info */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    /* socket */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        erroHandle("socket() error");

    /* bind */
    if (bind(listenfd, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        erroHandle("bind() error");

    listen(listenfd,20);

    maxi=-1;
    maxfd=listenfd;

    for(int i=0;i<setSize;i++)
        {
            client[i].fd=-1;
            client[i].fileNum=0;
            user[i].fileNum=0;
            client[i].sleep=0;
        }
    for(int i=0;i<max_process;i++)
    {
        work[i].using=0;
        work[i].fd=-1;
        work[i].f_size=0;
        work[i].aleady_send=0; 
        work[i].client_id=0;   
    }

    FD_ZERO(&allset);           
    FD_SET(listenfd,&allset);  
    int work_num=0;
    while(1)
    {
        rset=allset;
        //printf("before select\n");
        timeSet.tv_sec = sec;
	if(work_num>1)
        timeSet.tv_usec = usec+(work_num-1)*loading;
	else
	timeSet.tv_usec = usec;

        nready=select(maxfd+1,&rset,NULL,NULL,&timeSet);
        if(nready==0)
            {
                for(int i=0;i<setSize;i++)
                {
                    
                    if(client[i].fd<0)
                    {
                        continue;
                    }
                    else
                    {
                       // printf("client %d file %d\n",i,client[i].fileNum );
                        //printf("user %s file %d\n",user[client[i].user_id].name,user[client[i].user_id].fileNum );
                        int id=client[i].user_id;
                        int totalNum = user[id].fileNum;
                        int nowNum = client[i].fileNum;
                        if(nowNum<totalNum)
                        {
                            int find;
                            int fid;
                            for(int p=0;p<user[id].fileNum;p++)
                            {
                                int q;
                                find = 0;
                                for(int q=0;q<client[i].fileNum;q++)
                                {
                                    if(!strcmp(user[id].file[p],client[i].file[q]))
                                    {
                                        find = 1;
                                        break;
                                    }
                                }
                                if(find == 0)
                                    {
                                        fid = p;
                                        break;
                                    }
                            }
                            int j;
                            for(j=0;j<max_process;j++)
                            {
                                if(!work[j].using)
                                    break;
                            }
                            if(j==max_process)
                                {
                                    printf("FUCK **process FULL**\n");
                                    break;
                                }
                            memset(work[j].path,'\0',sizeof(work[j].path));
                            sprintf(work[j].path,"%s//%s",user[id].name,user[id].file[fid]);
                            stat(work[j].path, &status);
                            work[j].f_size = status.st_size;    //Size of the file
                            work[j].using=1;
                            work[j].fd=client[i].fd;
                            work[j].aleady_send=0;
                            work[j].client_id=i;
                            memset(work[j].fileName,'\0',sizeof(work[j].fileName));
                            sprintf(work[j].fileName,"%s",user[id].file[fid]);
                            sprintf(client[i].file[nowNum],"%s",user[id].file[fid]);
                            client[i].fileNum++;
				//work_num++;
 
                            printf("work %d fileName: %s path: %s size: %lld\n",j,work[j].fileName,work[j].path,work[j].f_size );
                        }
                    }
                }
		work_num=0;
                for(int i=0;i<max_process;i++)
                {
                    if(!work[i].using || client[work[i].client_id].sleep)
                    {
                        continue;
                    }
                    else
                    {
                       // printf("work %d send fileName: %s [path: %s] to %d\n",i,work[i].fileName,work[i].path,work[i].client_id );
                        memset(&packet,0,sizeof(packet));
                        FILE *ptr;
                        work_num++;
                        if(access(work[i].path,F_OK)!=-1)
                             ptr = fopen(work[i].path,"rb");
                        else
                            printf("file %s GG\n",work[i].path );
                        long st = work[i].aleady_send;
                        if(fseek( ptr, st, SEEK_SET)!=0)
                        {
                            printf("fseek GG\n");
                        }
                        int un_send = work[i].f_size-st;
                        int read=un_send;
                        if(un_send>buf_size)
                            read = buf_size;
                        sprintf(packet.command,"get");
                        packet.len = fread(packet.info, 1, read, ptr);
                        sprintf(packet.fileName,"%s",work[i].fileName);
                        work[i].aleady_send+=packet.len;
                        packet.already_send = work[i].aleady_send;
                        packet.f_size = work[i].f_size;
                        //printf("%d %lld\n",packet.already_send,packet.f_size );
                        send(work[i].fd, &(packet), sizeof(packet),0);  
                        if(packet.already_send==work[i].f_size)
                        {
                            work[i].using=0;
                            work[i].fd=-1;
                            work[i].aleady_send=0; 
                            work[i].client_id=0;
			    printf("send %s size: %d done\n",work[i].fileName,work[i].f_size);
			    work[i].f_size = 0;
			    //work_num--;   
                        }
                        fclose(ptr);
                    }
                }
                continue;
            }

        if(FD_ISSET(listenfd,&rset))
        {
            //printf("new client\n");
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
            int i;
            for(i=0;i<setSize;i++)                          //try to find a place for new client
            {
                if(client[i].fd<0)
                    {
                        client[i].fd=connfd;   
                        client[i].fileNum=0;   
                        client[i].sleep=0; 
                        break;
                    }
            }

            if(i==setSize)                                  //we didn't find QQ
            {
                printf("too many clientss"); 
                continue;
            } 

            FD_SET(connfd, &allset);

          

            if (i > maxi)                                   //update maxi && maxfd if need
                maxi = i; 
             if (connfd > maxfd)
                maxfd = connfd;                                       
            if (--nready <= 0)                              //if no more readable data, back to wait
                continue;
        }



        for(int i=0;i<=maxi;i++)                                //handle client's message
        {
            if((sockfd=client[i].fd)<0)                 //skip empty in client array
                continue;

            if(FD_ISSET(sockfd,&rset))                      
            {
                memset(&packet,0, sizeof(struct frame));
                int len=recv(sockfd, &(packet), sizeof(struct frame), 0);
                
                if(len<=0)                                  //someone is leave
                {
                    
                    close(sockfd);                          //handle the leaving
                    FD_CLR(sockfd,&allset);
                    client[i].fd=-1;
                    client[i].fileNum=0;
                    if(sockfd==maxfd)maxfd--;
                    if(i==maxi)maxi--;
                }
                else
                {

                   
                    if(!strcmp(packet.command,"name"))
                    {
                        char *p;
                        p=strtok(packet.info," ");
                        printf("recv name %s\n",packet.info );
                        int j;
                        int new_user=1;
                        for(j=0;j<user_num;j++)
                        {
                            //printf("%d %s %s\n",user_num,user[j].name,p );
                            if(!strcmp(user[j].name,p))
                            {
                                new_user=0;
                                break;
                            }
                        }
                        if(new_user)                                  //new User
                        {
                            sprintf(user[j].name,"%s",p);
                            mkdir(p,S_IRWXU);
                            user_num++;
                            printf("new user %s\n", user[j].name);
                        }

                        client[i].user_id=j;

                        //memset(&(packet),'\0',sizeof(packet));       //say hello to new client
                        //sprintf(packet.command,"hello");
                        //sprintf(packet.info,"Welcome to the dropbox-like server: %s\n",user[j].name);
                        //printf("send hello to %s\n",user[j].name);
                        //write(client[i].fd,&(packet),sizeof(packet));
                        
                    }
                    else if(!strcmp(packet.command,"put"))
                    {
                        
                        int id = client[i].user_id;
                        
                        char pathname[30];
                        memset(pathname,'\0',sizeof(pathname));
                        sprintf(pathname,"%s//%s",user[id].name,packet.fileName);
                        //printf("write to %s\n", pathname);
                        
                        fptr=fopen(pathname,"a+");
                      
                        int wIN = fwrite(packet.info, 1, packet.len, fptr);
                        fclose(fptr);
                        if(packet.f_size == packet.already_send)
                        {
                            sprintf(user[id].file[user[id].fileNum],"%s",packet.fileName);
                            sprintf(client[i].file[client[i].fileNum],"%s",packet.fileName);
                            client[i].fileNum++;
                            user[id].fileNum++;  
                        
                        printf("File: %s total: %lld done\n", packet.fileName, packet.f_size);
                    	}
		    }     
                    else if(!strcmp(packet.command,"sleep"))
                    {
                        client[i].sleep=1;  
                        printf("client %d sleep\n",i );                        //handle the leaving
                    }
                    else if(!strcmp(packet.command,"wakeup"))
                    {
                        client[i].sleep=0;           
                        printf("client %d wakeup\n",i );               //handle the leaving
                    }           
                    else if(!strcmp(packet.command,"exit"))
                    {
                        close(sockfd);                          //handle the leaving
                        FD_CLR(sockfd,&allset);
                        client[i].fd=-1;
                        client[i].fileNum=-1;
                        if(sockfd==maxfd)maxfd--;
                        if(i==maxi)maxi--;
                    }
                }
            }         
        }
    }    
    return 0;
}
