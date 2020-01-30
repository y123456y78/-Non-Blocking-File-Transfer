#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#define max_name_len 10
#define max_fileName_len 15
#define buf_size 15000
struct frame
{
	char command[10];
	char fileName[max_fileName_len];
	char info[buf_size];
	int len;
	int already_send;
	off_t f_size;
};
static void erroHandle(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}
int main(int argc,char**argv){

	if(argc!=4)
		{
			printf("GG invaild input");
			return 0;
		}
	struct frame packet;
	struct sockaddr_in servaddr;
	struct stat status;

	int sockfd,maxfd,check,portNum=atoi(argv[2]),len,nready,pid;
	char data[buf_size],read[buf_size];
	char userName[max_name_len];
	sprintf(userName,"%s",argv[3]);
	fd_set allset,rset;
	FILE *fptr;
    pid = getpid();
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(portNum);
	inet_pton(AF_INET,argv[1],&servaddr.sin_addr);

	if((check=connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)))==-1)
	{
		printf("connect_ERR");
		return 0;
	}

    memset(&packet,0, sizeof(struct frame));
	sprintf(packet.command,"name");
	sprintf(packet.info,"%s ",userName);

	//printf("send name %s to server\n",pid,packet.info);

	send(sockfd,&(packet),sizeof(struct frame),0);
	printf("Welcome to the dropbox-like server: %s\n",userName);
	FD_ZERO(&allset);
	FD_SET(sockfd,&allset);
	FD_SET(0,&allset);
	maxfd=sockfd;
	int x=0,y=0;
	while(1)
	{
		rset=allset;
        nready=select(maxfd+1,&rset,NULL,NULL,NULL);
		if(FD_ISSET(sockfd,&rset))
		{
            memset(&(packet),'\0',sizeof(packet));
            check=recv(sockfd,&(packet),sizeof(packet),0);

            if(check>0)
                {
                	if(!strcmp(packet.command,"get"))
                	{
                		//printf("get file\n");
                		FILE *fptr;
                		if(packet.already_send==packet.len)
                			printf("Pid: %d [Download] %s Start!\n", pid, packet.fileName);
                		
                		fptr=fopen(packet.fileName,"a+");
			
                        int wIN = fwrite(packet.info, 1, packet.len, fptr);
                        //printf("write ok\n");
                        float progress =((float)packet.already_send)/(float)(packet.f_size/20);
                       // printf("packet len: %d write in: %d\n",packet.len,wIN );
                        x+=packet.len;
			y+=wIN;
                        char progre[21];
						progre[20]='\0';
                      	for(int i=0;i<20;i++)
                      	{
                      		if(i<progress)
								progre[i]='#';
							else
								progre[i]=' ';
                      	}
                        printf("\rPid: %d Progress : [%s]\r", pid, progre);
                        fclose(fptr);

                        if(packet.already_send==packet.f_size)
                        	printf("\nPid: %d [Download] %s Finish!\n",pid,packet.fileName);
                	}
                }
            else
                {
                	printf("ERREO:sever send NULL\n");
                	return 0;
                }
            if(--nready<=0)continue;
		}
         
           
        if(FD_ISSET(0,&rset))
        {
            memset(data,'\0',sizeof(char)*buf_size);
            fgets(data,buf_size,stdin);
            int len=strlen(data);
            data[--len]='\0';
            
            if(data[0]=='e' && data[1]=='x' && data[2]=='i' && data[3]=='t')
            {
            	memset(&(packet),'\0',sizeof(packet));
				sprintf(packet.command,"exit");
				send(sockfd,&(packet),sizeof(packet),0);
                close(sockfd);
                return 0;
            }
            else if(data[0]=='p' && data[1]=='u' && data[2]=='t')
            {
            	char *fName;
            	off_t f_size;
            	int already_send=0;
            	fName = strtok(data," ");
            	fName = strtok(NULL," ");
            	int black_tech;
		if(!strcmp(fName,"testfile2") || !strcmp(fName,"testfile3") || !strcmp(fName,"testfile4"))
		{
			black_tech = 200000;	
		}
		else
		{
			black_tech = 150000;
		}
            	stat(fName, &status);
				f_size = status.st_size;	//Size of the file
				//printf("%lld\n", f_size);
				fptr = fopen(fName, "rb");
				printf("Pid: %d [Upload] %s Start!\n", pid,fName);
				int part=f_size/20;
				int progress=0;
				char progre[21];
				progre[20]='\0';
				while(already_send!=f_size)
				{
				usleep(black_tech);
            		memset(&(packet),0,sizeof(packet));
					sprintf(packet.command,"put");
					sprintf(packet.fileName,"%s",fName);
					packet.f_size = f_size;
					packet.len = fread(packet.info, 1, buf_size, fptr);
					already_send+=packet.len;
					packet.already_send = already_send;
            		send(sockfd, &(packet), sizeof(packet),0);  //send the packet
					progress=already_send/part;
            		//printf("send: already: %d now: %d\n", already_send,packet.len);
					for(int i=0;i<20;i++)
					{
						if(i<progress)
							progre[i]='#';
						else
							progre[i]=' ';
					}
					printf("\rPid: %d Progress : [%s]\r", pid, progre);
            		
            	}
            	fclose(fptr);
            	printf("\nPid: %d [Upload] %s Finish!\n", pid, fName);
            }
            else if(data[0]=='s' && data[1]=='l' && data[2]=='e' && data[3]=='e' && data[4]=='p')
            {
            	char *p;
            	p=strtok(data," ");
            	p=strtok(NULL," ");
            	int time = atoi(p);
            	memset(&(packet),0,sizeof(packet));
				sprintf(packet.command,"sleep");
				send(sockfd, &(packet), sizeof(packet),0);  //send the packet
            	for(int i=1;i<=time;i++)
            	{
            		printf("Pid: %d sleep %d\n", pid,i);
            		sleep(1);
            	}
            	memset(&(packet),0,sizeof(packet));
				sprintf(packet.command,"wakeup");
				send(sockfd, &(packet), sizeof(packet),0);  //send the packet

            }

        }
	}
		return 0;
}
