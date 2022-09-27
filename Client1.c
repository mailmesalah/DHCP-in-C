/************* UDP CLIENT CODE *******************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>

struct OutgoingPacket {
   unsigned int  ip[4];
   int  transID;
};

struct DHCPSuggest {
   unsigned int  ip[4];
   int  timed;
};

struct DHCPACK {
   unsigned int  ip[4];
};

int randRange(int Min, int Max)
{
    int diff = Max-Min;
    return (int) (((double)(diff+1)/RAND_MAX) * rand() + Min);
}

int main(int argc, char *argv[])
{
	//checks if corrent number of arguments
	if (argc != 3) {
		printf("Usage: ./Client <IP_ADDRESS> <PORT_NUMBER>\n");
		return 1;
	}
	
	//Initalise for randomize
	srand(time(NULL));
	
	int portNo =atoi(argv[2]);
	
	int clientSocket, portNum, nBytes;
	struct sockaddr_in serverAddr;
	socklen_t addr_size;

	/*Create UDP socket*/
	clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

	/*Configure settings in address struct*/
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portNo);
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

	/*Initialize size variable to be used later on*/
	addr_size = sizeof serverAddr;

	/*Send message to server*/
	//Send 0.0.0.0
	int transsactionID =randRange(1,1000);
	struct OutgoingPacket *packet=malloc(sizeof(struct OutgoingPacket));
	packet->ip[0]=0;
	packet->ip[1]=0;
	packet->ip[2]=0;
	packet->ip[3]=0;
	packet->transID=transsactionID;//Transaction id
	sendto(clientSocket,packet,sizeof(struct OutgoingPacket),0,(struct sockaddr *)&serverAddr,addr_size);
	printf("Requesting New IP 0.0.0.0\n");
	
	struct DHCPSuggest *s1=malloc(sizeof(struct DHCPSuggest));
	nBytes = recvfrom(clientSocket,s1,sizeof(struct DHCPSuggest),0,(struct sockaddr *)&serverAddr, &addr_size);
	printf("\nIP Received %d.%d.%d.%d with time out %d\n",s1->ip[0],s1->ip[1],s1->ip[2],s1->ip[3],s1->timed);
	
	struct DHCPSuggest *s2=malloc(sizeof(struct DHCPSuggest));
	nBytes = recvfrom(clientSocket,s2,sizeof(struct DHCPSuggest),0,(struct sockaddr *)&serverAddr, &addr_size);
	printf("IP Received %d.%d.%d.%d with time out %d\n",s2->ip[0],s2->ip[1],s2->ip[2],s2->ip[3],s1->timed);
	
	struct DHCPSuggest *s3=malloc(sizeof(struct DHCPSuggest));
	nBytes = recvfrom(clientSocket,s3,sizeof(struct DHCPSuggest),0,(struct sockaddr *)&serverAddr, &addr_size);
	printf("IP Received %d.%d.%d.%d with time out %d\n",s3->ip[0],s3->ip[1],s3->ip[2],s3->ip[3],s1->timed);
	
	//Confirms by selecting random
	int toss = randRange(0,2);
	if(toss==0){
		packet=malloc(sizeof(struct OutgoingPacket));
		packet->ip[0]=s1->ip[0];
		packet->ip[1]=s1->ip[1];
		packet->ip[2]=s1->ip[2];
		packet->ip[3]=s1->ip[3];
		packet->transID=++transsactionID;//Transaction id
		sendto(clientSocket,packet,sizeof(struct OutgoingPacket),0,(struct sockaddr *)&serverAddr,addr_size);
	}else if(toss==1){
		packet=malloc(sizeof(struct OutgoingPacket));
		packet->ip[0]=s2->ip[0];
		packet->ip[1]=s2->ip[1];
		packet->ip[2]=s2->ip[2];
		packet->ip[3]=s2->ip[3];
		packet->transID=++transsactionID;//Transaction id
		sendto(clientSocket,packet,sizeof(struct OutgoingPacket),0,(struct sockaddr *)&serverAddr,addr_size);
	}else{
		packet=malloc(sizeof(struct OutgoingPacket));
		packet->ip[0]=s3->ip[0];
		packet->ip[1]=s3->ip[1];
		packet->ip[2]=s3->ip[2];
		packet->ip[3]=s3->ip[3];
		packet->transID=++transsactionID;//Transaction id
		sendto(clientSocket,packet,sizeof(struct OutgoingPacket),0,(struct sockaddr *)&serverAddr,addr_size);
	}
	
	//ACK
	struct DHCPACK *ack=malloc(sizeof(struct DHCPACK));
	nBytes = recvfrom(clientSocket,ack,sizeof(struct DHCPSuggest),0,(struct sockaddr *)&serverAddr, &addr_size);
	printf("\nACK IP Received %d.%d.%d.%d\n",ack->ip[0],ack->ip[1],ack->ip[2],ack->ip[3]);

	return 0;
}