#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "lib/helper.h"

#define FILE_LEN 32
#define BUF_SIZE 1024
void error_handling(char *message);
void *handle_client(void *arg);
pthread_mutex_t lock; 

struct IpandSock {
    int sock;
    char IP[20];
	char binary_file[30];
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

	mkdir("testcase", 0777);

	int binary_filelist = 0;

	struct IpandSock* args;
	char temp[100];

	while (1)
	{
		args = (struct IpandSock*)malloc(sizeof(struct IpandSock));

		clnt_adr_sz = sizeof(clnt_adr);    
		clnt_sd = accept(serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

		strcpy(args->IP, inet_ntoa(clnt_adr.sin_addr));
		args->sock = clnt_sd;

		sprintf(temp, "%d", binary_filelist++);
		strcpy(args->binary_file, two_string_concat("./bin/target_program", temp) );

        pthread_create(&thread, NULL, handle_client, (void*)args);
        pthread_detach(thread);
	
	}
	
	close(serv_sd);
	return 0;
}

void *handle_client(void *arg)
{   
    // TODO: file receiving 
	struct IpandSock* args = (struct IpandSock*)arg;

    int sock = args->sock;
	char *IP = args->IP;
	char* target_file_name = args->binary_file;

    char file_name[FILE_LEN];
    char buf[BUF_SIZE];
	char cli_options[FILE_LEN];
    int str_len = 0;

    //read file name
	str_len = read(sock, file_name, sizeof(file_name));
	file_name[str_len] = 0;

	strcpy(file_name, extract_file_name(file_name));

	str_len = read(sock, cli_options, sizeof(cli_options));
	cli_options[str_len] = 0;

	fprintf(stdout, "Received %s from %s\n", file_name, IP);

	char* file_path = two_string_concat("./testcase/", file_name);

	pthread_mutex_lock(&lock);

    FILE *fp;
    fp = fopen(file_path, "wb");
    
    while ( (str_len=read(sock, buf, sizeof(buf))) != 0 )
        fwrite((void*)buf, 1, str_len, fp);    
    
	fclose(fp);

	char *file_executed_message = NULL;

	char* compile_message = compile_c_code(file_path, target_file_name);

	pthread_mutex_unlock(&lock);

	if ( compile_message == NULL ) { //Impossible to Compile

		fprintf(stdout, "%s", "File is uncompilable becasue of unknown error\n");
		write(sock, "File is uncompilable\n", 25);

		
	} else if ( strcmp(compile_message, "No error") == 0 ) { // NO compile error

		file_executed_message = get_msg_using_pipe(target_file_name, cli_options); 

		fprintf(stdout, "Compile %s and return results\n%s", file_name, file_executed_message);

		write(sock, file_executed_message, strlen(file_executed_message));

		free(file_executed_message);

		remove_file(target_file_name);

	} else {

		if (fileExists(target_file_name)) { //Compiled, with warnings
 
			fprintf(stdout, "Warning: %s\n", compile_message);
			file_executed_message = get_msg_using_pipe(target_file_name, cli_options); 

			char* result_message_with_warning = two_string_concat(compile_message, file_executed_message);
			fprintf(stdout, "%s\n", file_executed_message);

			write(sock, result_message_with_warning, strlen(result_message_with_warning));

			free(file_executed_message);
			remove_file(target_file_name);

		} else {

			fprintf(stdout, "File is not compilable: %s", compile_message);
			write(sock, compile_message, strlen(compile_message));

		}

		free(compile_message);
	}

	close(sock);
	remove_file(file_path);
	free(args);
	
	return NULL;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
