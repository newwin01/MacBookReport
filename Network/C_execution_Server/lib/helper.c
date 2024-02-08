#include "helper.h"

char* compile_c_code (char *c_file, char *targe_file_name) {

    char *compile_result_message = (char *)malloc( RESULT_BUFFER_SIZE);
    memset(compile_result_message, 0x0, RESULT_BUFFER_SIZE);

    char *compileCommand;
    char buf[RESULT_BUFFER_SIZE];

    mkdir("bin", 0777);

	compileCommand = two_string_concat("gcc ", c_file);
	compileCommand = two_string_concat(compileCommand, " -o ");
    compileCommand = two_string_concat(compileCommand, targe_file_name);
    compileCommand = two_string_concat(compileCommand, " 2>&1"); //to read stderr

    FILE *pipe_file = popen(compileCommand, "r");

    while(fgets(buf, RESULT_BUFFER_SIZE, pipe_file) != NULL){
        compile_result_message = two_string_concat(compile_result_message, buf);
    }

    free(compileCommand);

    if(pipe_file == NULL) { //Send it is uncompilable file.
        pclose(pipe_file);
        return NULL; 
    } else if(strlen(compile_result_message) > 0) { //Error on compilation

        return compile_result_message;

    }
    else { //No error
        pclose(pipe_file);
        return "No error";
    }

    pclose(pipe_file);
    return NULL;
}

char* extract_file_name(char *path) {
    char *fileName = strrchr(path, '/');
    
    if (fileName != NULL) {
        fileName++; 
    } else {
        fileName = path;
    }

    return fileName;
}

int fileExists(char *filename) {
    return access(filename, F_OK) == 0;
}

int check_c_file(char *string) {
    int length = strlen(string);

    if (length >= 2 && string[length - 2] == '.' && string[length - 1] == 'c') {
        return 1;
    }

    return 0; 
}

char * get_msg_using_pipe (char *target_file_name, char *cli_options) { // 0 is solution 1 is target
    
    char* exeCommand;

    if ( strcmp(cli_options, "no options") == 0 ) {
        exeCommand = target_file_name;
    } else {
        exeCommand = two_string_concat(target_file_name, " ");
        exeCommand = two_string_concat(exeCommand, cli_options);
    }

    char *buf = (char*)malloc(RESULT_BUFFER_SIZE);
    char *result = (char*)malloc(RESULT_BUFFER_SIZE);

    FILE *pipe_file = popen(exeCommand, "r");

    while(fgets(buf, RESULT_BUFFER_SIZE, pipe_file) != NULL){
        result = two_string_concat(result, buf);
    }

    free(buf);
    pclose(pipe_file);

    return result;
}

void remove_file (char *filepath)  {
    
    if (remove(filepath) != 0)     
        fprintf(stderr, "%s\n", "Error deleting file");

}

char * two_string_concat (char *string1, char *string2) {

    if (string1 == NULL) {
        return string2;
    }
    if (string2 == NULL) {
        return string1;
    }

    char *result_string = malloc(sizeof(char) * (strlen(string1) + strlen(string2) + 1));

    if (result_string == NULL) {
        fprintf(stderr, "%s", "Memory allocation failed\n");
        exit(-1);
    }

    strcpy(result_string, string1);

    strcat(result_string, string2);

    return result_string;
}
