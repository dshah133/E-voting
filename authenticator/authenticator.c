#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include "RSA.c"

void conv(char x[30])
	{
	int i;
	for(i=0;x[i]!=':';i++);
	x[i]=' ';
	return;	
	}
int main(int argc,char **argv)
{
	int listenfd = 0,connfd = 0;
	int sockfd=0;
	int sockfd1=0;
	struct sockaddr_in serv_addr;
	struct sockaddr_in counter_addr;
	char readBuff[2048];
	char sendBuff[2048];
	char filename[1024];
	char filename1[1024];
	char filename2[1024];
	
	char PKdist[30];
	char registrar[30];
	char auth[30];
	char counter[30];
	char ip_t[30];
	int port_t;

	
	FILE *ip=fopen("ip.config","r");
	FILE *vote,*key,*rec;
	fgets(registrar,30,ip);
	fgets(PKdist,30,ip);
	fgets(auth,30,ip);
	fgets(counter,30,ip);
	fclose(ip);
	
	
	unsigned int sr=0;
	unsigned int voterID;

	
	conv(PKdist);
	sscanf(PKdist,"%s %d",ip_t,&port_t);



	counter_addr.sin_family = AF_INET;
	counter_addr.sin_port = htons(port_t);
	counter_addr.sin_addr.s_addr = inet_addr(ip_t);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	connect(sockfd, (struct sockaddr *)&counter_addr, sizeof(serv_addr));

	

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff));
	serv_addr.sin_family = AF_INET;    
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(atoi(argv[1]));
	
	bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));

	if(listen(listenfd, 1024) == -1 )
		{
		printf("Failed to listen\n");
		return -1;
		}

	printf("Authenticator is up and running!\n");
	
	fflush(stdout);

	while(1)
		{
		connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);
		read(connfd,readBuff,52);
		memcpy(&voterID,readBuff+16+24,4);
		printf("Voter %08X vote received!\n",voterID);
		sprintf(filename,"votes/%08X_vote",voterID);
		if(!access(filename,R_OK))
			{
			printf("Already voted\n");
			continue;	
			}
		vote=fopen(filename,"wb");
		fwrite(readBuff,52,1,vote);
		fclose(vote);
///// ask public key of voter from PKdist
		write(sockfd,&voterID,4);
		read(sockfd,readBuff,17);
		if(readBuff[0]==0)
			{
			sprintf(filename1,"%08X_pub",voterID);
			key=fopen(filename1,"wb");
			fwrite(readBuff+1,16,1,key);
			fclose(key);
			
			if(verify(filename,filename1))
				{
				sendBuff[0]=0;
				sprintf(filename2,"%08X_rec",voterID);
				certify(filename,"pri_key",filename2);
				rec=fopen(filename2,"rb");
				fread(sendBuff+1,68,1,rec);
				write(connfd,sendBuff,69);
				fclose(rec);
				remove(filename2);
				printf("vote counted \n");
				fflush(stdout);
				}
			else
				{
				printf("Forged vote!\n");
				continue;	
				}
			remove(filename1);
			}
		else
			{
			printf("error!\n");
			continue;	
			}
		sleep(1);
		}	

	return 0;
}