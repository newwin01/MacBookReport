#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include "lib/helper.h"
#include <pthread.h>

#define BUF_SIZE 32
#define FILE_LEN 128
pthread_mutex_t lock; 
int dest_sock;
char *IP;
int finish_count;
int count;
struct _checklist checklist;

struct IpandFile {
    int sock;
	char file[128];
	int index;
};

struct _checklist {
	char **file_list;
	int *count;
};

void error_handling(char *message);

void sig_handler(int sig)
{
    if(sig == SIGINT)
    {
		fprintf(stdout, "%s", "Ctrl+C pressed\n");
		write(dest_sock, "File Transmission is Canceled", BUF_SIZE);

		fprintf(stdout, "%d/%d files has been uploaded!\n", finish_count, count);

		for (int i = 0 ; i < count ; i++) {
			if (checklist.count[i] == 0){
				fprintf(stdout, "[%s] upload canceled\n", checklist.file_list[i]);
			}
		}

		exit(0);
    }
}


void * thread_send(void *arg)
{   
	FILE *fp;

	struct IpandFile* args = (struct IpandFile*)arg;

	int sd = args->sock;
	char *file_name = args->file;
	int index = args->index;

    char *buf = malloc(BUF_SIZE);

	pthread_mutex_lock(&lock);
	dest_sock = sd;

	write(sd, file_name, FILE_LEN);
	fp = fopen(file_name, "rb");
	if(fp == NULL) {
		fprintf(stderr, "%s", "File Open Error");
		exit(-1);
	}

	int read_cnt;
	int total_cnt = 0;

	while(1)
	{
		memset(buf, 0x0, BUF_SIZE);
		
		read_cnt = fread((void*)buf, 1, BUF_SIZE, fp);
		total_cnt += read_cnt;

		if (read_cnt < BUF_SIZE)
		{
			total_cnt += BUF_SIZE-read_cnt;
			write(sd, two_string_concat(buf, make_space_string(BUF_SIZE-read_cnt)), BUF_SIZE);

			write(sd, "File Transmission Is finised", BUF_SIZE);
			

			read(sd, buf, BUF_SIZE);

			if ( strcmp(buf, "File Transmission Is Completed") == 0 ) {
				fprintf(stdout, "[%s] Upload completed! … %d bytes \n", file_name, total_cnt);
				checklist.count[index] = 1;
				finish_count++;
				break;
			}
		}

		write(sd, buf, BUF_SIZE);
	}

	// sleep(1);

	pthread_mutex_unlock(&lock);

	#ifdef _DEBUG
		fprintf(stdout, "%d %s\n", read_size ,"file sent");
	#endif

	fclose(fp);
	free(args);

	return NULL;
}


int main(int argc, char *argv[])
{
	int sd = 0;
	char temp[5];
	memset(temp, 0x0, 5);
	
	struct sockaddr_in serv_adr;
	if (argc != 4) {
		printf("Usage: %s <IP> <port> <dir name> \n", argv[0]);
		exit(1);
	} 

	pthread_mutex_init(&lock, NULL);
	

	sd = socket(PF_INET, SOCK_STREAM, 0);   

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));

	
	char **dir_list = list_dir(argv[3], &count);

	checklist.file_list = dir_list;
	checklist.count = (int *)malloc(count * sizeof(int));
	
	for (int i = 0 ; i < count ; i++){
		checklist.count[i] = 0;
	}

	signal(SIGINT, sig_handler) ;

	connect(sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr));

	pthread_t thread[count];
	struct IpandFile *args;

	sprintf(temp, "%d", count);
	write(sd, temp, 5);

	for (int i = 0 ; i < count ; i++) {
		// fprintf(stdout, "%d", 1); //TODO: Delete this line
		args = (struct IpandFile*)malloc(sizeof(struct IpandFile));
		strcpy(args->file, dir_list[i]);
		// fprintf(stdout, "%s\n", dir_list[i]);
		args->sock = sd;
		args->index = i;
		pthread_create(&thread[i], NULL, thread_send, (void*)args);
	}

	for (int i = 0 ; i < count ; i++) {
		pthread_join(thread[i], NULL);
	}

	fprintf(stdout, "%d/%d files has been uploaded! \n", finish_count, count);
	
	pthread_mutex_destroy(&lock);
	close(sd);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}