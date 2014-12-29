#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include "RSA.c"
 
int main(int argc,char **argv)
	{
	int listenfd = 0,connfd = 0;
	struct sockaddr_in serv_addr;
	char sendBuff[1025];  
	char readBuff[1024];
	int n;
	unsigned int SN;
	FILE *vote;
	char filename[1024];
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(atoi(argv[1]));
	
	bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
	
	if(listen(listenfd, 1024) == -1 )
		{
		printf("Failed to listen\n");
		return -1;
		}
	printf("Counter is up and running!\n");
	connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); 
	while(1)
		{
		n=read(connfd,readBuff,44);
		if(n==0)
			{
			printf("Can't read from authenticator\n");
			break;	
			}
		else if(n!=44)
			{
			printf("Currupted Vote!\n");
			sendBuff[0]=1;
			write(connfd, sendBuff,1);
			continue;	
			}
		memcpy(&SN,readBuff+40,4);
		sprintf(filename,"votes/%08X_vote",SN);
		vote=fopen(filename,"wb");
		fwrite(readBuff,44,1,vote);
		fclose(vote);
		if(verify(filename,"FFFFFFFF_pub")==false)
			{
			printf("Voter Forged!\n");
			remove(filename);
			sendBuff[0]=2;
			write(connfd, sendBuff,1);
			continue;
			}
		sendBuff[0]=0;
		vote=fopen("temp","wb");
		fwrite(&SN,4,1,vote);
		fclose(vote);
		certify("temp","pri_key","temp1");
		vote=fopen("temp1","rb");
		fread(sendBuff+1,20,1,vote);
		write(connfd,sendBuff,21);
		remove("temp");
		remove("temp1");
		}
	close(connfd);
	return 0;
	}
