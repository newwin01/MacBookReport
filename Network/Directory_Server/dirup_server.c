#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>
#include "lib/helper.h"

#define FILE_LEN 128
#define BUF_SIZE 32
void error_handling(char *message);
void *handle_client(void *arg);
pthread_mutex_t lock; 

struct IpandSock {
    int sock;
    char IP[20];
};

int main(int argc, char *argv[])
{
	int serv_sd, clnt_sd;
	pthread_t thread; 
	
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;
	
	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sd = socket(PF_INET, SOCK_STREAM, 0);   
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	if (bind(serv_sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if (listen(serv_sd, 5) == -1)
		error_handling("listen() error");

	struct IpandSock* args;
	while (1)
	{
		clnt_adr_sz = sizeof(clnt_adr);    
		clnt_sd = accept(serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

		args = (struct IpandSock*)malloc(sizeof(struct IpandSock));

		strcpy(args->IP, inet_ntoa(clnt_adr.sin_addr));
		args->sock = clnt_sd;

        pthread_create(&thread, NULL, handle_client, (void*)args);
        pthread_detach(thread);
	}
	
	close(serv_sd);
	return 0;
}

void *handle_client(void *arg)
{   
	struct IpandSock* args = (struct IpandSock*)arg;

	int sock = args->sock;
	char *IP = args->IP;

    int successful_files;

    char file_name[FILE_LEN];
    char buf[BUF_SIZE];
    long str_len = 0;

	int count;
	int total_cnt = 0;

	str_len = read(sock, buf, 5);
	buf[str_len] = 0;
	count = atoi(buf);

	for (int i = 0 ; i < count ; i++) {
		
		str_len = read(sock, file_name, FILE_LEN);
		file_name[str_len] = 0;

		if ( strcmp(file_name, "File Transmission is Canceled") == 0) {
			fprintf(stdout, "%s\n" , two_string_concat("File upload cancellation message is received from ", IP));
			close(sock);
			free(args);
			return NULL;
		}

		FILE *fp;
    	// fp = fopen( two_string_concat("testcase/", extract_file_name(file_name)), "wb"); 

		create_directory(file_name);
		fp = fopen( file_name, "wb"); 
		
		while (1){
			
			str_len = read(sock, buf, BUF_SIZE);

			if ( strcmp(buf, "File Transmission is Canceled") == 0)  {
				fprintf(stdout, "%s\n" , two_string_concat("File upload cancellation message is received from ", IP));
				close(sock);
				free(args);
				remove_file_exists(file_name);
				return NULL;
			}

			if ( strcmp(buf, "File Transmission Is finised") == 0) {
				write(sock, "File Transmission Is Completed", BUF_SIZE);
				fprintf(stdout, "[%s] Received From %s … %d bytes \n", file_name, IP, total_cnt);
                successful_files++;
				break;
			}
			
			total_cnt += str_len;
			fwrite((void*)buf, 1, str_len, fp);  
			
		}	
	
		total_cnt = 0;
		fclose(fp);
	}
	

    fprintf(stdout, "[%s] The client (%s) has completed the upload of %d/%d files!\n", file_name,
    IP, successful_files, count);
	close(sock);

	return NULL;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
