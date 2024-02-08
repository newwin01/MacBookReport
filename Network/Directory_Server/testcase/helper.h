#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h> 
#define RESULT_BUFFER_SIZE 4086

void remove_file(char *filepath);
char * two_string_concat (char *string1, char *string2);
int fileExists(char *filename);
char* extract_file_name(char *path);
char ** list_dir (char * dirpath, int *count);
int remove_file_exists(char *file_path);
int create_directory(char *file_path);
char *make_space_string(int count);                                                                                                                         