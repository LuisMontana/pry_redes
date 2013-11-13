#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <string.h>
#include <sys/sendfile.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
 
#define SERVER_PORT	6543
#define SERVER_ADDRESS	"192.168.250.122"
#define MAXLINE		512
#define FILENAME "/home/dmontenegro/Escritorio/nuevo.txt" 
void* recibir(void* p);
void* enviar(void* p);
 
int main()
{
	struct sockaddr_in addr;
	int sd,status;
	pthread_t hilos[2];
 
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
	addr.sin_port = htons(SERVER_PORT);
 
	if((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Error al crear el socket\n");
		exit(0);	
	}	
	if(connect(sd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		printf("Error al conectar\n");
		exit(0);
	}
	else
	{	
		if ( (status = pthread_create(&hilos[0],NULL,recibir,(void*)&sd)) )
		{
			printf("Error al crear hilo para recibir\n");
			close(sd);
			exit(0);
		}		
		if ( (status = pthread_create(&hilos[1],NULL,enviar,(void*)&sd)) )
		{
			printf("Error al crear hilo para enviar\n");
			close(sd);
			exit(0);
		}
		pthread_join(hilos[0],NULL);
		pthread_join(hilos[1],NULL);		
	}
	return 1;
}
 
void* recibir(void* p)
{
	int* id;
	char buffer[MAXLINE];
	id = (int*) p;
	FILE* received_file;
	int remain_data=0;
	int file_size;
	ssize_t len;
	while(1)
	{
		recv(*id,buffer,MAXLINE,0);
		if (strstr(buffer,"DOWN")){
			/* Receiving file size */
			printf("Downloading file..\n");
			printf("Waiting for size..");
			fflush(stdout);
        	recv(*id,buffer,MAXLINE,0);
        	file_size = atoi(buffer);
        	fprintf(stdout, "\nFile size : %d\n", file_size);
	
        	received_file = fopen(FILENAME, "w");
        	if (received_file == NULL)
        	{
        	        fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));

        	        exit(EXIT_FAILURE);
       		}

        	remain_data = file_size;
			printf("Waiting to store file at client..");
        	while (((len = recv(*id,buffer,MAXLINE,0)) > 0) && (remain_data > 0))
        	{
         	       fwrite(buffer, sizeof(char), len, received_file);
         	       remain_data -= len;
        	        fprintf(stdout, "Receive %d bytes and we hope :- %d bytes\n", len, remain_data);
			if (remain_data==0)break;
      		}	
			fclose(received_file);
			printf("Succesfully download!");
			fflush(stdout);
		}
		else{
			printf("%s\n",buffer);
		}
		fflush(stdout);
		sleep(2);
	}
}

void subCadena(char *subCad, char *cad, int inicio, int cuantos)
{
     int i,j=0;
     for(i=inicio;i<inicio+cuantos && cad[i]!='\0';i++)
     {
        subCad[j]=cad[i];
        j++;
     }
     subCad[j]='\0';
} 

void* enviar(void* p)
{
	int* id;
	int fd,rc;
	off_t offset = 0;
	char buffer[MAXLINE];
	char filename[MAXLINE];
	id = (int*) p;
	struct stat stat_buf;
	char file_size[256];
	
	while(1)
	{	 
		printf("\tIntroduce el mensaje");
		fgets(buffer , MAXLINE , stdin);
		strtok(buffer,"\n");
		if(strstr(buffer,"FILE")){
			
			subCadena(filename,buffer,5,strlen(buffer)-5);
			fd = open(filename, O_RDONLY);
			fstat(fd, &stat_buf);
			offset = 0;			
			send(*id,buffer,MAXLINE,0);
			sleep(5);

			sprintf(file_size, "%d", stat_buf.st_size);
			send(*id,file_size, sizeof(file_size),0);

			sleep(3);
			rc = sendfile (*id, fd, &offset, stat_buf.st_size);
		    if (rc == -1) {
				fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
     	 		exit(1);
    			}
    		    if (rc != stat_buf.st_size) {
      			fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n",
              	rc,
              	(int)stat_buf.st_size);
      		exit(1);
    		}

    		/* close descriptor for file that was sent */
    		close(fd);
			}
		else{
			send(*id,buffer,MAXLINE,0);
		}
		//sleep(3);
	}
}
