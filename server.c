#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <time.h>


/*typedef struct users
{
	char* nickname;
	int sockt1;
	int sockt2;
};*/

//struct users list[20];

pthread_t threads[21];

int sockall[20];
int th[20];
int newsock;
int i=0;
char ultimorecibido[256]; 
int ultimomensajeid;
int threadtokill; 

void *hiloread(void *arg){
	printf("Hilo! %d",i);
	int tmp=i;
	printf("%d",tmp);
	fflush(stdout);
	char buffer[256];
	ssize_t recibido;
	int desconectado=0; 
	time_t start; 
	time_t actual; 
	while(1){	
		recibido= recv(sockall[tmp], buffer, 255, 0);
		/* este if es para ver si la conexion sigue activa, sino le da un timer para volver a conectarse, si despues de ese tiempo no se reconecta.
		   el hilo se mata y el cliente es sacado de la lista de clientes permitidos, pero se podra volver a conectar.  
		*/
		if(recibido !=0){
			printf("Message: %d %s \n",tmp ,buffer);
			fflush(stdout);	
			ultimomensajeid=tmp; 	
			strncpy(ultimorecibido, buffer, sizeof(ultimorecibido));
		}
		else{
			if( desconectado !=1){
				desconectado =1; 
				time(&start); 			
			}
			time(&actual); 
			if( difftime(actual, start) > 5.0 ){
				threadtokill=tmp; 
				break; 
			}
		
		}
	}
}

void *hilowrite (void *arg){ 
	printf ("Hilo! %d", i); 
	int tmp=i; 
	printf("%d", tmp); 
	fflush(stdout); 
	char ultimoenviado[256];
	
	while(1){
		if(strcmp(ultimoenviado, ultimorecibido) && ultimomensajeid!=tmp-1){		
			strncpy(ultimoenviado, ultimorecibido, sizeof(ultimoenviado));
			if(write(sockall[tmp], ultimoenviado, 255)){
				printf("mensaje enviado exitosamente %s", ultimoenviado );
				fflush(stdout); 
			}
		}
	}
}

void *hilokill( void *arg){
	while(1){
		if (threadtokill!=21){
			pthread_kill(threads[threadtokill]); 
			pthread_kill(threads[threadtokill+1] );	
			printf("mato al hilo %d \n", threadtokill); 	 	
			threadtokill=21; 		
		}
	}
}

int main(int argc, char** argv){
	int port=5050;
	int port2=5051;
	int sock,sock2,addrlen;
	threadtokill=21; 
	// toda la informacion pertinente al socket
	if(!(sock = socket(AF_INET, SOCK_STREAM, 0))){
		printf("ERROR : error creating socket\n");
	}
	if(!(sock2 = socket(AF_INET, SOCK_STREAM, 0))){
		printf("ERROR : error creating socket\n");
	}

	
	struct sockaddr_in ad, add, ad2, add2;
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = htons(port);

	ad2.sin_family = AF_INET;
	ad2.sin_addr.s_addr = INADDR_ANY;
	ad2.sin_port = htons(port2);
	
	int tr=1;

	if (setsockopt(sock2,SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	
	if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	bind(sock, (struct sockaddr *)&ad, sizeof(ad));
	listen(sock, 10);

	bind(sock2, (struct sockaddr *)&ad2, sizeof(ad2));
	listen(sock2, 10);
	
	printf("Inicializacion del server \n");
	fflush(stdout);
	//pthread_create( &threads[20], NULL, hilokill, "..."); 
	while(1){
		if( i<20){
			//identificador de la conexion para escuchar a el cliente i
			addrlen = sizeof(add);
			sockall[i] = accept(sock,(struct sockaddr *) &add, &addrlen);
			if (!sockall[i]){
		    	perror("Error al aceptar la conexion");
			}
			else{
				write(sockall[i],"Conected!",100);
				fflush(stdout); 
				// hilo para escribir al cliente
				pthread_create(&threads[i], NULL, hiloread, "...");
				usleep(5000);
				i=i+1;
			}	
			addrlen = sizeof(add2);
		
			//identificador de la conexion para escribir a el cliente i-1
			sockall[i] = accept(sock2,(struct sockaddr *) &add2, &addrlen);
			if (!sockall[i]){
		    	perror("Error al aceptar la conexion");
			}
			else{

				write(sockall[i],"Conected!",100);
				fflush(stdout); 
				// hilo para escribir al cliente
				pthread_create(&threads[i], NULL, hilowrite, "...");
				usleep(5000);
				i=i+1;
			} 
			printf(" hilos %d, %d", i-2, i-1); 
			fflush(stdout); 
		}
		

	}
	
}
