/************* UDP SERVER CODE *******************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>


struct IPnTime {
   unsigned int  ip[4];
   int  timed;
};

struct IncomingPacket {
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

int isIPAvailable(unsigned int ipAddress[], struct IPnTime *temp, int count){
	
	for(int i=0;i<count;++i){
		int found=1;
		for(int k=0;k<4;++k){
			if(ipAddress[k]!=temp->ip[k]){
				found=0;
				break;
			}
		}
		if(found==1){
			return 0;
		}		
		++temp;
	}
	
	return 1;
}

int* nextIPAddress (unsigned int hostS[], unsigned int network[], unsigned int currentIP[]) {
    static int nextIP[4];
	int found=0;
	for(int i=3;i>=0;--i){
		//printf("i=%d %d %d %d %d \n",i,hostS[i],currentIP[i],network[i],found);
		if(hostS[i]!=0 && hostS[i]<=(currentIP[i]-network[i]) && !found){
			nextIP[i]=0;
		}else if(hostS[i]!=0 && !found){
			nextIP[i]=currentIP[i]+1;
			found=1;
		}else{
			nextIP[i]=currentIP[i];			
		}		
	}
	
	return nextIP;
}

int main(int argc, char *argv[]){
	
	//Checks if the no of arguments are of correct
	if (argc != 4) {
		printf("Usage: ./Server <PORT_NUMBER> <GATEWAY_IP> <SUBNET_MASK>\n");
		return 1;
	}
	
	int portNo =atoi(argv[1]);	
	
	//Getting the subnet mask
	char *p = strtok (argv[3], ".");
    char *array[3];	
	int i,sCount=0;
    /*while (p != NULL)
    {
        array[i++] = p;
        p = strtok (NULL, ".");
		++sCount;
    }
	
	if(sCount!=4){
		printf("Subnet Given is Wrong\n");
		return 1;
	}

	int iSubnet[4];
    for (i = 0; i < sCount; ++i){ 
        //printf("%s\n", array[i]);
		iSubnet[i]=atoi(array[i]);
		if(iSubnet[i]<0||iSubnet[i]>255){
			printf("Subnet Given is Wrong\n");
			return 1;
		}
	}
	
	//Getting the gateway	
	p = strtok (argv[2], ".");
    
	sCount=0;
	i=0;
    while (p != NULL)
    {
        array[i++] = p;
        p = strtok (NULL, ".");
		++sCount;
    }
	
	if(sCount!=4){
		printf("Gateway Given is Wrong\n");
		return 1;
	}
	
	int iGateway[4];
	for (i = 0; i < sCount; ++i){ 
        //printf("%s\n", array[i]);
		iGateway[i]=atoi(array[i]);
		if(iGateway[i]<0||iGateway[i]>255){
			printf("Gateway Given is Wrong\n");
			return 1;
		}
	}
	
	unsigned int hostSize[4];
	unsigned int networkIP[4];
	
	//finding size	
	for (i = 0; i < sCount; ++i){ 
       hostSize[i]=iSubnet[i]^255;	   
	   networkIP[i]=iSubnet[i]&iGateway[i];
	   printf("%d %d\n", hostSize[i], networkIP[i]);
	}
	
		
	int udpSocket, nBytes;
	char buffer[1024];
	struct sockaddr_in serverAddr, clientAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size, client_addr_size;
	

	/*Create UDP socket*/
	udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  
	
	/*Configure settings in address struct*/
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portNo);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	

	/*Bind socket with address struct*/
	bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	/*Initialize size variable to be used later on*/
	addr_size = sizeof serverStorage;

	struct IncomingPacket *inData= malloc(sizeof(struct IncomingPacket));
	
	int k;
	int currentIP[4];	
	for(k=0;k<4;++k){
		currentIP[k]=networkIP[k];
	}
	int *pp;
	
	int mSize=(hostSize[0]>0?hostSize[0]:1)*(hostSize[1]>0?hostSize[1]:1)*(hostSize[2]>0?hostSize[2]:1)*(hostSize[3]>0?hostSize[3]:1);
	printf("Size %d\n",mSize);
	struct IPnTime *root=malloc(mSize*sizeof(struct IPnTime));
	int countReserved=0;
	
	for(k=0;k<4;++k){
		root[countReserved].ip[k]=iGateway[k];
	}
	root[countReserved].timed=-1;//For Gateway
	++countReserved;
	
	while(1){
		nBytes = recvfrom(udpSocket,inData,sizeof(*inData),0,(struct sockaddr *)&serverStorage, &addr_size);
		if(inData->ip[0]==0&&inData->ip[1]==0&&inData->ip[2]==0&&inData->ip[3]==0){
			printf("\nRequest For New IP Received %d.%d.%d.%d\n",inData->ip[0],inData->ip[1],inData->ip[2],inData->ip[3]);
			printf("Client Transaction ID is %d\n",inData->transID);
			//Request for New ip
			//First IP
			while(!isIPAvailable(currentIP,root,countReserved)){
				pp=nextIPAddress(hostSize,networkIP,currentIP);
				for(k=0;k<4;++k){
					currentIP[k]=*(pp+k);
				}
			}			
			struct DHCPSuggest *s1=malloc(sizeof(struct DHCPSuggest));
			for(k=0;k<4;++k){
				s1->ip[k]=currentIP[k];				
			}
			s1->timed=3600;//Time
			//Send to client				
			sendto(udpSocket,s1,sizeof(struct DHCPSuggest),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("IP Address %d.%d.%d.%d is sent\n",s1->ip[0],s1->ip[1],s1->ip[2],s1->ip[3]);
			
			//Next IP to send
			pp=nextIPAddress(hostSize,networkIP,currentIP);
			for(k=0;k<4;++k){
				currentIP[k]=*(pp+k);
			}
				
			//Second IP
			while(!isIPAvailable(currentIP,root,countReserved)){
				pp=nextIPAddress(hostSize,networkIP,currentIP);
				for(k=0;k<4;++k){
					currentIP[k]=*(pp+k);
				}
			}
			struct DHCPSuggest *s2=malloc(sizeof(struct DHCPSuggest));
			for(k=0;k<4;++k){
				s2->ip[k]=currentIP[k];				
			}
			s2->timed=3600;//Time
			//Send to client				
			sendto(udpSocket,s2,sizeof(struct DHCPSuggest),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("IP Address %d.%d.%d.%d is sent\n",s2->ip[0],s2->ip[1],s2->ip[2],s2->ip[3]);
			
			//Next IP to send
			pp=nextIPAddress(hostSize,networkIP,currentIP);
			for(k=0;k<4;++k){
				currentIP[k]=*(pp+k);
			}
			
			//Third IP
			while(!isIPAvailable(currentIP,root,countReserved)){
				pp=nextIPAddress(hostSize,networkIP,currentIP);
				for(k=0;k<4;++k){
					currentIP[k]=*(pp+k);
				}
			}
			struct DHCPSuggest *s3=malloc(sizeof(struct DHCPSuggest));
			for(k=0;k<4;++k){
				s3->ip[k]=currentIP[k];				
			}
			s3->timed=3600;//Time
			//Send to client				
			sendto(udpSocket,s3,sizeof(struct DHCPSuggest),0,(struct sockaddr *)&serverStorage,addr_size);
			printf("IP Address %d.%d.%d.%d is sent\n",s3->ip[0],s3->ip[1],s3->ip[2],s3->ip[3]);
			
			//Next IP to send
			pp=nextIPAddress(hostSize,networkIP,currentIP);
			for(k=0;k<4;++k){
				currentIP[k]=*(pp+k);
			}
		}else{
			printf("\nRegistering IP Address %d.%d.%d.%d is sent\n",inData->ip[0],inData->ip[1],inData->ip[2],inData->ip[3]);
			printf("Client Transaction ID is %d\n",inData->transID);
			//Confirm IP
			//Register			
			struct DHCPACK *ackMsg=malloc(sizeof(struct DHCPACK));
			for(k=0;k<4;++k){
				root[countReserved].ip[k]=inData->ip[k];
				ackMsg->ip[k]=inData->ip[k];//For sending back
			}
			root[countReserved].timed=-1;//Time
			++countReserved;
			
			//Send back				
			sendto(udpSocket,ackMsg,sizeof(struct DHCPACK),0,(struct sockaddr *)&serverStorage,addr_size);
		}
		/*Send uppercase message back to client, using serverStorage as the address*/
		//sendto(udpSocket,buffer,nBytes,0,(struct sockaddr *)&serverStorage,addr_size);
	}

	return 0;
}