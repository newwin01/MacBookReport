#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#define RESULT_BUFFER_SIZE 4086

char * compile_c_code (char *c_file, char *target_file_name);
char * get_msg_using_pipe (char *target_file_name, char *cli_options);
void remove_file(char *filepath);
char * two_string_concat (char *string1, char *string2);
int check_c_file(char *string);
int fileExists(char *filename);
char* extract_file_name(char *path);